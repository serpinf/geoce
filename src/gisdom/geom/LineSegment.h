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

#pragma once

#include "Coordinate.h"

namespace geom
{
template <bool Z, bool M>
class line_segment final
{
public:
    using coordinate_type = detail::coo_impl<Z, M>;
    line_segment() = default;

    template <bool L, bool F>
    explicit line_segment(const line_segment<L, F> &other) : A(other.A), B(other.B)
    {}

    /**
     * Constructs a LineSegment with the given start and end coordinates.
     *
     * @param c0 - start point
     * @param c1 - end point
     */
    template <bool L, bool F>
    line_segment(const detail::coo_impl<L, F> &a, const detail::coo_impl<L, F> &b) : A(a), B(b)
    {}

    /**
     * Returns the counter-clockwise 2D angle from a base LineSegment to this LineSegment.
     *
     * @param base - base line segment for comparison
     *
     * @return the angle in radians
     */
    double angle(const line_segment &base) const
    {
        return gce::angle(glm::dvec2(B.pos - A.pos), glm::dvec2(base.B.pos - base.A.pos));
    }

    /**
     * Computes the intersection points of this line segment with a circle.
     *
     * @param res - array to store the resulting intersection points
     * @param cen - center of the circle
     * @param R - radius of the circle
     *
     * @return number of intersections
     */
    int intersectCircle(Coordinate res[2], const glm::dvec2 &cen, double R) const
    {
        glm::dvec2 T = glm::dvec2(B.pos) - glm::dvec2(A.pos);
        glm::dvec2 C = glm::dvec2(A.pos) - cen;
        double TT = dot(T, T);
        double TC = dot(T, C);

        // calc discriminant
        const double D = TC * TC + TT * (R * R - dot(C, C));

        int nPoints = 0;
        if (std::abs(D) < std::numeric_limits<double>::epsilon())
        {
            double t = -TC / TT;
            if (t >= 0.0 && t <= 1.0)
            {
                res[nPoints++] = lerp(t);
            }
        }
        if (D > 0.0)
        {
            double t1 = (-TC - sqrt(D)) / TT;
            if (t1 >= 0.0 && t1 <= 1.0)
            {
                res[nPoints++] = lerp(t1);
            }

            double t2 = (-TC + sqrt(D)) / TT;
            if (t2 >= 0.0 && t2 <= 1.0)
            {
                res[nPoints++] = lerp(t2);
            }
        }
        return nPoints;
    }

    /**
     * Returns the angle this segment makes with the X axis.
     *
     * @return the angle in radians
     */
    double angle() const
    {
        return gce::angleX(B.pos - A.pos);
    }

    /**
     * Returns the angle this segment makes with the Y axis.
     *
     * @return the angle in radians
     */
    double angleY() const
    {
        // note the use of inverted X axis position
        return atan2(A.pos.x - B.pos.x, B.pos.y - A.pos.y);
    }

    /**
     * Computes the 2D length of this line segment.
     *
     * @return the length of the segment
     */
    double length2d() const
    {
        return glm::distance(glm::dvec2(A.pos), glm::dvec2(B.pos));
    }

    /**
    * Tests whether this segment is horizontal in 2D.
     *
     * @return true if the segment is horizontal, false otherwise
     */
    bool isHorizontal() const
    {
        return fabs(B.pos.y - A.pos.y) < std::numeric_limits<double>::epsilon();
    }

    /**
     * Tests whether this segment is vertical in 2D.
     *
     * @return true if the segment is vertical, false otherwise
     */
    bool isVertical() const
    {
        return fabs(B.pos.x - A.pos.x) < std::numeric_limits<double>::epsilon();
    }

    /**
     * Tests whether this segment is parallel to another segment.
     *
     * @param seg2 - the segment to test for parallelism
     *
     * @return true if the segments are parallel, false otherwise
     */
    bool isParallel(const line_segment &seg2) const
    {
        return gce::parallel(B.pos - A.pos, seg2.B.pos - seg2.A.pos);
    }


    coordinate_type lerp(double t) const
    {
        return mix(A, B, t);
    }

    /**
     * Computes the projection factor for projecting a point onto this line segment.
     * The projection factor is the scalar k by which the direction vector of this segment
     * must be multiplied to equal the vector to the projection of the point.
     *
     * @param p - the point to project
     *
     * @return the projection factor
     */
    double projectionFactor(const coordinate_type &p) const
    {
        return gce::projectionFactor(p.pos, A.pos, B.pos);
    }

    /**
     * Returns a LineSegment that is perpendicular to this segment (rotated by PI/2).
     *
     * @return a perpendicular line segment
     */
    line_segment perpSegment() const
    {
        coordinate_type B1 = B;
        B1.pos = A.pos + gce::perpVector(B.pos - A.pos);
        return line_segment(A, B1);
    }

    /**
     * Computes the number of intersection points between lines passing thruogh this segment and another segment.
     *
     * @param dest - stores the intersection point if one exists
     * @param seg - the segment to test for intersection
     *
     * @return 0 if no intersection, 1 if intersection found (dest contains the point),
     *         -1 if infinite intersections (collinear segments)
     */
    int intersection2d(glm::dvec2 &dest, const line_segment &seg) const
    {
        // if lines are represented as q + s and p + r
        // intersection point is: p + t * r = q + u * s
        // projection of intersection point on p is:
        // u = (q − p) × r / (r × s), 

        static constexpr double eps = gce::big_epsilon<double>();

        auto r = glm::dvec2(B.pos) - glm::dvec2(A.pos);
        auto s = glm::dvec2(seg.B.pos) - glm::dvec2(seg.A.pos);
        auto pq = glm::dvec2(seg.A.pos) - glm::dvec2(A.pos);

        double rs = gce::cross(r, s);
        double pqr = gce::cross(pq, r);

        if (std::abs(rs) < eps)
        {
            return std::abs(pqr) < eps ? -1 : 0;
        }

        double u = pqr / rs;
        dest = glm::dvec2(seg.A.pos) + s * u;
        return 1;
    }

    /**
     * Computes the closest point on this line segment to another point.
     *
     * @param p - the point for which to find the closest point
     *
     * @return a Coordinate representing the closest point on this segment to point p
     */
    coordinate_type closestPoint(const coordinate_type &p) const
    {
        return lerp(std::clamp(projectionFactor(p), 0.0, 1.0));
    }

    /**
     * Translates this segment so that point A coincides with the given point.
     *
     * @param A1 - the new position for point A
     *
     * @return a translated copy of this line segment
     */
    line_segment translateSegment(const coordinate_type &A1) const
    {
        coordinate_type B1 = B;
        B1.pos += A1.pos - A.pos;
        return line_segment(A1, B1);
    }

    bool operator==(const line_segment &other) const
    {
        return A == other.A && B == other.B;
    }

    //! Segment start
    detail::coo_impl<Z, M> A;
    //! Segment end
    detail::coo_impl<Z, M> B;
};
using LineSegmentXY = line_segment<false, false>;
using LineSegmentXYZ = line_segment<true, false>;
using LineSegmentXYM = line_segment<false, true>;
using LineSegment = line_segment<true, true>;
}; // namespace geom
