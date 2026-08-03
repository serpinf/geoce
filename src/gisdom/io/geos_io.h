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
#include "geom/Geometry.h"

namespace geom
{

class GEOSreader final : boost::noncopyable
{
public:
    explicit GEOSreader(CoordinateType coordType) : m_coordType(coordType)
    {}

    std::unique_ptr<Geometry> readGeometry(const GEOSGeometry *g1);

    std::unique_ptr<Geometry> readGeometry(const GEOSGeom_scoped_t &g1)
    {
        return readGeometry(g1.get());
    }

private:
    std::unique_ptr<Point> readPoint(const GEOSGeometry *g1);

    std::unique_ptr<MPoint> readMPoint(const GEOSGeometry *g1);

    std::unique_ptr<LineString> readLineString(const GEOSGeometry *g1);

    std::unique_ptr<MLineString> readMLineString(const GEOSGeometry *g1);

    std::unique_ptr<Polygon> readPolygon(const GEOSGeometry *g1);

    std::unique_ptr<MPolygon> readMPolygon(const GEOSGeometry *g1);

    std::unique_ptr<MGeometry> readMGeometry(const GEOSGeometry *g1);

    CoordinateType m_coordType;
};

}; //namespace geom
