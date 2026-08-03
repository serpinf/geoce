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

#include "CoordSeq.h"
#include "Geometry.h"

namespace geom
{

/**
 * @class LineString
 * @brief Represents a line string geometry consisting of a sequence of connected coordinate points.
 *
 * A LineString is a one-dimensional geometry object defined by a sequence of coordinates.
 * It can represent various geometric entities such as roads, rivers, or boundaries.
 * LineString is a final class and cannot be subclassed.
 *
 * @note A valid LineString must have at least 2 points.
 * @note A LineString is immutable after construction through the use of default copy semantics.
 */
class LineString final : public Geometry
{
public:

    /**
     * @brief Constructs a LineString with the specified coordinate type.
     *
     * @param coordType The type of coordinates (2D or 3D) for this LineString.
     */
    explicit LineString(CoordinateType coordType) : Geometry(coordType), coordinates(coordType)
    {}

    /**
     * @brief Copy constructor for LineString.
     */
    LineString(const LineString &ls) = default;

    /**
     * @brief Creates a LineString from a coordinate sequence.
     *
     * @param seq The coordinate sequence to create the LineString from.
     * @return true if the LineString was successfully created, false otherwise.
     */
    bool Create(const CoordinateSeq &seq) override;

    /**
     * @brief Destructor for LineString.
     */
    ~LineString() override = default;

    /**
     * @brief Casts this geometry to a LineString (non-const).
     *
     * @return A pointer to this LineString if this object is a LineString, nullptr otherwise.
     */
    LineString *isLineString() final
    {
        return this;
    }

    /**
     * @brief Casts this geometry to a LineString (const).
     *
     * @return A const pointer to this LineString if this object is a LineString, nullptr otherwise.
     */
    const LineString *isLineString() const final
    {
        return this;
    }

    /**
     * @brief Converts this LineString to a GEOS geometry representation.
     *
     * @return A scoped pointer to a GEOS geometry object.
     */
    GEOSGeom_scoped_t toGEOSGeom() const override;

    /**
     * @brief Applies a geometry filter to this LineString.
     *
     * @param filter The geometry filter to apply.
     */
    void apply_geometry_filter(GeometryFilter &filter) const override
    {
        filter(*this);
    }

    /**
     * @brief Clears all coordinates from this LineString.
     */
    void Clear() override
    {
        coordinates.clear();
    }


    /**
     * @brief Checks if this LineString is equal to another geometry.
     *
     * @param geom The geometry to compare with.
     * @return true if the geometries are equal, false otherwise.
     */
    bool equals(const Geometry *geom) const override;

    /**
     * @brief Creates a deep copy of this LineString.
     *
     * @return A pointer to a new LineString that is an exact copy of this object.
     */
    LineString *clone() const override { return new LineString(*this); }

    /**
     * @brief Returns the dimension of this geometry.
     *
     * For a LineString, the dimension is always 1 (one-dimensional).
     *
     * @return The dimension value (1 for LineString).
     */
    int getDimension() const override
    {
        return 1;
    }

    /**
     * @brief Calculates the distance from this LineString to a given 2D point.
     *
     * @param pos The 2D position to measure distance to.
     * @return The shortest distance from the LineString to the position.
     */
    double distance(const glm::dvec2 &pos) const override
    {
        return coordinates.distance(pos);
    }

    /**
     * @brief Returns the number of vertices in this LineString.
     *
     * @return The number of coordinate points.
     */
    size_t getNumPoints() const override
    {
        return coordinates.size();
    }

    /**
     * @brief Checks if this LineString is simple (non-self-intersecting).
     *
     * @return true if the LineString does not intersect itself, false otherwise.
     */
    bool isSimple() const override
    {
        return true;
    }

    /**
     * @brief Checks if this LineString is empty.
     *
     * @return true if the LineString contains no coordinates, false otherwise.
     */
    bool isEmpty() const override
    {
        return coordinates.empty();
    }

    /**
     * @brief Checks if this LineString is valid.
     *
     * A LineString is considered valid if it contains at least 2 points.
     *
     * @return true if the LineString is valid, false otherwise.
     */
    bool isValid() const override
    {
        return coordinates.size() != 1;
    }

    /**
     * @brief Checks if this LineString is closed.
     *
     * A LineString is considered closed if the first and last coordinates are the same.
     *
     * @return true if the LineString is closed, false otherwise.
     */
    bool isClosed() const
    {
        return coordinates.isClosed();
    }

    /**
     * @brief Checks if this LineString is a valid ring (closed and non-self-intersecting).
     *
     * A ring is a closed LineString that does not intersect itself.
     *
     * @return true if the LineString is a valid ring, false otherwise.
     */
    bool isRing() const
    {
        return coordinates.isRing();
    }

    /**
     * @brief Returns the geometry type identifier for this LineString.
     *
     * @return The geometry type (gceGeometryType::LineString).
     */
    gceGeometryType getGeometryTypeId() const override
    {
        return gceGeometryType::LineString;
    }

    /**
     * @brief Calculates the 2D length of this LineString.
     *
     * The length is computed from the x and y coordinates only.
     *
     * @return The 2D length of the LineString.
     */
    double length() const override
    {
        return coordinates.length2D();
    }

    /**
     * @brief Calculates the 3D length of this LineString.
     *
     * The length is computed using all three dimensions (x, y, and z).
     *
     * @return The 3D length of the LineString.
     */
    double length3d() const override
    {
        return coordinates.length3D();
    }

    /**
     * @brief Returns a const reference to the coordinate sequence.
     *
     * @return A const reference to the CoordinateSeq containing all vertices.
     */
    const CoordinateSeq &getCoordSeq() const
    {
        return coordinates;
    }

    /**
     * @brief Returns a mutable reference to the coordinate sequence.
     *
     * @return A reference to the CoordinateSeq containing all vertices.
     */
    CoordinateSeq &getCoordSeq()
    {
        return coordinates;
    }

    /**
     * @brief Retrieves the geometry component for a specific vertex.
     *
     * @param idx The index of the vertex.
     * @return A pair containing the vertex index and a pointer to the associated geometry.
     */
    std::pair<size_t, const Geometry *> getGeometryForVertex(size_t idx) const override;

    /**
     * @brief Applies a read-write filter to this LineString.
     *
     * @param filter The read-write filter to apply.
     */
    void apply_filter_rw(filter_rw &filter) override;

    /**
     * @brief Applies a read-only filter to this LineString.
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

private:
    /** @brief The sequence of coordinates that define this LineString. */
    CoordinateSeq coordinates;
};

}; // namespace geom
