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

#include "Point.h"
#include "CoordSeq.h"
#include "CoordinateFilter.h"

namespace geom
{

bool Point::Create(const CoordinateSeq &seq)
{
    if (!seq.empty())
    {
        seq.getFront(this->coordinate);
        return true;
    }
    return false;
}

double Point::distance(const glm::dvec2 &pos) const
{
    return glm::distance(glm::dvec2(coordinate.pos), pos);
}

void Point::apply_filter_rw(filter_rw &filter)
{
    coordinate = filter(coordinate);
}

bool Point::equals(const Geometry *geom) const
{
    if (const Point *p = geom->isPoint(); p != nullptr)
    {
        return coordinate == p->coordinate;
    }
    return false;
}

// this does nothing for points as point must have one element
void Point::Clear() {}

GEOSGeom_scoped_t Point::toGEOSGeom() const
{
    GEOSCoordSeq seq = GEOSCoordSeq_copyFromBuffer(&coordinate.pos.x, 1, hasZ(), hasM());
    return GEOSGeom_scoped_t(seq != nullptr ? GEOSGeom_createPoint(seq) : nullptr);
}

void Point::apply_filter_ro(filter_ro &filter) const
{
    filter.beginGeometry(*this);
    filter(coordinate);
    filter.endGeometry();
}

void Point::removeVertex(size_t)
{}

void Point::updateVertex(size_t idx, const Coordinate &coo)
{
    if (idx != 0)
    {
        throw std::logic_error("Point::updateVertex idx!=0");
    }

    this->coordinate = coo;
}

void Point::insertVertex(size_t, const Coordinate &)
{}

std::pair<size_t, const Geometry *> Point::getGeometryForVertex(size_t idx) const
{
    return std::make_pair(idx, this);
}
};//namespace geom{
