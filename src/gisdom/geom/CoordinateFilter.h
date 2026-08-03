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
#include <geom/Coordinate.h>
#include <geom/Box.h>

namespace geom
{
class Geometry;

class filter_ro
{
public:
    virtual ~filter_ro() {}
    virtual void operator()(const Coordinate &coo) = 0;
    virtual void beginGeometry(const Geometry &geom);
    virtual void endGeometry();
};

class filter_rw
{
public:
    virtual ~filter_rw() {}
    virtual Coordinate operator()(const Coordinate &coo) = 0;
};

//!standard affine filter
class affine_filter : public filter_rw
{
public:
    explicit affine_filter(const glm::dmat4 &transform);
    virtual Coordinate operator()(const Coordinate &coo);
private:
    const glm::dmat4 transform;
};
//!standard move filter
class move_filter : public filter_rw
{
public:
    explicit move_filter(const glm::dvec3 &offset) : offset(offset) {}
    virtual Coordinate operator()(const Coordinate &coo);
private:
    const glm::dvec3 offset;
};
//!standard rotate filter
class rotate_filter : public filter_rw
{
public:
    rotate_filter(const glm::dvec2 &baseCoord, const double angle);
    virtual Coordinate operator()(const Coordinate &coo);
private:
    const glm::dvec2 _base;
    const double _sin;
    const double _cos;
};

//!standard box filter
class box_filter : public filter_ro
{
public:
    virtual void operator()(const Coordinate &coo);
    Box3D bbox{};
};

//! project from flat to wgs
class filter_flat2wgs : public geom::filter_rw
{
public:
    virtual geom::Coordinate operator()(const geom::Coordinate &coo);
};
//! project from wgs to flat
class filter_wgs2flat : public geom::filter_rw
{
public:
    virtual geom::Coordinate operator()(const geom::Coordinate &coo);
};
}; //namespace geom
