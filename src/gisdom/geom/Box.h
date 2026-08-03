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

#include "alg/gc_algebra.h"

namespace geom
{
class filter_ro;

template <size_t DIMS> class Box
{
public:
    using vec_type = glm::vec<DIMS, double>;

    Box() = default;

    /*!
     * @brief construct box with given corners
     * @param a bottom-left
     * @param b top-right
     */
    Box(const vec_type &a, const vec_type &b) : cmin(a), cmax(b)
    {}

    /*!
    * Initialize box from center and half-size
    *
    * @param cen       - box center
    * @param half_size - half box size
    */
    void init_cen_size(const vec_type &cen, const vec_type &half_size)
    {
        cmin = cen - half_size;
        cmax = cen + half_size;
    }

    /**
    * Get 2D box part (1-4 quadrants)
    *
    * @param part quadrant number (1=top-left, 2=top-right, 3=bottom-left, 4=bottom-right)
    * @return part box
    */
    Box<2> getPart2D(int part) const
    {
        glm::dvec2 cen = glm::dvec2(GetCenter());
        switch (part)
        {
        case 1:
            return Box<2>({cmin.x, cen.x}, {cen.x, cmax.y});
        case 2:
            return Box<2>(cen, glm::dvec2(cmax));
        case 3:
            return Box<2>(glm::dvec2(cmin), cen);
        case 4:
            return Box<2>({cen.x, cmin.y}, {cmax.x, cen.y});
        }
        return {};
    }

    /*!
    * Expand to fit point
    *
    * @param p coordinates of expanding point
    */
    void expand(const vec_type &p)
    {
        cmin = glm::min(cmin, p);
        cmax = glm::max(cmax, p);
    }

    /*!
    * Expand to contain box
    *
    * @param other box to contain
    */
    void expand(const Box &other)
    {
        cmin = glm::min(cmin, other.cmin);
        cmax = glm::max(cmax, other.cmax);
    }

    /*!
    * Scale relative to box center
    *
    * @param s scale factor
    */
    void scale(const vec_type &s)
    {
        vec_type cen = GetCenter();
        vec_type half_size = (cmax - cen) * s;

        cmin = cen - half_size;
        cmax = cen + half_size;
    }

    /*!
    * Scale relative to given center
    *
    * @param s   scale factor
    * @param cen scale center
    */
    void scaleTo(const vec_type &s, const vec_type &cen)
    {
        cmin = (cmin - cen) * s + cen;
        cmax = (cmax - cen) * s + cen;
    }

    void offset(const vec_type &off)
    {
        cmin += off;
        cmax += off;
    }

    /*!
    * Clamp point to box bounds
    *
    * @param p point to clamp
    * @return clamped point
    */
    vec_type clamp(const vec_type &p) const
    {
        return glm::clamp(p, cmin, cmax);
    }

    /*!
    * Check if point is contained in box
    *
    * @param p point to check
    * @return true if point is inside box
    */
    bool contains(const vec_type &p) const
    {
        return glm::all(glm::greaterThanEqual(p, cmin)) && glm::all(glm::lessThanEqual(p, cmax));
    }

    /*!
    * Check if box is contained in this box
    *
    * @param other box to check
    * @return true if other is completely inside this box
    */
    bool contains(const Box &other) const
    {
        return glm::all(glm::greaterThanEqual(other.cmin, cmin)) &&
            glm::all(glm::lessThanEqual(other.cmax, cmax));
    }

    /*!
    * Check if point overlaps with box
    *
    * @param p point to check
    * @return true if point is inside box
    */
    bool overlaps(const vec_type &p) const
    {
        return contains(p);
    }

    /*!
    * Check if box overlaps with this box
    *
    * @param other box to check
    * @return true if boxes overlap or touch
    */
    bool overlaps(const Box &other) const
    {
        return !(glm::any(glm::lessThan(other.cmax, cmin)) ||
                 glm::any(glm::greaterThan(other.cmin, cmax)));
    }

    /*!
    * Check if two boxes are equal
    *
    * @param other box to compare
    * @return true if boxes have identical bounds
    */
    bool equals(const Box &other) const
    {
        return cmin == other.cmin && cmax == other.cmax;
    }

    /*!
    * Check if box is empty (invalid)
    *
    * @return true if cmin > cmax in any dimension
    */
    bool empty() const
    {
        return glm::any(glm::greaterThan(cmin, cmax));
    }

    //! Get position of bottom-left corner
    glm::dvec2 getBottomLeft() const
    {
        return glm::dvec2(cmin);
    }

    //! Get position of bottom-right corner
    glm::dvec2 getBottomRight() const
    {
        return glm::dvec2(cmax.x, cmin.y);
    }

    //! Get position of top-left corner
    glm::dvec2 getTopLeft() const
    {
        return glm::dvec2(cmin.x, cmax.y);
    }

    //! Get position of top-right corner
    glm::dvec2 getTopRight() const
    {
        return glm::dvec2(cmax);
    }

    //! Get box center
    vec_type GetCenter() const
    {
        return 0.5 * (cmin + cmax);
    }

    /*!
    * Calculate distance between boxes
    *
    * @param other box to measure distance to
    * @return distance (0 if boxes overlap)
    */
    double distance(const Box &other) const
    {
        if (overlaps(other)) return 0.0;

        vec_type dd = glm::max(glm::max(cmin - other.cmax, other.cmin - cmax), 0.0);
        return glm::length(dd);
    }

    /*!
    * Calculate intersection of two boxes
    *
    * @param other box to intersect with
    * @return resulting intersection box (may be empty)
    */
    Box intersection(const Box &other) const
    {
        Box ret;
        ret.cmin = glm::max(cmin, other.cmin);
        ret.cmax = glm::min(cmax, other.cmax);
        return ret;
    }

    //! Get box width (X dimension)
    double width() const
    {
        return cmax.x - cmin.x;
    }

    //! Get box height (Y dimension)
    double height() const
    {
        return cmax.y - cmin.y;
    }

    //! Get box depth (Z dimension) - only valid for 3D boxes
    double depth() const
    {
        if constexpr (DIMS == 3) return cmax.z - cmin.z;
        else return 0.0;
    }

    //! Get box size vector
    vec_type size() const
    {
        return cmax - cmin;
    }

    //! Get box volume (valid for 2D and 3D)
    double volume() const
    {
        vec_type sz = size();
        if constexpr (DIMS == 2) return sz.x * sz.y;
        else if constexpr (DIMS == 3) return sz.x * sz.y * sz.z;
        else return 0.0;
    }

    vec_type cmin{+DoubleInfinity};
    vec_type cmax{-DoubleInfinity};
};

using Box2D = Box<2>;
using Box3D = Box<3>;

}; // namespace geom
