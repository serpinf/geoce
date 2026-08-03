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

#include "io/wkbio.h"
#include <geom/Point.h>
#include <geom/MPoint.h>
#include <geom/LineString.h>
#include <geom/MLineString.h>
#include <geom/MPolygon.h>

#define G_TYPE(x) ((x)&0x0F)

static void skipbyte(const uint8_t **c)
{
    *c += 1;
}

static uint32_t popint(const uint8_t **c)
{
    uint32_t i;
    memcpy(&i, *c, 4);
    *c += 4;
    return i;
}

static void skipint(const uint8_t **c)
{
    *c += 4;
}

static double popdouble(const uint8_t **c)
{
    double d;
    memcpy(&d, *c, 8);
    *c += 8;
    return d;
}

namespace geom
{

std::unique_ptr<Geometry> WKBreader::readGeometry(CoordinateType cooType, const uint8_t *wkb, size_t size)
{
    m_ptr = wkb;
    //TODO: implement full wkb validation here, use size
    if (size == 0)
    {
        return {};
    }

    skipbyte(&m_ptr);// skip byteOrder
    uint32_t wkbType = popint(&m_ptr); // read wkb type
    if (wkbType & wkbSRID) skipint(&m_ptr); //skip srid

    switch (G_TYPE(wkbType))
    {
    case wkbPoint:
    {
        auto geom = std::make_unique<Point>(cooType);
        read(*geom, wkbType);
        return geom;
    }
    case wkbLineString:
    {
        auto geom = std::make_unique<LineString>(cooType);
        read(*geom, wkbType);
        return geom;
    }
    case wkbPolygon:
    {
        auto geom = std::make_unique<Polygon>(cooType);
        read(*geom, wkbType);
        return geom;
    }
    case wkbMultiPoint:
    {
        auto geom = std::make_unique<MPoint>(cooType);
        read<Point>(*geom, wkbType);
        return geom;
    }
    case wkbMultiLineString:
    {
        auto geom = std::make_unique<MLineString>(cooType);
        read<LineString>(*geom, wkbType);
        return geom;
    }
    case wkbMultiPolygon:
    {
        auto geom = std::make_unique<MPolygon>(cooType);
        read<Polygon>(*geom, wkbType);
        return geom;
    }
    /*case wkbGeometryCollection:
        res = readMGeometry(static_cast<MGeometry&>(geom)); break;
        */
    }
    return {};
}


void WKBreader::read(Point &geom, uint32_t wkbType)
{
    read(geom.getCoordinate(), wkbType);
}

void WKBreader::read(LineString &geom, uint32_t wkbType)
{
    read(geom.getCoordSeq(), wkbType, popint(&m_ptr));
}

void WKBreader::read(Polygon &geom, uint32_t wkbType)
{
    uint32_t nrings = popint(&m_ptr); //num_rings
    if (uint32_t i = 0; i < nrings)
    {
        read(geom.m_shell, wkbType, popint(&m_ptr));
        geom.m_holes.reserve(nrings - 1);
        while (++i < nrings)
        {
            auto &hole = geom.m_holes.emplace_back(geom.getCoordinateType());
            read(hole, wkbType, popint(&m_ptr));
        }
    }
}

template <typename Geom>
void WKBreader::read(MGeometry &geom, uint32_t /*wkbType*/)
{
    uint32_t npoint = popint(&m_ptr); // number of geometries in collection

    for (uint32_t i = 0; i < npoint; i++)
    {
        auto p = std::make_unique<Geom>(geom.getCoordinateType());

        skipbyte(&m_ptr);// skip byteOrder
        uint32_t wkbType = popint(&m_ptr); // read wkb type
        if (wkbType & wkbSRID) skipint(&m_ptr); //skip srid

        read(*p, wkbType);
        geom.collection.emplace_back(std::move(p));
    }
}

// helper functions to reed XY, XYZ, XYM or XYZM types
//
void WKBreader::read(Coordinate &c, uint32_t wkbType)
{
    c.pos.x = popdouble(&m_ptr);
    c.pos.y = popdouble(&m_ptr);
    if (wkbType & wkbZ) c.pos.z = popdouble(&m_ptr);
    if (wkbType & wkbM) c.m = popdouble(&m_ptr);
}

void WKBreader::read(CoordinateSeq &seq, uint32_t wkbType, size_t len)
{
    seq.resize(len);
    Coordinate c;
    for (size_t i = 0; i < len; i++)
    {
        read(c, wkbType);
        seq.set(c, i);
    }
}

} // namespace geom
