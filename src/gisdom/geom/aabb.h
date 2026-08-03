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

#include "Box.h"

namespace geom
{

/**
 * @brief Axis-Aligned Bounding Box (AABB) template class.
 *
 * Represents an axis-aligned bounding box in N-dimensional space using a center point
 * and half-size representation. This class provides efficient geometric queries such as
 * point intersection, box overlap, and distance calculations.
 *
 * @tparam DIMS The dimensionality of the bounding box (typically 2 for 2D or 3 for 3D).
 */
template <size_t DIMS> class aabb
{
public:
    using vec_type = glm::vec<DIMS, double>; ///< Vector type for center and size in DIMS dimensions

    /**
     * @brief Default constructor.
     *
     * Creates an AABB centered at the origin with zero size.
     */
    constexpr aabb() : cen(0.0), size(0.0) {}

    /**
     * @brief Constructor with center and size.
     *
     * Creates an AABB with the specified center and half-size.
     *
     * @param cen The center point of the bounding box
     * @param size The half-size (extent) of the bounding box in each dimension
     */
    constexpr aabb(const vec_type &cen, const vec_type &size) : cen(cen), size(size) {}

    /**
     * @brief Constructor from a Box.
     *
     * Creates an AABB from a Box object by extracting its center and computing the half-size.
     *
     * @param source The Box to convert to AABB format
     */
    explicit aabb(const geom::Box<DIMS> &source) : cen(source.GetCenter()), size(0.5 * source.size()) {}

    /**
     * @brief Tests if a point is contained within this AABB.
     *
     * Checks whether the given point is inside or on the boundary of this axis-aligned
     * bounding box by comparing the distance from the center to the point with the half-size
     * in each dimension.
     *
     * @param c The point to test
     * @return true if the point is within the AABB, false otherwise
     */
    bool intersects(const vec_type &c) const
    {
        return glm::all(glm::lessThanEqual(glm::abs(cen - c), size));
    }

    /**
     * @brief Tests if this AABB overlaps with another AABB.
     *
     * Determines whether two axis-aligned bounding boxes have any overlapping region
     * by comparing the distance between centers with the sum of their half-sizes.
     *
     * @param other The other AABB to test for overlap
     * @return true if the AABBs overlap, false otherwise
     */
    bool overlaps(const aabb &other) const
    {
        return glm::all(glm::lessThan(glm::abs(cen - other.cen), size + other.size));
    }

    /**
     * @brief Calculates the squared distance from a point to this AABB.
     *
     * Computes the squared Euclidean distance from the given position to the closest
     * point on the AABB. Returns zero if the point is inside the box.
     *
     * @param pos The position to measure distance from
     * @return The squared distance from the point to the AABB
     */
    double distance2(const vec_type &pos) const
    {
        auto d = glm::max(glm::abs(pos - cen) - size, 0.0);
        return glm::dot(d, d);
    }

    /**
     * @brief Converts this AABB to a Box representation.
     *
     * Creates a Box object from this AABB by computing the minimum and maximum corners
     * from the center and half-size representation.
     *
     * @return A Box with equivalent geometry
     */
    Box<DIMS> toBox() const
    {
        return Box<DIMS>{cen - size, cen + size};
    }

    /**
     * @brief Gets the metrics (characteristic size) of this AABB.
     *
     * Returns a single scalar value representing the characteristic size of the bounding box.
     * For 2D, this is the maximum of the two half-sizes times 2. For 3D, it is the maximum
     * of the three half-sizes times 2 (i.e., the maximum full dimension).
     *
     * @return The metrics value representing the size of the AABB
     */
    double getMetrics() const
    {
        if constexpr (DIMS == 2)
            return std::max(size.x, size.y) * 2.0;
        else
            return std::max(size.x, std::max(size.y, size.z)) * 2.0;
    }

    /**
     * @brief Gets a 2D quadrant partition of this AABB.
     *
     * Divides this AABB into four equal quadrants and returns the one specified by the part index.
     * This method is useful for spatial partitioning and quad-tree construction.
     * The quadrant layout is:
     * - 1: Top-left (negative x, positive y)
     * - 2: Top-right (positive x, positive y)
     * - 3: Bottom-left (negative x, negative y)
     * - 4: Bottom-right (positive x, negative y)
     *
     * @param part The quadrant number (1-4). Returns an empty AABB if out of range.
     * @return A 2D AABB representing the specified quadrant
     */
    aabb<2> getPart2D(int part) const
    {
        constexpr glm::dvec2 quadrant_offsets[] = {{-1.0, 1.0}, {1.0, 1.0}, {-1.0, -1.0}, {1.0, -1.0}};
        if (part < 1 || part > 4) return {};
        const glm::dvec2 half{0.5 * size.x, 0.5 * size.y};
        return aabb<2>{glm::dvec2(cen) + quadrant_offsets[part - 1] * half, half};
    }

    vec_type cen{}; ///< Center point of the AABB
    vec_type size{}; ///< Half-size (extent) of the AABB in each dimension
};

/// @typedef aabb2
/// @brief 2D axis-aligned bounding box
using aabb2 = aabb<2>;

/// @typedef aabb3
/// @brief 3D axis-aligned bounding box
using aabb3 = aabb<3>;

/// @typedef psAABB
/// @brief Alias for 3D axis-aligned bounding box used in spatial partitioning
using psAABB = aabb3;

}
