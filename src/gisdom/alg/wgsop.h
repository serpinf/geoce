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
#define USE_SPHEROID 1
class IScene;
namespace wgsop
{
const double M_2PI = 6.283185307179586476925286766559;
#if USE_SPHEROID
const double Ra = 6378137.0; // equator radius
const double Rb = 6356752.314245; // pole radius
const double sqr_e = 0.00669438; // squred eccentricity of WGS84
#else
const double Ra = 6371000.0; // equator radius
const double Rb = 6371000.0; // pole radius
const double sqr_e = 0.0; // squred eccentricity of WGS84
#endif
const double FL = (Ra - Rb) / Ra; //flattering

/** lat lon heignt -> xyz conversion
*/
void wgs2xyz(glm::dvec3 &xyz, double lat, double lon, double height);
void wgs2xyz_up(glm::dvec3 &xyz, glm::dvec3 &up, const glm::dvec2 &pos, double h);

/**
 * @param up [out] receives up vector
 * @param lon wgs position: longitude in radians)
 * @param lat wgs position: latitude in radians
 */
void wgs2up(glm::dvec3 &up, double lon, double lat);

/**
 * Calculates tangent space basis for given wgs position on the Globe
 *
 * @param east [out] receives east vector
 * @param north [out] receives north vector
 * @param up [out] receives up vector
 * @param lon wgs position: longitude in radians)
 * @param lat wgs position: latitude in radians
 */
void wgs2ts(glm::dvec3 &east, glm::dvec3 &north, glm::dvec3 &up, double lon, double lat);

inline double wgs2flat_Y(double lat)
{
    return log(fabs(tan(M_PI / 4 + lat / 2)));
}

inline double flat2wgs_Y(double v)
{
    return 2.0 * (std::atan(std::exp(v)) - M_PI / 4);
}

inline glm::dvec2 flat2wgs_degrees(const glm::dvec2 &A)
{
    return glm::degrees(glm::dvec2(A.x, flat2wgs_Y(A.y)));
}

inline glm::dvec3 flat2wgs_degrees(const glm::dvec3 &A)
{
    return glm::degrees(glm::dvec3(A.x, flat2wgs_Y(A.y), A.z));
}

inline glm::dvec2 wgs2flat(const glm::dvec2 &wgs)
{
    return glm::dvec2(wgs.x, wgs2flat_Y(wgs.y));
}

inline glm::dvec2 wgs_degrees2flat(const glm::dvec2 &wgs)
{
    return {glm::radians(wgs.x), wgs2flat_Y(glm::radians(wgs.y))};
}

inline glm::dvec3 wgs2flat(const glm::dvec3 &wgs)
{
    return glm::dvec3(wgs.x, wgs2flat_Y(wgs.y), wgs.z);
}

inline glm::dvec3 wgs_degrees2flat(const glm::dvec3 &wgs)
{
    return {glm::radians(wgs.x), wgs2flat_Y(glm::radians(wgs.y)), wgs.z};
}

inline geom::Box2D wgs2flat(const geom::Box2D &wgsBox)
{
    geom::Box2D flatBox(wgsBox);
    flatBox.cmin.y = wgs2flat_Y(flatBox.cmin.y);
    flatBox.cmax.y = wgs2flat_Y(flatBox.cmax.y);
    return flatBox;
}

inline geom::Box3D wgs2flat(const geom::Box3D &wgsBox)
{
    geom::Box3D flatBox(wgsBox);
    flatBox.cmin.y = wgs2flat_Y(flatBox.cmin.y);
    flatBox.cmax.y = wgs2flat_Y(flatBox.cmax.y);
    return flatBox;
}

inline glm::dvec2 flat2wgs(const glm::dvec2 &flat)
{
    return glm::dvec2(flat.x, flat2wgs_Y(flat.y));
}

inline geom::Box2D flat2wgs(const geom::Box2D &flatBox)
{
    geom::Box2D wgsBox(flatBox);
    wgsBox.cmin.y = flat2wgs_Y(wgsBox.cmin.y);
    wgsBox.cmax.y = flat2wgs_Y(wgsBox.cmax.y);
    return wgsBox;
}

/**
* get radius based scale from flat UV to metric
*/
inline double flat2scale(double v)
{
    return Ra * cos(flat2wgs_Y(v));
}

/**
* get radius based scale from wgs radians to metric (for flatCS)
*/
inline double wgs2scale(double lat)
{
    return Ra * cos(lat);
}
// geocentic xyz to wgs (degrees) lon, lat, altitude 
glm::dvec3 xyz2wgs(const glm::dvec3 &pos);

inline void xyz2flat(glm::dvec3 &dest, const glm::dvec3 &src)
{
    dest = xyz2wgs(src);
    dest.x = glm::radians(dest.x);
    dest.y = wgs2flat_Y(glm::radians(dest.y));
}

/**
@param dest - out XYZ
@param u - horizontal flat coord
@param v - vertical flat coord
@param height -
*/
inline void flat2xyz(glm::dvec3 &dest, double u, double v, double height)
{
    wgs2xyz(dest, flat2wgs_Y(v), u, height);
}

inline glm::dvec2 wgs2tex(const glm::dvec2 &pos)
{
    return glm::dvec2(pos.x, wgs2flat_Y(pos.y)) / M_2PI + 0.5;
}

struct toWGS_notransform
{
    double X(double x) const
    {
        return x;
    }
    double Y(double y) const
    {
        return y;
    }
};
struct toWGS_fromflat
{
    double X(double x) const
    {
        return x;
    }
    double Y(double y) const
    {
        return flat2wgs_Y(y);
    }
};

/**
 * supports very efficient way to convert regular lat/lon oriented grid to geocentric space,
 * based precomputed tables for 1 row and 1 column
 */
template <int N, int M, typename toWGS_transform>
class xyz_grid
{
public:
    struct LON
    {
        double cos_lon;
        double sin_lon;
    };
    struct LAT
    {
        double cos_lat;
        double sin_lat;
        double Rn;
    };
    xyz_grid()
    {}

