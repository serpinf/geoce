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

#include <geom/MGeometry.h>
#include <geom/CoordinateFilter.h>

namespace geom
{

MGeometry::MGeometry(const MGeometry &other) : Geometry(other)
{
    for (auto &_val : other)
    {
        collection.emplace_back(_val->clone());
    }
}

bool MGeometry::isEmpty() const
{
    for (auto &_val : collection)
    {
        if (!_val->isEmpty())
        {
            return false;
        }
    }
    return true;
}

int MGeometry::getDimension() const
{
    int dimension = -1;
    for (auto &_val : *this)
    {
        dimension = std::max(dimension, _val->getDimension());
    }
    return dimension;
}

double MGeometry::distance(const glm::dvec2 &pos) const
{
    double dist = DoubleInfinity;
    for (auto &g : *this)
    {
        dist = std::min(dist, g->distance(pos));
    }
    return dist;
}

size_t MGeometry::getNumGeometries() const
{
    return this->size();
}

Geometry *MGeometry::getGeometryN(size_t n)
{
    return collection[n].get();
}

const Geometry *MGeometry::getGeometryN(size_t n) const
{
    return collection[n].get();
}

void MGeometry::apply_geometry_filter(GeometryFilter &filter) const
{
    for (auto &_val : *this)
    {
        _val->apply_geometry_filter(filter);
    }
}

size_t MGeometry::getNumPoints() const
{
    size_t numPoints = 0;
    for (auto &_val : *this)
    {
        numPoints += _val->getNumPoints();
    }
    return numPoints;
}

bool MGeometry::isSimple() const
{
    return false;
}

double MGeometry::area() const
{
    double area = 0.0;
    for (auto &_val : *this)
    {
        area += _val->area();
    }
    return area;
}

double MGeometry::length() const
{
    double sum = 0.0;
    for (auto &_val : *this)
    {
        sum += _val->length();
    }
    return sum;
}


double MGeometry::length3d() const
{
    double sum = 0.0;
    for (auto &_val : *this)
    {
        sum += _val->length3d();
    }
    return sum;
}


MGeometry::~MGeometry()
{}

gceGeometryType MGeometry::getGeometryTypeId() const
{
    return gceGeometryType::GeometryCollection;
}

bool MGeometry::equals(const Geometry *geom) const
{
    if (const MGeometry *mgeom = geom->isMGeometry(); mgeom != nullptr && size() == mgeom->size())
    {
        return std::equal(collection.begin(), collection.end(), mgeom->collection.begin(), [](auto &a, auto &b){ return a->equals(b.get()); });
    }
    return false;
}

GEOSGeom_scoped_t MGeometry::toGEOSGeom() const
{
    int type = 0;
    switch (getGeometryTypeId())
    {
    case gceGeometryType::GeometryCollection:
        type = GEOS_GEOMETRYCOLLECTION;
        break;
    case gceGeometryType::MultiPoint:
        type = GEOS_MULTIPOINT;
        break;
    case gceGeometryType::MultiLineString:
        type = GEOS_MULTILINESTRING;
        break;
    case gceGeometryType::MultiPolygon:
        type = GEOS_MULTIPOLYGON;
        break;
    default:
        throw std::logic_error("unexpected geometry type for MGeometry");
    }

    std::vector<GEOSGeom> geoms;
    geoms.reserve(this->size());
    for (auto &gtmp : *this)
    {
        auto g = gtmp->toGEOSGeom();
        if (g)
        {
            geoms.emplace_back(g.release());
        }
    }
    return GEOSGeom_scoped_t{GEOSGeom_createCollection(type, geoms.data(), (unsigned)geoms.size())};
}

void MGeometry::apply_filter_rw(filter_rw &filter)
{
    for (auto &_val : *this)
    {
        _val->apply_filter_rw(filter);
    }
}

void MGeometry::apply_filter_ro(filter_ro &filter) const
{
    filter.beginGeometry(*this);
    for (auto &_val : *this)
    {
        _val->apply_filter_ro(filter);
    }
    filter.endGeometry();
}

std::pair<size_t, const Geometry *> MGeometry::getGeometryForVertex(size_t idx) const
{
    for (auto &_val : *this)
    {
        size_t count = _val->getNumPoints();
        if (idx < count)
        {
            return _val->getGeometryForVertex(idx);
        }
        idx -= count;
    }
    return {0, nullptr};
}

void MGeometry::removeVertex(size_t idx)
{
    if (auto [_idx, _geom] = this->getCollectionElementForVertex(idx); _geom != nullptr)
    {
        _geom->removeVertex(_idx);
    }
}

void MGeometry::updateVertex(size_t idx, const Coordinate &coo)
{
    if (auto [_idx, _geom] = this->getCollectionElementForVertex(idx); _geom != nullptr)
    {
        _geom->updateVertex(_idx, coo);
    }
}

void MGeometry::insertVertex(size_t idx, const Coordinate &coo)
{
    if (auto [_idx, _geom] = this->getCollectionElementForVertex(idx); _geom != nullptr)
    {
        _geom->insertVertex(_idx, coo);
    }
}

std::pair<size_t, Geometry *> MGeometry::getCollectionElementForVertex(size_t idx)
{
    for (auto &_val : *this)
    {
        size_t count = _val->getNumPoints();
        if (idx < count)
        {
            return {idx, _val.get()};
        }
        idx -= count;
    }
    return {0, nullptr};
}

bool MGeometry::isValid() const
{
    for (auto &_val : *this)
    {
        if (!_val->isValid())
        {
            return false;
        }
    }
    return true;
}

bool MGeometry::CanMakeValid() const
{
    for (auto &_val : *this)
    {
        if (!_val->CanMakeValid())
        {
            return false;
        }
    }
    return true;
}

void MGeometry::MakeValid()
{
    for (auto &_val : *this)
    {
        _val->MakeValid();
    }
}
};//namespace geom{
