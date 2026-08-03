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

#include <geos_c.h>
#include "Coordinate.h"
#include "Box.h"

/**
 * Enumeration of all supported geometry types.
 * Maps to OGC (Open Geospatial Consortium) geometry type codes.
 */
enum class gceGeometryType
{
    Point = 1,              //!< Single point geometry
    LineString = 2,         //!< Line string (connected line segments)
    Polygon = 3,            //!< Polygon (area bounded by rings)
    MultiPoint = 4,         //!< Collection of multiple points
    MultiLineString = 5,    //!< Collection of multiple line strings
    MultiPolygon = 6,       //!< Collection of multiple polygons
    GeometryCollection = 7, //!< Heterogeneous collection of geometries
};

namespace geom
{
//! Read-only coordinate filter visitor
class filter_ro;
//! Read-write coordinate filter visitor
class filter_rw;
//! Coordinate sequence container
class CoordinateSeq;

//! Point geometry type
class Point;
//! LineString geometry type
class LineString;
//! Polygon geometry type
class Polygon;
//! Multi-geometry base class
class MGeometry;
//! MultiPoint geometry type
class MPoint;
//! MultiLineString geometry type
class MLineString;
//! MultiPolygon geometry type
class MPolygon;

/**
 * Abstract geometry filter for visiting basic geometry types.
 * Implements the visitor pattern for geometry objects.
 * Only handles Point, LineString, and Polygon types.
 *
 * To process all geometry types including multi-geometries and collections,
 * use the apply_geometry_filter() method on the Geometry class.
 */
struct GeometryFilter
{
    /**
     * Virtual destructor.
     */
    virtual ~GeometryFilter() = default;
    /**
    * Visit a Point geometry.
     *
     * @param point the Point to process
     */
    virtual void operator()(const Point &point) = 0;
    /**
     * Visit a LineString geometry.
     *
     * @param lineString the LineString to process
     */
    virtual void operator()(const LineString &lineString) = 0;
    /**
     * Visit a Polygon geometry.
     *
     * @param polygon the Polygon to process
     */
    virtual void operator()(const Polygon &polygon) = 0;
};
namespace detail
{
/**
 * Custom deleter for GEOS geometry objects.
 * Properly destroys GEOS geometry resources when used with std::unique_ptr.
 */
struct GEOSGeom_destroyer
{
    /**
     * Destroy a GEOS geometry.
     *
     * @param g pointer to the GEOS geometry to destroy
     */
    void operator()(GEOSGeometry *g) { GEOSGeom_destroy(g); }
};
}
/**
 * Smart pointer type for GEOS geometry objects.
 * Automatically manages memory and ensures proper cleanup of GEOS resources.
 */
using GEOSGeom_scoped_t = std::unique_ptr<GEOSGeometry, detail::GEOSGeom_destroyer>;

/**
 * Base class for all geometry types.
 *
 * This is an abstract base class that defines the interface for all geometry objects.
 * It provides methods for type checking, transformation, querying properties, and validation.
 *
 * Geometry objects support coordinates with optional Z (elevation) and M (measure) values.
 * They can be created from coordinate sequences or GEOS geometry objects.
 *
 * Key operations:
 * - Type checking via is*() methods and getGeometryTypeId()
 * - Geometric operations: affine transformations, translation, rotation
 * - Property queries: area, length, dimension, bounding box
 * - Filtering: coordinate filters, geometry filters with visitor pattern
 * - Vertex manipulation: insert, update, remove vertices
 *
 * @see gceGeometryType
 * @see Point
 * @see LineString
 * @see Polygon
 * @see MGeometry
 */
class Geometry
{
public:
    /**
     * Factory method to create a geometry from a dimension, coordinate type, and coordinate sequence.
     *
     * @param dimension the dimension of the geometry
     * @param cooType the coordinate type
     * @param seq the coordinate sequence
     * @return a unique pointer to the created geometry, or nullptr if creation fails
     */
    static std::unique_ptr<Geometry> Create(int dimension, CoordinateType cooType, const CoordinateSeq &seq);

    /**
     * Factory method to create a geometry from GEOS geometry.
     *
     * @param cooType the coordinate type
     * @param geosGeom the GEOS geometry to convert
     * @return a unique pointer to the created geometry, or nullptr if creation fails
     */
    static std::unique_ptr<Geometry> Create(CoordinateType cooType, const GEOSGeom_scoped_t &geosGeom);

    /**
     * Copy constructor.
     *
     * @param other the geometry to copy
     */
    explicit Geometry(const Geometry &other) : m_coordType(other.m_coordType)
    {}

    /**
     * Constructor that initializes the geometry with a coordinate type.
     *
     * @param coordType the coordinate type for this geometry
     */
    explicit Geometry(CoordinateType coordType) : m_coordType(coordType)
    {}

    /**
     * Virtual destructor.
     */
    virtual ~Geometry() = default;

    /*!
     * create geometry from coordinate sequence
     *
     * @param seq base coordinate sequence
     *
     * @return operation status
     */
    virtual bool Create(const CoordinateSeq &seq);

