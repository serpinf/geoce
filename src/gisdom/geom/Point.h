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

#include "Geometry.h"

namespace geom
{

/**
 * @class Point
 * @brief Represents a zero-dimensional point geometry with a single coordinate.
 *
 * A Point is the simplest geometry type, consisting of a single coordinate in 2D or 3D space.
 * It can represent locations such as cities, landmarks, or observation points.
 * Point is a final class and cannot be subclassed.
 *
 * @note A Point is immutable after construction through the use of default copy semantics.
 * @note Every Point is considered simple (non-self-intersecting) by definition.
 */
class Point final : public Geometry
{
public:
    /**
     * @brief Constructs a Point with the specified coordinate type.
     *
     * @param coordType The type of coordinates (2D or 3D) for this Point.
     */
    explicit Point(CoordinateType coordType) : Geometry(coordType)
    {}

    /**
     * @brief Copy constructor for Point.
     */
    Point(const Point &pt) = default;

    /**
     * @brief Creates a Point from a single coordinate.
     *
     * @param c The coordinate to set for this Point.
     */
    void Create(const Coordinate &c)
    {
        coordinate = c;
    }

    /**
     * @brief Creates a Point from a coordinate sequence.
     *
     * If the sequence contains multiple coordinates, only the first one is used.
     *
     * @param seq The coordinate sequence to create the Point from.
     * @return true if the Point was successfully created, false otherwise.
     */
    bool Create(const CoordinateSeq &seq) override;

    /**
     * @brief Destructor for Point.
     */
    virtual ~Point()
    {}

    /**
     * @brief Casts this geometry to a Point (non-const).
     *
     * @return A pointer to this Point if this object is a Point.
     */
    Point *isPoint() override
    {
        return this;
    }

    /**
     * @brief Casts this geometry to a Point (const).
     *
     * @return A const pointer to this Point if this object is a Point.
     */
    const Point *isPoint() const override
    {
        return this;
    }

    /**
     * @brief Converts this Point to a GEOS geometry representation.
     *
     * @return A scoped pointer to a GEOS geometry object.
     */
    GEOSGeom_scoped_t toGEOSGeom() const override;

    /**
     * @brief Applies a geometry filter to this Point.
     *
     * @param filter The geometry filter to apply.
     */
    void apply_geometry_filter(GeometryFilter &filter) const override
    {
        filter(*this);
    }

    /**
     * @brief Clears the coordinate from this Point.
     */
    void Clear() override;


    /**
     * @brief Checks if this Point is equal to another geometry.
     *
     * @param geom The geometry to compare with.
     * @return true if the geometries are equal, false otherwise.
     */
    bool equals(const Geometry *geom) const override;

    /**
     * @brief Returns a mutable reference to the coordinate.
     *
     * @return A reference to the Coordinate of this Point.
     */
    Coordinate &getCoordinate()
    {
        return coordinate;
    }

    /**
     * @brief Returns a const reference to the coordinate.
     *
     * @return A const reference to the Coordinate of this Point.
     */
    const Coordinate &getCoordinate() const
    {
        return coordinate;
    }

    /**
     * @brief Creates a deep copy of this Point.
     *
     * This performs a low-level copy operation, creating an independent copy of the coordinate.
     *
     * @return A pointer to a new Point that is an exact copy of this object.
     */
    Point *clone() const override { return new Point(*this); }

    /**
     * @brief Returns the number of vertices in this Point.
     *
     * For a Point, this is always 1.
     *
     * @return The number of points (always 1).
     */
    size_t getNumPoints() const override
    {
        return 1;
    }

    /**
     * @brief Checks if this Point is empty.
     *
     * A Point is never empty by definition.
     *
     * @return false (always).
     */
    bool isEmpty() const override
    {
        return false;
    }

    /**
     * @brief Checks if this Point is simple (non-self-intersecting).
     *
     * A Point is always simple by definition.
     *
     * @return true (always).
     */
    bool isSimple() const override
    {
        return true;
    }

    /**
     * @brief Returns the dimension of this geometry.
     *
     * For a Point, the dimension is always 0 (zero-dimensional).
     *
     * @return The dimension value (0 for Point).
     */
    int getDimension() const override
    {
        return 0;
    }

    /**
     * @brief Calculates the distance from this Point to a given 2D point.
     *
     * @param pos The 2D position to measure distance to.
     * @return The Euclidean distance from this Point to the position.
     */
    double distance(const glm::dvec2 &pos) const override;

    /**
     * @brief Returns the geometry type identifier for this Point.
     *
     * @return The geometry type (gceGeometryType::Point).
     */
    gceGeometryType getGeometryTypeId() const override
    {
        return gceGeometryType::Point;
    }

    /**
     * @brief Applies a read-write filter to this Point.
     *
     * @param filter The read-write filter to apply.
     */
    void apply_filter_rw(filter_rw &filter) override;

    /**
     * @brief Applies a read-only filter to this Point.
     *
     * @param filter The read-only filter to apply.
     */
    void apply_filter_ro(filter_ro &filter) const override;

    /**
     * @brief Removes a vertex at the specified index.
     *
     * @param idx The index of the vertex to remove.
     */
    void removeVertex(size_t idx) override;

    /**
     * @brief Updates the coordinate at the specified vertex index.
     *
     * @param idx The index of the vertex to update.
     * @param coo The new coordinate value.
     */
    void updateVertex(size_t idx, const Coordinate &coo) override;

    /**
     * @brief Inserts a new vertex at the specified index.
     *
     * @param idx The index where the new vertex will be inserted.
     * @param coo The coordinate of the new vertex.
     */
    void insertVertex(size_t idx, const Coordinate &coo) override;

    /**
     * @brief Retrieves the geometry component for a specific vertex.
     *
     * @param idx The index of the vertex.
     * @return A pair containing the vertex index and a pointer to the associated geometry.
     */
    std::pair<size_t, const Geometry *> getGeometryForVertex(size_t idx) const override;

private:
    /** @brief The coordinate that defines this Point. */
    Coordinate coordinate{};
};

}; // namespace geom
