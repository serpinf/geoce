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

#include "MGeometry.h"

namespace geom
{

/**
 * @class MLineString
 * @brief Represents a multi-line string geometry consisting of multiple line string components.
 *
 * A MLineString (Multi-LineString) is a collection of one or more LineString geometries.
 * Each component is a separate one-dimensional geometry object that can be accessed and
 * manipulated independently. MLineString is commonly used to represent features with
 * multiple disconnected line segments, such as roads with gaps or networks with branches.
 *
 * @note Each component LineString should have at least 2 points to be valid.
 */
class MLineString : public MGeometry
{
public:

    /** @brief Scoped pointer type for automatic memory management of MLineString objects. */
    typedef std::unique_ptr<MLineString> scoped_ptr;

    /**
     * @brief Copy constructor for MLineString.
     *
     * Creates a new MLineString that is an exact copy of the provided geometry.
     *
     * @param geom The MLineString to copy.
     */
    explicit MLineString(const MLineString &geom) : MGeometry(geom)
    {}

    /**
     * @brief Constructs an MLineString with the specified coordinate type.
     *
     * @param coordType The type of coordinates (2D or 3D) for all line strings in this collection.
     */
    explicit MLineString(CoordinateType coordType) : MGeometry(coordType)
    {}

    /**
     * @brief Destructor for MLineString.
     */
    virtual ~MLineString();

    /**
     * @brief Casts this geometry to an MLineString (non-const).
     *
     * @return A pointer to this MLineString if this object is an MLineString.
     */
    MLineString *isMLineString() override
    {
        return this;
    }

    /**
     * @brief Casts this geometry to an MLineString (const).
     *
     * @return A const pointer to this MLineString if this object is an MLineString.
     */
    const MLineString *isMLineString() const override
    {
        return this;
    }

    /**
     * @brief Creates an MLineString from a coordinate sequence.
     *
     * @param seq The coordinate sequence to create the MLineString from.
     * @return true if the MLineString was successfully created, false otherwise.
     */
    bool Create(const CoordinateSeq &seq) override;

    /**
     * @brief Creates a deep copy of this MLineString.
     *
     * This performs a low-level copy operation, creating independent copies of all
     * component geometries.
     *
     * @return A pointer to a new MLineString that is an exact copy of this object.
     */
    MLineString *clone() const override { return new MLineString(*this); }

    /**
     * @brief Returns the dimension of this geometry.
     *
     * For an MLineString, the dimension is always 1 (one-dimensional).
     *
     * @return The dimension value (1 for MLineString).
     */
    int getDimension() const override
    {
        return 1;
    }

    /**
     * @brief Returns the geometry type identifier for this MLineString.
     *
     * @return The geometry type (gceGeometryType::MultiLineString).
     */
    gceGeometryType getGeometryTypeId() const override
    {
        return gceGeometryType::MultiLineString;
    }

    /**
     * @brief Checks if all line strings in this collection are closed.
     *
     * A line string is considered closed if its first and last coordinates are the same.
     * This method verifies that all component geometries satisfy the closed condition.
     *
     * @return true if all line strings in the collection are closed, false otherwise.
     */
    bool isClosed() const;
};
}; // namespace geom
