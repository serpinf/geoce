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

#include <io/geos_io.h>
#include <geom/Point.h>
#include <geom/MPoint.h>
#include <geom/LineString.h>
#include <geom/MLineString.h>
#include <geom/MPolygon.h>
namespace geom
{
static inline void readCoord(Coordinate &coo, const GEOSCoordSequence *geosSeq)
{
    unsigned int len;
    GEOSCoordSeq_getSize(geosSeq, &len);
    if (len == 1)
    {
        GEOSCoordSeq_copyToBuffer(geosSeq, &coo.pos.x, true, true);
    }
}

std::unique_ptr<Geometry> GEOSreader::readGeometry(const GEOSGeometry *g1)
{
    switch (GEOSGeomTypeId(g1))
    {
    case GEOS_POINT:
        return readPoint(g1);
    case GEOS_MULTIPOINT:
        return readMPoint(g1);
    case GEOS_LINESTRING:
        return readLineString(g1);
    case GEOS_MULTILINESTRING:
        return readMLineString(g1);
    case GEOS_POLYGON:
        return readPolygon(g1);
    case GEOS_MULTIPOLYGON:
        return readMPolygon(g1);
    case GEOS_GEOMETRYCOLLECTION:
        return readMGeometry(g1);
    }
    return {};
}


std::unique_ptr<Point> GEOSreader::readPoint(const GEOSGeometry *g1)
{
    auto g = std::make_unique<Point>(m_coordType);
    readCoord(g->getCoordinate(), GEOSGeom_getCoordSeq(g1));
    return g;
}

std::unique_ptr<MPoint> GEOSreader::readMPoint(const GEOSGeometry *g1)
{
    auto g = std::make_unique<MPoint>(m_coordType);

    int ngeom = GEOSGetNumGeometries(g1);
    g->collection.reserve(ngeom);
    for (int n = 0; n < ngeom; n++)
    {
        g->collection.emplace_back(readPoint(GEOSGetGeometryN(g1, n)));
    }
    return g;
}


std::unique_ptr<LineString> GEOSreader::readLineString(const GEOSGeometry *g1)
{
    auto g = std::make_unique<LineString>(m_coordType);
    g->getCoordSeq().fromGEOSCoordSeq(GEOSGeom_getCoordSeq(g1));
    return g;
}

std::unique_ptr<MLineString> GEOSreader::readMLineString(const GEOSGeometry *g1)
{
    auto g = std::make_unique<MLineString>(m_coordType);

    int ngeom = GEOSGetNumGeometries(g1);
    g->collection.reserve(ngeom);
    for (int n = 0; n < ngeom; n++)
    {
        g->collection.emplace_back(readLineString(GEOSGetGeometryN(g1, n)));
    }
    return g;
}

std::unique_ptr<Polygon> GEOSreader::readPolygon(const GEOSGeometry *g1)
{
    auto g = std::make_unique<Polygon>(m_coordType);

    // read exterior ring
    g->m_shell.fromGEOSCoordSeq(GEOSGeom_getCoordSeq(GEOSGetExteriorRing(g1)));

    // read interior rings
    int nrings = GEOSGetNumInteriorRings(g1);
    g->m_holes.reserve(nrings);
    for (int n = 0; n < nrings; n++)
    {
        auto &hole = g->m_holes.emplace_back(m_coordType);
        hole.fromGEOSCoordSeq(GEOSGeom_getCoordSeq(GEOSGetInteriorRingN(g1, n)));
    }
    return g;
}

std::unique_ptr<MPolygon> GEOSreader::readMPolygon(const GEOSGeometry *g1)
{
    auto g = std::make_unique<MPolygon>(m_coordType);

    int ngeom = GEOSGetNumGeometries(g1);
    g->collection.reserve(ngeom);
    for (int n = 0; n < ngeom; n++)
    {
        g->collection.emplace_back(readPolygon(GEOSGetGeometryN(g1, n)));
    }
    return g;
}

std::unique_ptr<MGeometry> GEOSreader::readMGeometry(const GEOSGeometry *g1)
{
    auto g = std::make_unique<MGeometry>(m_coordType);

    int ngeom = GEOSGetNumGeometries(g1);
    g->collection.reserve(ngeom);
    for (int n = 0; n < ngeom; n++)
    {
        g->collection.emplace_back(readGeometry(GEOSGetGeometryN(g1, n)));
    }
    return g;
}

}; //namespace geom
