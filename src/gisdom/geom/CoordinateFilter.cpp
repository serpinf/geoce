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

#include "CoordinateFilter.h"
#include "Box.h"
#include "alg/wgsop.h"

namespace geom
{
///////////////////////////////////////////////////////////////////////////
// Base RO filter
//
void filter_ro::beginGeometry(const Geometry &)
{}
void filter_ro::endGeometry()
{}

//!standard affine filter
affine_filter::affine_filter(const glm::dmat4 &transform) : transform(transform) {}

Coordinate affine_filter::operator()(const Coordinate &coo)
{
    return Coordinate(glm::dmat4x3(transform) * glm::dvec4(coo.pos, 1.0), coo.m);
}

///////////////////////////////////////////////////////////////////////////
// Move filter
//
Coordinate move_filter::operator()(const Coordinate &coo)
{
    return Coordinate(coo.pos + offset, coo.m);
}
///////////////////////////////////////////////////////////////////////////
// Rotate filter
//
rotate_filter::rotate_filter(const glm::dvec2 &baseCoord, const double angle) :
    _base(baseCoord), _sin(sin(angle)), _cos(cos(angle))
{}

Coordinate rotate_filter::operator()(const Coordinate &coo)
{
    double dx = coo.pos.x - _base.x;
    double dy = coo.pos.y - _base.y;
    double dx1 = dx * _cos - dy * _sin;
    double dy1 = dx * _sin + dy * _cos;
    return Coordinate(glm::dvec3(_base.x + dx1, _base.y + dy1, coo.pos.z), coo.m);
}
///////////////////////////////////////////////////////////////////////////
// Box filter
//
void box_filter::operator()(const Coordinate &coo)
{
    bbox.expand(coo.pos);
}
///////////////////////////////////////////////////////////////////////////
// flat2wgs filter
//
geom::Coordinate filter_flat2wgs::operator()(const geom::Coordinate &coo)
{
    return geom::Coordinate({coo.pos.x, wgsop::flat2wgs_Y(coo.pos.y), coo.pos.z}, coo.m);
}

///////////////////////////////////////////////////////////////////////////
// wgs2flat filter
//
geom::Coordinate filter_wgs2flat::operator()(const geom::Coordinate &coo)
{
    return geom::Coordinate({coo.pos.x, wgsop::wgs2flat_Y(coo.pos.y), coo.pos.z}, coo.m);
}
};
