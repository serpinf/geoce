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
#include "geom/Point.h"
#include "geom/LineString.h"
#include "geom/Polygon.h"
#include "geom/MGeometry.h"

inline void putbyte(char **c, uint8_t val)
{
    *((*c)++) = val;
}

inline void putint(char **c, uint32_t val)
{
    memcpy(*c, &val, 4);
    *c += 4;
}

inline void putdouble(char **c, double val)
{
    memcpy(*c, &val, 8);
    *c += 8;
}

namespace geom
{

class wkb_SizeFinder
{
public:
    explicit wkb_SizeFinder(size_t pointSize) : pointSize(pointSize) {}

    void sizeWkb(size_t &sz, const Point &) const
    {
        // byteOrder+wkbType+SRID+1*pointSize
        sz += 9 + pointSize;
    }

    void sizeWkb(size_t &sz, const LineString &g) const
    {
        // byteOrder:1 + wkbType:4 + SRID:4 + numPoints:4 + numPoints*pointSize
        sz += 13 + g.getNumPoints() * pointSize;
    }

    void sizeWkb(size_t &sz, const Polygon &g) const
    {
        sz += 13;// byteOrder:1+wkbType:4+SRID:4+numRings:4
        // enum all rings
        sz += 4 + g.m_shell.size() * pointSize;
        for (auto &pRing : g.m_holes)
        {
            // numPoints:4 + 
            sz += 4 + pRing.size() * pointSize;
        }
    }

    void sizeWkb(size_t &sz, const Geometry &geom) const
    {
        switch (geom.getGeometryTypeId())
        {
        case gceGeometryType::Point:
            sizeWkb(sz, static_cast<const Point &>(geom)); break;
        case gceGeometryType::LineString:
            sizeWkb(sz, static_cast<const LineString &>(geom)); break;
        case gceGeometryType::Polygon:
            sizeWkb(sz, static_cast<const Polygon &>(geom)); break;
        case gceGeometryType::GeometryCollection:
        case gceGeometryType::MultiPoint:
        case gceGeometryType::MultiLineString:
        case gceGeometryType::MultiPolygon:
            sizeWkb(sz, static_cast<const MGeometry &>(geom)); break;
        }
    }

    void sizeWkb(size_t &sz, const MGeometry &g) const
    {
        sz += 13;// byteOrder+wkbType+SRID+numGeometries

        //enum geometries sizes
        for (auto &pGeom : g)
        {
            sizeWkb(sz, *pGeom);
        }
    }
private:
    const size_t pointSize;
};
size_t WKBwriter::sizeGeometry(const Geometry *geom)
{
    size_t sz = 0;
    if (geom != nullptr)
    {
        wkb_SizeFinder szfinder(sizeof(double) * dimensions(geom->getCoordinateType()));
        szfinder.sizeWkb(sz, *geom);
    }
    return sz;
}

size_t WKBwriter::write(char *buf, const Geometry &geom)
{
    m_ptr = buf;
    m_wkbFlags = 0;
    if (hasZ(geom.getCoordinateType())) m_wkbFlags |= wkbZ;
    if (hasM(geom.getCoordinateType())) m_wkbFlags |= wkbM;

    write(geom);

    return (m_ptr - buf);
}
void WKBwriter::write(const Geometry &geom)
{
    //::perfCheckMsg("WKBwriter::addGeometry");
    gceGeometryType gType = geom.getGeometryTypeId();
    switch (gType)
    {
    case gceGeometryType::Point:
        write(static_cast<const Point &>(geom));
        break;
    case gceGeometryType::LineString:
        write(static_cast<const LineString &>(geom));
        break;
    case gceGeometryType::Polygon:
        write(static_cast<const Polygon &>(geom));
        break;
    case gceGeometryType::MultiPoint:
    case gceGeometryType::MultiLineString:
    case gceGeometryType::MultiPolygon:
    case gceGeometryType::GeometryCollection:
        write(static_cast<const MGeometry &>(geom), to_wkbType(gType));
        break;
    }
}

void WKBwriter::write(const Point &geom)
{
    putbyte(&m_ptr, wkbNDR);	// byteOrder
    putint(&m_ptr, uint32_t(wkbPoint) | uint32_t(wkbSRID) | m_wkbFlags); // wkbType
    putint(&m_ptr, m_SRID); // SRID

    write(geom.getCoordinate());
}

void WKBwriter::write(const LineString &geom)
{
    putbyte(&m_ptr, wkbNDR);	// byteOrder
    putint(&m_ptr, wkbLineString | wkbSRID | m_wkbFlags);// wkbType
    putint(&m_ptr, m_SRID);// SRID

    write(geom.getCoordSeq());
}

void WKBwriter::write(const Polygon &geom)
{
    putbyte(&m_ptr, wkbNDR);	// byteOrder
    putint(&m_ptr, wkbPolygon | wkbSRID | m_wkbFlags);// wkbType
    putint(&m_ptr, m_SRID);// SRID

    putint(&m_ptr, (uint32_t)geom.m_holes.size() + 1);// num_rings = shell + holes
    write(geom.m_shell);
    for (auto &pRing : geom.m_holes)
    {
        write(pRing);
    }
}

void WKBwriter::write(const MGeometry &geom, wkbGeometryType collectionType)
{
    putbyte(&m_ptr, wkbNDR);	// byteOrder
    putint(&m_ptr, collectionType | wkbSRID | m_wkbFlags);// wkbType
    putint(&m_ptr, m_SRID);// SRID

    putint(&m_ptr, (uint32_t)geom.size());// num_wkbGeometries
    for (auto &pGeom : geom)
    {
        write(*pGeom);
    }
}

//	Helper funcs (protected)
//
void WKBwriter::write(const Coordinate &c)
{
    putdouble(&m_ptr, c.pos.x);
    putdouble(&m_ptr, c.pos.y);
    if (m_wkbFlags & wkbZ)	putdouble(&m_ptr, c.pos.z);
    if (m_wkbFlags & wkbM)	putdouble(&m_ptr, c.m);
}

void WKBwriter::write(const CoordinateSeq &seq)
{
    size_t sz = seq.size();
    putint(&m_ptr, (int)sz);// num_points
    CoordinateXYZM c;
    for (size_t i = 0; i < sz; i++)
    {
        seq.get(c, i);
        write(c);
    }
}


};//namespace geom{
