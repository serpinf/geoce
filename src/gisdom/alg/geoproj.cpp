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
#include "geoproj.h"
#include "wgsop.h"
#include <GeographicLib/Geodesic.hpp>
#include <GeographicLib/Geocentric.hpp>
#include <GeographicLib/GeodesicLine.hpp>
#include <GeographicLib/PolygonArea.hpp>
#include "geom/LineString.h"
#include "geom/Polygon.h"
#include "engine.hpp"

class gceProjectionDefault final : public gceProjection
{
public:
    bool toInternal(glm::dvec2 &dest, const glm::dvec2 &pos) const override
    {
        dest = wgsop::flat2wgs_degrees(pos);
        return true;
    }
    bool toInternal(glm::dvec3 &dest, const glm::dvec3 &pos) const override
    {
        dest = wgsop::flat2wgs_degrees(pos);
        return true;
    }

    bool fromInternal(glm::dvec2 &dest, const glm::dvec2 &pos) const override
    {
        dest = wgsop::wgs_degrees2flat(pos);
        return true;

    }
    bool fromInternal(glm::dvec3 &dest, const glm::dvec3 &pos) const override
    {
        dest = wgsop::wgs_degrees2flat(pos);
        return true;

    }

    std::string getProjName() override
    {
        return "Visualization mercator";
    }

    geom::Box2D getExtent() const override
    {
        return geom::Box2D(glm::dvec2(-M_PI), glm::dvec2(M_PI));
    }
    void computeMetrics(const geom::CoordinateSeq &seq, double &perimeter, double &area, bool polyline) const override
    {
        const size_t seqSize = seq.size();

        if (seqSize < 2)
        {
            area = 0.0;
            perimeter = 0.0;
        }
        else
        {
            try
            {
                GeographicLib::PolygonArea poly(geod, seqSize > 2 ? polyline : true);
                geom::CoordinateXY A;
                for (size_t n = 0; n < seqSize; ++n)
                {
                    seq.get(A, n);
                    poly.AddPoint(A.pos.y, A.pos.x);
                }
                (void)poly.Compute(false, true, perimeter, area);
            }
            catch (const std::exception &e)
            {
                gceContext::log_error("computeMetrics error: {}", e.what());
            }
        }
    }

    double getScale(const glm::dvec2 &p) const override
    {
        return wgsop::wgs2scale(glm::radians(p.y));
    }

    GeographicLib::Geodesic geod{GeographicLib::Constants::WGS84_a(), GeographicLib::Constants::WGS84_f()};
    GeographicLib::Geocentric earth{GeographicLib::Constants::WGS84_a(), GeographicLib::Constants::WGS84_f()};
};


std::string gceProjection::getInternalCSName()
{
    return "WGS84";
}

double gceProjection::getScaleFromProjected(const glm::dvec2 &p) const
{
    glm::dvec2 latlon{0.0};
    if (toInternal(latlon, p))
    {
        return getScale(latlon);
    }
    return 1.0;
}

struct GeometryStatsAccumulator final : public geom::GeometryFilter
{
    GeometryStatsAccumulator(const gceProjection *proj) : m_proj(proj) {}

    void operator()(const geom::Point &) final
    {}

    void operator()(const geom::LineString &g) final
    {
        process(g.getCoordSeq(), true, 0.0);
    }

    void operator()(const geom::Polygon &poly) final
    {
        process(poly.m_shell, false, 1.0);
        for (auto &hole : poly.m_holes)
        {
            process(hole, false, -1.0);
        }
    }

    void process(const geom::CoordinateSeq &seq, bool polyline, double areaSign)
    {
        double area = 0.0, perimeter = 0.0;
        m_proj->computeMetrics(seq, perimeter, area, polyline);
        m_area += areaSign * std::abs(area);
        m_perimeter += perimeter;
    }
    const gceProjection *m_proj;
    double m_area = 0.0;
    double m_perimeter = 0.0;
};

void gceProjection::computeMetrics(const geom::Geometry &g, double &perimeter, double &area) const
{
    GeometryStatsAccumulator f(this);
    g.apply_geometry_filter(f);
    perimeter = f.m_perimeter;
    area = f.m_area;
}

std::string gceProjection::formatLength(double len)
{
    if (len < 1.0e+4)
    {
        return fmt::format("{:.6g}m", len);
    }
    return fmt::format("{:.7g}km", len / 1000);
}

std::string gceProjection::formatArea(double area)
{
    if (area < 1.0e+6)
    {
        return fmt::format("{:.6g}m²", area);
    }
    return fmt::format("{:.7g}km²", area / 1e6);
}

std::unique_ptr<gceProjection> gceProjection::create(gceProjectionType type, const std::string &)
{
    switch (type)
    {
    case gceProjectionType::WGS84_VISUALIZATION:
        return std::make_unique<gceProjectionDefault>();
    case gceProjectionType::PROJ_CUSTOM:
        return {};
    }
    return {};
}
namespace gce
{
namespace earth
{
static const auto &geod = GeographicLib::Geodesic::WGS84();

double distance(const glm::dvec2 &A, const glm::dvec2 &B)
{
    double d;
    geod.Inverse(A.y, A.x, B.y, B.x, d);
    return d;
}

glm::dvec2 position(const glm::dvec2 &pos1, const glm::dvec2 &pos2, double length)
{
    auto line = geod.InverseLine(pos1.y, pos1.x, pos2.y, pos2.x);
    double lat, lon;
    line.Position(length, lat, lon);
    return {lon, lat};
}
glm::dvec2 position(const glm::dvec2 &pos, double azi, double length)
{
    double lat, lon;
    geod.Direct(pos.y, pos.x, azi, length, lat, lon);
    return {lon, lat};
}
const GeographicLib::Geodesic &getGeodesic()
{
    return geod;
}
}
}