    /**
     * Cast to Point geometry if this geometry is a Point.
     *
     * @return pointer to Point if this is a Point geometry, nullptr otherwise
     */
    virtual Point *isPoint()
    {
        return nullptr;
    }
    /**
     * Cast to const Point geometry if this geometry is a Point.
     *
     * @return const pointer to Point if this is a Point geometry, nullptr otherwise
     */
    virtual const Point *isPoint() const
    {
        return nullptr;
    }

    /**
     * Cast to LineString geometry if this geometry is a LineString.
     *
     * @return pointer to LineString if this is a LineString geometry, nullptr otherwise
     */
    virtual LineString *isLineString()
    {
        return nullptr;
    }
    /**
     * Cast to const LineString geometry if this geometry is a LineString.
     *
     * @return const pointer to LineString if this is a LineString geometry, nullptr otherwise
     */
    virtual const LineString *isLineString() const
    {
        return nullptr;
    }

    /**
     * Cast to Polygon geometry if this geometry is a Polygon.
     *
     * @return pointer to Polygon if this is a Polygon geometry, nullptr otherwise
     */
    virtual Polygon *isPolygon()
    {
        return nullptr;
    }
    /**
     * Cast to const Polygon geometry if this geometry is a Polygon.
     *
     * @return const pointer to Polygon if this is a Polygon geometry, nullptr otherwise
     */
    virtual const Polygon *isPolygon() const
    {
        return nullptr;
    }

    /**
     * Cast to multi-geometry if this geometry is a multi-geometry.
     *
     * @return pointer to MGeometry if this is a multi-geometry, nullptr otherwise
     */
    virtual MGeometry *isMGeometry()
    {
        return nullptr;
    }
    /**
     * Cast to const multi-geometry if this geometry is a multi-geometry.
     *
     * @return const pointer to MGeometry if this is a multi-geometry, nullptr otherwise
     */
    virtual const MGeometry *isMGeometry() const
    {
        return nullptr;
    }

    /**
     * Cast to MultiPoint geometry if this geometry is a MultiPoint.
     *
     * @return pointer to MPoint if this is a MultiPoint geometry, nullptr otherwise
     */
    virtual MPoint *isMPoint()
    {
        return nullptr;
    }
    /**
     * Cast to const MultiPoint geometry if this geometry is a MultiPoint.
     *
     * @return const pointer to MPoint if this is a MultiPoint geometry, nullptr otherwise
     */
    virtual const MPoint *isMPoint() const
    {
        return nullptr;
    }

    /**
     * Cast to MultiLineString geometry if this geometry is a MultiLineString.
     *
     * @return pointer to MLineString if this is a MultiLineString geometry, nullptr otherwise
     */
    virtual MLineString *isMLineString()
    {
        return nullptr;
    }
    /**
     * Cast to const MultiLineString geometry if this geometry is a MultiLineString.
     *
     * @return const pointer to MLineString if this is a MultiLineString geometry, nullptr otherwise
     */
    virtual const MLineString *isMLineString() const
    {
        return nullptr;
    }

    /**
     * Cast to MultiPolygon geometry if this geometry is a MultiPolygon.
     *
     * @return pointer to MPolygon if this is a MultiPolygon geometry, nullptr otherwise
     */
    virtual MPolygon *isMPolygon()
    {
        return nullptr;
    }
    /**
     * Cast to const MultiPolygon geometry if this geometry is a MultiPolygon.
     *
     * @return const pointer to MPolygon if this is a MultiPolygon geometry, nullptr otherwise
     */
    virtual const MPolygon *isMPolygon() const
    {
        return nullptr;
    }

    /**
     * Check whether this geometry is valid.
     *
     * @return true if the geometry is valid, false otherwise
     */
    virtual bool isValid() const;

    /**
     * Check whether this geometry can be made valid.
     *
     * @return true if the geometry can be made valid, false otherwise
     */
    virtual bool CanMakeValid() const;

    /**
     * Attempt to make this geometry valid.
     * Use isValid() method to test for success.
     */
    virtual void MakeValid();

    /**
     * Check whether this geometry has Z coordinates.
     *
     * @return true if geometry has Z coordinates, false otherwise
     */
    bool hasZ() const
    {
        return geom::hasZ(m_coordType);
    }
    /**
     * Check whether this geometry has M (measure) coordinates.
     *
     * @return true if geometry has M coordinates, false otherwise
     */
    bool hasM() const
    {
        return geom::hasM(m_coordType);
    }

    /**
     * Apply an affine transformation to this geometry (modifies the geometry in place).
     *
     * @param m the transformation matrix
     * @return a reference to this geometry
     */
    const Geometry &affine(const glm::dmat4 &m);

    /**
     * Get the coordinates of a uniquely determined base point of the geometry.
     *
     * @return the base coordinate of this geometry
     */
    virtual glm::dvec3 GetBaseCoord() const;

