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
#include "geom/Box.h"
#include "geom/CoordSeq.h"
#include "geom/CoordinateFilter.h"
#include <GeographicLib/Geodesic.hpp>

enum class gceProjectionType
{
    // points in degrees are projected using sphere of R = 1.0 for 2D visualization
    // geocentric transform is performed using WGS84 ellipsoid
    WGS84_VISUALIZATION,
    PROJ_CUSTOM
};

class gceProjection
{
public:
    static std::unique_ptr<gceProjection> create(gceProjectionType type, const std::string &params);
    virtual ~gceProjection() = default;
    virtual bool toInternal(glm::dvec2 &dest, const glm::dvec2 &pos) const = 0;
    virtual bool toInternal(glm::dvec3 &dest, const glm::dvec3 &pos) const = 0;

    virtual bool fromInternal(glm::dvec2 &dest, const glm::dvec2 &pos) const = 0;
    virtual bool fromInternal(glm::dvec3 &dest, const glm::dvec3 &pos) const = 0;

    virtual void computeMetrics(const geom::CoordinateSeq &seq, double &perimeter, double &area, bool polyline) const = 0;

    virtual geom::Box2D getExtent() const = 0;

    virtual std::string getProjName() = 0;
    std::string getInternalCSName();

    /*!
     * @brief get projection scale at point
     * @param p latitude, longitude in degrees
     * @return scale to meters
     */
    virtual double getScale(const glm::dvec2 &p) const = 0;
    double getScaleFromProjected(const glm::dvec2 &p) const;

    void computeMetrics(const geom::Geometry &g, double &perimeter, double &area) const;

    /*!
     * @brief formats geographic position to string
     * @param pos lat, lon, height
     * @return string representation
     */
    static std::string formatPosition(const glm::dvec3 &pos)
    {
        return fmt::format("{:.6f}, {:.6f}, {:.2f}m", pos.x, pos.y, pos.z);
    }

    static std::string formatLength(double len);

    static std::string formatArea(double area);

private:
};

namespace gce
{
namespace earth
{
/*!
* @brief distance in meters betwieen 2 geodesic points
* @param A
* @param B
* @return
*/
double distance(const glm::dvec2 &A, const glm::dvec2 &B);
//double inverse(const glm::dvec2 &A, const glm::dvec2 &B, double &azi1);

glm::dvec2 position(const glm::dvec2 &pos1, const glm::dvec2 &pos2, double length);

glm::dvec2 position(const glm::dvec2 &pos, double azi, double length);

const GeographicLib::Geodesic &getGeodesic();

}
}

class unproject_filter : public geom::filter_rw
{
public:
    explicit unproject_filter(const gceProjection *proj) : m_proj(proj) {}
    geom::Coordinate operator()(const geom::Coordinate &coo) override
    {
        geom::Coordinate res = coo;
        m_proj->toInternal(res.pos, coo.pos);
        return res;
    }
private:
    const gceProjection *m_proj;
};

class project_filter : public geom::filter_rw
{
public:
    explicit project_filter(const gceProjection *proj) : m_proj(proj) {}
    geom::Coordinate operator()(const geom::Coordinate &coo) override
    {
        geom::Coordinate res = coo;
        m_proj->fromInternal(res.pos, coo.pos);
        return res;
    }
private:
    const gceProjection *m_proj;
};
