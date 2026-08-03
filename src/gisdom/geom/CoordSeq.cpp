// Copyright 2026 Sergei Pikin
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
#include "gcprec.h"

#include "CoordSeq.h"
#include "CoordinateFilter.h"

template <class Iter>
void reverse_stride(Iter first, Iter last, int stride)
{
    std::advance(last, -stride);
    while (first < last)
    {
        std::swap_ranges(first, first + stride, last);
        std::advance(first, stride);
        std::advance(last, -stride);
    }
}

// Function to compute the cross product of vectors (p1p2)
// and (p1p3)
inline double crossProduct(const glm::dvec2 &p1, const glm::dvec2 &p2, const glm::dvec2 &p3)
{
    return (p2.x - p1.x) * (p3.y - p1.y) - (p2.y - p1.y) * (p3.x - p1.x);
}

// Function to check if point p lies on segment p1p2
inline bool isPointOnSegment(const glm::dvec2 &p, const glm::dvec2 &p1, const glm::dvec2 &p2)
{
    // Check if point p lies on the line segment p1p2 and
    // within the bounding box of p1p2
    return crossProduct(p1, p2, p) == 0
        && p.x >= std::min(p1.x, p2.x)
        && p.x <= std::max(p1.x, p2.x)
        && p.y >= std::min(p1.y, p2.y)
        && p.y <= std::max(p1.y, p2.y);
}

template<glm::length_t L, typename T, glm::qualifier Q>
T distancePointToSegment2(const glm::vec<L, T, Q> &p, const glm::vec<L, T, Q> &A, const glm::vec<L, T, Q> &B) noexcept
{
    const glm::vec<L, T, Q> AB = B - A;
    const glm::vec<L, T, Q> AP = p - A;
    const T r = gce::projectionFactor(AP, AB);

    // Vector p -> closest point on the segment: A + r * AB
    const glm::vec<L, T, Q> PR = AP - glm::clamp(r, T(0), T(1)) * AB;

    // Distance^2 from p to the line
    return glm::dot(PR, PR);
}

namespace geom
{

CoordinateSeq::CoordinateSeq(CoordinateType format) :m_format(format)
{
    switch (m_format)
    {
    case CoordinateType::XY:
        m_hasZ = false;
        m_offsetM = 0;
        m_stride = 2;
        break;
    case CoordinateType::XYZ:
        m_hasZ = true;
        m_offsetM = 0;
        m_stride = 3;
        break;
    case CoordinateType::XYM:
        m_hasZ = false;
        m_offsetM = 2;
        m_stride = 3;
        break;
    default: // CoordinateType::XYZM:
        m_hasZ = true;
        m_offsetM = 3;
        m_stride = 4;
        break;
    }
}

double CoordinateSeq::distance(const glm::dvec2 &pos) const
{
    using coordinate_type = detail::coo_impl<false, false>;

    if (m_data.empty()) return DoubleInfinity;
    if (m_data.size() == m_stride)
    {
        return glm::distance(pos, front<false, false>().pos);
    }

    double dist2 = DoubleInfinity;
    for_each_edge<false, false>([&dist2, pos](const coordinate_type &a, const coordinate_type &b){
        dist2 = std::min(dist2, distancePointToSegment2(pos, a.pos, b.pos));
    });
    return sqrt(dist2);

}

bool CoordinateSeq::EnsureRing()
{
    if (!has_at_least_points(3)) return false;
    auto A = front<true, false>();
    auto B = back<true, false>();
    if (A.pos != B.pos)
    {
        if (gce::equalsEpsilon(A.pos, B.pos))
        {
            // if front and back are nearly equal, assign front to back
            set_at(A, std::prev(m_data.end(), m_stride));
        }
        else
        {
            // if the ring is actually open, add closing point
            push_back(A);
        }
    }
    return true;
}

void CoordinateSeq::apply_filter_rw(filter_rw &filter)
{
    for (auto it = m_data.begin(); it != m_data.end(); std::advance(it, m_stride))
    {
        set_at(filter(get_at<true, true>(it)), it);
    }
}

void CoordinateSeq::apply_filter_ro(filter_ro &filter) const
{
    for (auto it = m_data.cbegin(); it != m_data.cend(); std::advance(it, m_stride))
    {
        filter(get_at<true, true>(it));
    }
}

int CoordinateSeq::windingNumber(const glm::dvec2 &pos) const
{
    // TODO: use for_each_edge
    int n = this->size();
    if (n < 2)
    {
        return 1;
    }

    int windingNumber = 0;

    // Iterate through each edge of the polygon
    for (int i = 1; i < n; i++)
    {
        // prev vertex in the Polygon
        const glm::dvec2 p1 = this->get<false, false>(i - 1).pos;
        // Next vertex in the polygon
        const glm::dvec2 p2 = this->get<false, false>(i).pos;

        // Check if the point lies on the current edge
        if (isPointOnSegment(pos, p1, p2))
        {
            // Point is on the polygon boundary
            return 0;
        }

        // Calculate the cross product to determine winding
        // direction
        if (p1.y <= pos.y)
        {
            if (p2.y > pos.y && crossProduct(p1, p2, pos) > 0)
            {
                windingNumber++;
            }
        }
        else
        {
            if (p2.y <= pos.y && crossProduct(p1, p2, pos) < 0)
            {
                windingNumber--;
            }
        }
    }
    // Return the winding number
    return windingNumber;
}

double CoordinateSeq::signedArea() const
{
    if (m_data.size() < 4 * m_stride) return 0.0;

    double sum = 0.0;
    for_each_edge<false, false>([&sum](auto &b, auto &c){sum += (b.pos.x + c.pos.x) * (c.pos.y - b.pos.y); });
    return -sum / 2.0;
}

CoordinateSeq &CoordinateSeq::reverse()
{
    reverse_stride(m_data.begin(), m_data.end(), m_stride);
    return *this;
}

void CoordinateSeq::append(const CoordinateSeq &other)
{
    if (m_format == other.m_format)
    {
        m_data.insert(m_data.end(), other.m_data.begin(), other.m_data.end());
    }
    else
    {
        switch (m_format)
        {
        case CoordinateType::XY:
            append<false, false>(other);
            break;
        case CoordinateType::XYZ:
            append <true, false>(other);
            break;
        case CoordinateType::XYM:
            append<false, true>(other);
            break;
        case CoordinateType::XYZM:
            append<true, true>(other);
            break;
        }
    }
}

}; //namespace geom{
