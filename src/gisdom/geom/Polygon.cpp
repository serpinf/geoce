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
#include <algorithm>
#include "Polygon.h" // class's header file
#include "CoordinateFilter.h"

namespace geom
{

bool Polygon::Create(const CoordinateSeq &outer)
{
    //TODO: consider ring validation
    m_shell.assign(outer);
    m_holes.clear();
    return true;
}

size_t Polygon::getNumPoints() const
{
    size_t numPoints = m_shell.size();
    for (auto &hole : m_holes)
    {
        numPoints += hole.size();
    }
    return numPoints;
}

bool Polygon::isPointInPolygon(const glm::dvec2 &pos) const
{
    bool res = m_shell.windingNumber(pos) != 0;
    if (res)
    {
        for (auto &hole : m_holes)
        {
            if (hole.windingNumber(pos) != 0)
            {
                res = false;
                break;
            }
        }
    }
    return res;
}

double Polygon::distance(const glm::dvec2 &pos) const
{
    if (this->isPointInPolygon(pos)) return 0.0;

    double dist = m_shell.distance(pos);
    for (auto &hole : m_holes)
    {
        dist = std::min(dist, hole.distance(pos));
    }
    return dist;
}

bool Polygon::isSimple() const
{
    return true;
}

double Polygon::area() const
{
    double area = m_shell.signedArea();
    for (auto &val : m_holes)
    {
        area += val.signedArea();
    }
    return area;
}

double Polygon::length() const
{
    double len = m_shell.length3D();
    for (auto &hole : m_holes)
    {
        len += hole.length3D();
    }
    return len;
}

void Polygon::apply_filter_rw(filter_rw &filter)
{
    m_shell.apply_filter_rw(filter);
    for (auto &_val : m_holes)
    {
        _val.apply_filter_rw(filter);
    }
}

void Polygon::apply_filter_ro(filter_ro &filter) const
{
    filter.beginGeometry(*this);
    m_shell.apply_filter_ro(filter);
    for (auto &_val : m_holes)
    {
        _val.apply_filter_ro(filter);
    }
    filter.endGeometry();
}

void Polygon::removeVertex(size_t idx)
{
    if (auto [_idx, ring] = this->getRingForVertex(idx); ring != nullptr)
    {
        if (size_t cnt = ring->size(); _idx < cnt)
        {
            if (ring->isClosed())
            {
                if (cnt < 5)
                {
                    throw std::logic_error("Ring may not contain less then 4 points");
                }
                if (_idx == 0)
                {
                    _idx = cnt - 1;
                }
                ring->Erase(_idx);

                // close ring
                if (_idx == cnt - 1)
                {
                    geom::Coordinate tmp;
                    ring->getBack(tmp);
                    ring->setFront(tmp);
                }
            }
            else
            {
                if (cnt < 3)
                {
                    throw std::logic_error("LineString may not contain less then 2 points");
                }
                ring->Erase(_idx);
            }
        }
    }
}

void Polygon::updateVertex(size_t idx, const Coordinate &coo)
{
    if (auto [_idx, ring] = this->getRingForVertex(idx); ring != nullptr)
    {
        size_t cnt = ring->size();
        if ((_idx == 0 || _idx == cnt - 1) && ring->isClosed())
        {
            ring->setBack(coo);
            ring->setFront(coo);
        }
        else if (_idx < cnt)
        {
            ring->set(coo, _idx);
        }
    }
}

void Polygon::insertVertex(size_t idx, const Coordinate &coo)
{
    if (auto [_idx, ring] = this->getRingForVertex(idx); ring != nullptr)
    {
        if (_idx < ring->size())
        {
            ring->Insert(_idx, coo);
        }
    }
}

std::pair<size_t, const Geometry *> Polygon::getGeometryForVertex(size_t idx) const
{
    return {idx, this};
}

// equality test
bool Polygon::equals(const Geometry *geom) const
{
    if (const Polygon *poly = geom->isPolygon(); poly != nullptr)
    {
        return m_shell == poly->m_shell && m_holes == poly->m_holes;
    }
    return false;
}

void Polygon::Clear()
{
    m_shell.clear();
    m_holes.clear();
    //clear();
}

GEOSGeom_scoped_t Polygon::toGEOSGeom() const
{
    // create exterior ring
    auto shell = GEOSGeom_createLinearRing(m_shell.toGEOSCoordSeq());

    std::vector<GEOSGeometry *> geoms;
    geoms.reserve(m_holes.size());
    for (auto &hole : m_holes)
    {
        geoms.push_back(GEOSGeom_createLinearRing(hole.toGEOSCoordSeq()));
    }
    return GEOSGeom_scoped_t(GEOSGeom_createPolygon(shell, geoms.data(), (unsigned int)geoms.size()));
}

bool Polygon::isValid() const
{
    return m_shell.isRing() && std::all_of(m_holes.begin(), m_holes.end(), [](auto &hole){return hole.isRing(); });
}

bool Polygon::CanMakeValid() const
{
    return m_shell.has_at_least_points(3) && std::all_of(m_holes.begin(), m_holes.end(), [](auto &hole){return hole.has_at_least_points(3); });
}

void Polygon::MakeValid()
{
    m_shell.EnsureRing();
    m_shell.ForceRHR();
    for (auto &_val : m_holes)
    {
        _val.EnsureRing();
    }
}
namespace
{
template <class T>
auto get_ring_for_vertex(T *poly, size_t idx)
{
    auto *seq = &poly->m_shell;
    if (size_t count = seq->size(); idx >= count)
    {
        for (auto &hole : poly->m_holes)
        {
            idx -= count;
            seq = &hole;
            if (count = seq->size(); idx < count)
            {
                break;
            }
        }
    }
    return std::make_pair(idx, seq);
}

}

std::pair<size_t, const CoordinateSeq *> Polygon::getRingForVertex(size_t idx) const
{
    return get_ring_for_vertex(this, idx);
}

std::pair<size_t, CoordinateSeq *> Polygon::getRingForVertex(size_t idx)
{
    return get_ring_for_vertex(this, idx);
}
}; // namespace geom
