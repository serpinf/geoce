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

#include "Geometry.h" // class's header file
#include "CoordinateFilter.h"
#include "io/geos_io.h"
#include "CoordSeq.h"
#include "Point.h"
#include "MPoint.h"
#include "LineString.h"
#include "Polygon.h"

namespace geom
{
std::unique_ptr<Geometry> Geometry::Create(int dimension, CoordinateType cooType, const CoordinateSeq &seq)
{
    switch (dimension)
    {
    case 0:
        if (seq.size() == 1)
        {
            if (auto pgeom = std::make_unique<Point>(cooType); pgeom->Create(seq))
            {
                return pgeom;
            }
        }
        else if (auto pgeom = std::make_unique<MPoint>(cooType); pgeom->Create(seq))
        {
            return pgeom;
        }
        break;

    case 1:
        if (auto pgeom = std::make_unique<LineString>(cooType); pgeom->Create(seq))
        {
            return pgeom;
        }
        break;

    case 2:
        if (auto pgeom = std::make_unique<Polygon>(cooType); pgeom->Create(seq))
        {
            return pgeom;
        }
        break;
    }

    return {};
}

std::unique_ptr<Geometry> Geometry::Create(CoordinateType cooType, const GEOSGeom_scoped_t &geosGeom)
{
    if (geosGeom)
    {
        return GEOSreader(cooType).readGeometry(geosGeom);
    }
    return {};
}

geom::Box3D Geometry::bbox() const
{
    box_filter _filter;
    apply_filter_ro(_filter);
    return _filter.bbox;
}

const Geometry &Geometry::affine(const glm::dmat4 &m)
{
    affine_filter _filter(m);
    apply_filter_rw(_filter);
    return *this;
}
void Geometry::Move(const glm::dvec3 &offset)
{
    move_filter _filter(offset);
    apply_filter_rw(_filter);
}

void Geometry::RotateAroundBase(const glm::dvec2 &BaseCoord, double Angle)
{
    rotate_filter _filter(BaseCoord, Angle);
    apply_filter_rw(_filter);
}

glm::dvec3 Geometry::GetBaseCoord() const
{
    return bbox().GetCenter();
}
struct GEOSWKTWriter_destroyer
{
    void operator()(GEOSWKTWriter *writer) { GEOSWKTWriter_destroy(writer); }
};
struct GEOSFree_destroyer
{
    void operator()(char *str) { GEOSFree(str); }
};
std::string Geometry::toWKT() const
{
    if (auto A = this->toGEOSGeom(); A)
    {
        std::unique_ptr < GEOSWKTWriter, GEOSWKTWriter_destroyer > writer{GEOSWKTWriter_create()};
        if (writer)
        {
            std::unique_ptr<char, GEOSFree_destroyer > wkt{GEOSWKTWriter_write(writer.get(), A.get())};
            if (wkt)
            {
                return std::string(wkt.get());
            }
        }
    }
    return {};
}

bool Geometry::isValid() const
{
    return true;
}

bool Geometry::Create(const CoordinateSeq &)
{
    return false;
}

bool Geometry::CanMakeValid() const
{
    return isValid();
}

void Geometry::MakeValid()
{}

};//namespace geom