    xyz_grid(double xmin, double ymin, double xmax, double ymax)
    {
        init(xmin, ymin, xmax, ymax);
    }

    void init(double xmin, double ymin, double xmax, double ymax)
    {
        toWGS_transform transform;
        for (int n = 0; n < N; ++n)
        {
            double lat = transform.Y(glm::mix(ymin, ymax, double(n) / (N - 1)));

            lata[n].sin_lat = sin(lat);
            lata[n].cos_lat = cos(lat);
            lata[n].Rn = Ra / (sqrt(1.0 - sqr_e * lata[n].sin_lat * lata[n].sin_lat));
        }
        for (int m = 0; m < M; ++m)
        {
            double lon = transform.X(glm::mix(xmin, xmax, double(m) / (M - 1)));

            lona[m].sin_lon = sin(lon);
            lona[m].cos_lon = cos(lon);
        }
    }
    void xyz_at2i(glm::dvec3 &xyz, int n, int m, double height) const
    {
        const double g1 = (lata[n].Rn + height) * lata[n].cos_lat;
        const double g2 = ((lata[n].Rn * (1.0 - sqr_e)) + height) * lata[n].sin_lat;

        get_xyz(xyz, g1, g2, m);
    }

    glm::dvec3 xyz_at2i(int n, int m, double height) const
    {
        const double g1 = (lata[n].Rn + height) * lata[n].cos_lat;
        const double g2 = ((lata[n].Rn * (1.0 - sqr_e)) + height) * lata[n].sin_lat;

        return glm::dvec3{g1 * lona[m].cos_lon, g1 * lona[m].sin_lon, g2};
    }

    void xyz_at2i(glm::dvec3 &xyz, int n, int m) const
    {
        const double g1 = lata[n].Rn * lata[n].cos_lat;
        const double g2 = lata[n].Rn * (1.0 - sqr_e) * lata[n].sin_lat;

        get_xyz(xyz, g1, g2, m);

    }
    inline glm::fvec3 up_at2i(int n, int m)
    {
        return glm::fvec3(lona[m].cos_lon * lata[n].cos_lat, lona[m].sin_lon * lata[n].cos_lat, lata[n].sin_lat);
    }
    inline void get_xyz(glm::dvec3 &xyz, double g1, double g2, int m) const
    {
        xyz[0] = g1 * lona[m].cos_lon;
        xyz[1] = g1 * lona[m].sin_lon;
        xyz[2] = g2;
    }
    LAT lata[N];
    LON lona[M];
};
class Location
{
public:
    /**
    * constructs Location object, using spheroid-based height
    *
    * @param height height used for placement
    * @param pos wgs position (lon, lat)
    */
    Location(double height, const glm::dvec2 &pos) : Location(pos.x, pos.y, height)
    {}
    /*!
     * @brief constructs Location object for wgs position
     * @param lon longitude, radians
     * @param lat latitude, radians
     * @param height height meters
     */
    Location(double lon, double lat, double height)
    {
        m_height = static_cast<float>(height);
        wgsop::wgs2xyz(m_posGeocentric, lat, lon, m_height);
        wgsop::wgs2ts(m_east, m_north, m_up, lon, lat);
    }

    /**
    * @param h height over terrain
    *
    * @return placement matrix: mat*PLACEMENT
    */
    inline glm::dmat4 getPlacementMatrix(double h) const
    {
        return gce::transformBasis(m_east, m_north, m_up, m_posGeocentric + m_up * h);
    }

    inline glm::dmat3 getRotation() const
    {
        return glm::dmat3{m_east, m_north, m_up};
    }
    glm::fvec4 getRay(double h) const
    {
        return glm::fvec4(m_up, m_height + h);
    }

    const glm::dvec3 &getPosition() const
    {
        return m_posGeocentric;
    }

    const glm::dvec3 &getUp() const
    {
        return m_up;
    }

    double getHeight() const
    {
        return m_height;
    }
private:
    glm::dvec3 m_east;
    glm::dvec3 m_north;
    glm::dvec3 m_up;

    glm::dvec3 m_posGeocentric;
    float m_height;
};
};