    /**
     * Move (translate) this geometry by the given offset vector.
     *
     * @param offset the translation vector
     */
    void Move(const glm::dvec3 &offset);

    /**
     * Rotate this geometry counterclockwise in the XY plane around the base point by the given angle.
     *
     * @param BaseCoord the center of rotation
     * @param Angle the rotation angle in radians
     */
    void RotateAroundBase(const glm::dvec2 &BaseCoord, double Angle);

    /**
     * Create a deep copy of this geometry.
     *
     * @return a pointer to the cloned geometry
     */
    virtual Geometry *clone() const = 0;

    /**
     * Clear all content of this geometry.
     */
    virtual void Clear() = 0;

    /**
     * Test whether this geometry is equal to another geometry.
     *
     * @param geom the geometry to compare with
     * @return true if geometries are equal, false otherwise
     */
    virtual bool equals(const Geometry *geom) const = 0;

    /**
     * Get the total number of points in this geometry.
     *
     * @return the number of points
     */
    virtual size_t getNumPoints() const = 0;

    /**
     * Get the number of geometries in this collection.
     *
     * @return the number of geometries
     */
    virtual size_t getNumGeometries() const
    {
        return 1;
    }

    /**
     * Test whether this geometry is simple.
     * Simple geometries are: Point, LineString.
     *
     * @return true if the geometry is simple, false otherwise
     */
    virtual bool isSimple() const = 0;

    /**
     * Get the type identifier of this geometry.
     *
     * @return the geometry type ID
     */
    virtual gceGeometryType getGeometryTypeId() const = 0;

    /**
     * Check whether this geometry is empty (contains no points).
     *
     * @return true if empty, false otherwise
     */
    virtual bool isEmpty() const = 0;

    /**
     * Get the maximum dimension of geometries in this collection.
     *
     * @return the dimension (0=point, 1=line, 2=surface)
     */
    virtual int getDimension() const = 0;

    /**
     * Get the bounding box of this geometry.
     *
     * @return the bounding box
     */
    geom::Box3D bbox() const;

    /*!
     * @brief minimal distance from 2D position to geometry
     * @param pos 2D position
     * @return distance in currenty coordinate system
     */
    virtual double distance(const glm::dvec2 &pos) const = 0;

    /**
     * Calculate the area of this geometry.
     *
     * @return the area
     */
    virtual double area() const
    {
        return 0.0;
    }

    /**
     * Calculate the length (perimeter) of this geometry.
     *
     * @return the length
     */
    virtual double length() const
    {
        return 0.0;
    }

    /**
     * Calculate the 3D length of this geometry, taking the Z coordinate into account.
     *
     * @return the 3D length
     */
    virtual double length3d() const
    {
        return 0.0;
    }

    /**
     * Convert this geometry to GEOS format.
     *
     * @return the geometry in GEOS format
     */
    virtual GEOSGeom_scoped_t toGEOSGeom() const = 0;

    /*!
     * @brief Call handler operators for every basic geometry: Point, LineString or Polygon
     * @param filter user-defined filter implementation
     */
    virtual void apply_geometry_filter(GeometryFilter &filter) const = 0;

    /**
     * Apply a coordinate filter that modifies the geometry in place.
     * Example: affine transformation.
     *
     * @param filter the read-write filter to apply
     */
    virtual void apply_filter_rw(filter_rw &filter) = 0;

    /**
     * Apply a read-only coordinate filter without modifying the geometry.
     * Example: bounding box calculation.
     *
     * @param filter the read-only filter to apply
     */
    virtual void apply_filter_ro(filter_ro &filter) const = 0;

    /**
     * Get the coordinate type of this geometry.
     *
     * @return the coordinate type
     */
    CoordinateType getCoordinateType() const
    {
        return m_coordType;
    }

    /*!
     * @brief find basic geometry (Point, LineString, Polygon) for vertex with index idx,
     * Geometry pointer may invalidate if parent geometry is changed
     *
     * @param idx vertex index in geometry
     * @return vertex index realtive to geometry (always zero for Points), pointer to geometry
     */
    virtual std::pair<size_t, const Geometry *> getGeometryForVertex(size_t idx) const = 0;

    /*!
     * @brief remove vertex, may throw std::logic_error on failure
     * @param idx vertex index
     */
    virtual void removeVertex(size_t idx) = 0;

    /*!
     * @brief remove vertex, may throw std::logic_error on failure
     * @param idx vertex index
     * @param new coordinate
     */
    virtual void updateVertex(size_t idx, const Coordinate &coo) = 0;

    /*!
     * @brief insert vertex before idx
     * @param idx vertex index
     * @param coo coordinate of new vertex
     */
    virtual void insertVertex(size_t idx, const Coordinate &coo) = 0;

    /**
     * Convert this geometry to Well-Known Text (WKT) format.
     *
     * @return the WKT representation of this geometry
     */
    std::string toWKT() const;

    //! the coordinate type factory for stored coordinates
    const CoordinateType m_coordType;
};

}; // namespace geom
