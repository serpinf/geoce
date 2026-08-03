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

#include "wgsop.h"
#include <GeographicLib/Geocentric.hpp>

namespace wgsop
{

///////////////////////////////////////////////////////////////////////////
void wgs2xyz(glm::dvec3 &xyz, double lat, double lon, double height)
{
    double Sin_Lat = sin(lat);
    double Cos_Lat = cos(lat);
    double Sin2_Lat = Sin_Lat * Sin_Lat;
    double Rn = Ra / (sqrt(1.0 - sqr_e * Sin2_Lat));
    xyz[0] = (Rn + height) * Cos_Lat * cos(lon);
    xyz[1] = (Rn + height) * Cos_Lat * sin(lon);
    xyz[2] = ((Rn * (1.0 - sqr_e)) + height) * Sin_Lat;
}

void wgs2up(glm::dvec3 &up, double lon, double lat)
{
    const double cos_LAT = cos(lat);
    const double sin_LAT = sin(lat);

    const double cos_LON = cos(lon);
    const double sin_LON = sin(lon);

    up.x = cos_LON * cos_LAT;
    up.y = sin_LON * cos_LAT;
    up.z = sin_LAT;
}

void wgs2xyz_up(glm::dvec3 &xyz, glm::dvec3 &up, const glm::dvec2 &pos, double h)
{
    const double cos_LAT = cos(pos.y);
    const double sin_LAT = sin(pos.y);

    const double cos_LON = cos(pos.x);
    const double sin_LON = sin(pos.x);

    double Sin2_Lat = sin_LAT * sin_LAT;
    double Rn = Ra / (sqrt(1.0 - sqr_e * Sin2_Lat));
    xyz[0] = (Rn + h) * cos_LAT * cos_LON;
    xyz[1] = (Rn + h) * cos_LAT * sin_LON;
    xyz[2] = ((Rn * (1.0 - sqr_e)) + h) * sin_LAT;

    up.x = cos_LON * cos_LAT;
    up.y = sin_LON * cos_LAT;
    up.z = sin_LAT;
}

void wgs2ts(glm::dvec3 &east, glm::dvec3 &north, glm::dvec3 &up, double lon, double lat)
{
    const double cos_LAT = cos(lat);
    const double sin_LAT = sin(lat);

    const double cos_LON = cos(lon);
    const double sin_LON = sin(lon);

    up.x = cos_LON * cos_LAT;
    up.y = sin_LON * cos_LAT;
    up.z = sin_LAT;

    east.x = -sin_LON;
    east.y = +cos_LON;
    east.z = 0.0;

    north = glm::cross(up, east);
}
///////////////////////////////////////////////////////////////////////////
glm::dvec3 xyz2wgs(const glm::dvec3 &pos)
{
    using namespace GeographicLib;

    const Geocentric &earth = Geocentric::WGS84();
    double lat = 0.0, lon = 0.0, h = 0.0;
    earth.Reverse(pos.x, pos.y, pos.z, lat, lon, h);

    return glm::dvec3{lon, lat, h};
}

}
