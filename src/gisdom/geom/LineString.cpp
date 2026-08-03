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

#include "LineString.h"
#include "CoordinateFilter.h"

//
namespace geom
{

bool LineString::Create(const CoordinateSeq &ls)
{
    if (ls.empty() || ls.size() > 1)
    {
        coordinates.assign(ls);
        return true;
    }
    return false;
}

void LineString::removeVertex(size_t idx)
{
    size_t cnt = coordinates.size();
    if (idx < cnt)
    {
        if (cnt < 3)
        {
            throw std::logic_error("LineString may not contain less then 2 points");
        }
        coordinates.Erase(idx);
    }
}

void LineString::updateVertex(size_t idx, const Coordinate &coo)
{
    if (idx < coordinates.size())
    {
        coordinates.set(coo, idx);
    }
}

void LineString::insertVertex(size_t idx, const Coordinate &coo)
{
    if (idx < coordinates.size())
    {
        coordinates.Insert(idx, coo);
    }
}

bool LineString::equals(const Geometry *geom) const
{
    if (const LineString *const ls = geom->isLineString(); ls)
    {
        return coordinates == ls->coordinates;
    }
    return false;
}

GEOSGeom_scoped_t LineString::toGEOSGeom() const
{
    return GEOSGeom_scoped_t{GEOSGeom_createLineString(coordinates.toGEOSCoordSeq())};
}

std::pair<size_t, const Geometry *> LineString::getGeometryForVertex(size_t idx) const
{
    return std::make_pair(idx, this);
}

void LineString::apply_filter_rw(filter_rw &filter)
{
    coordinates.apply_filter_rw(filter);
}

void LineString::apply_filter_ro(filter_ro &filter) const
{
    filter.beginGeometry(*this);
    coordinates.apply_filter_ro(filter);
    filter.endGeometry();
}

};//namespace geom{
