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
/// @file alg_geos.h
/// @brief GEOS-based geometric algorithms and operations
/// @details Provides inline wrapper functions for GEOS geometric operations with support
/// for both GEOS geometry objects and Geometry class wrappers. All operations maintain
/// coordinate type information through the wrapping layer.

#include "geom/Geometry.h"

namespace geom
{
/**
 * @namespace alg_geos
 * @brief Geometric algorithms using the GEOS (Geometry Engine Open Source) library
 *
 * @details This namespace provides efficient implementations of common geometric operations
 * such as differences, intersections, unions, and validity checks. All functions operate on
 * both low-level GEOS geometry objects and high-level Geometry wrappers, with seamless
 * conversion between the two representations.
 *
 * The functions are implemented as inline wrappers around GEOS functions to minimize
 * overhead while providing a consistent C++ interface and proper resource management
 * through smart pointers.
 */
namespace alg_geos
{

/**
 * @brief Computes the set difference of two GEOS geometries.
 *
 * @param A The first GEOS geometry (minuend)
 * @param B The second GEOS geometry (subtrahend)
 * @return A scoped GEOS geometry representing A \ B (all points in A but not in B)
 *
 * @details The result represents the geometric difference operation: all parts of geometry A
 * that are not contained in or overlapping with geometry B. The returned geometry is
 * automatically managed and will be freed when the scoped pointer is destroyed.
 *
 * @note The input geometries are not modified by this operation.
 * @see GeomIntersection, GeomUnion
 */
inline GEOSGeom_scoped_t GeomDifference(const GEOSGeom_scoped_t &A, const GEOSGeom_scoped_t &B)
{
    return GEOSGeom_scoped_t{GEOSDifference(A.get(), B.get())};
}

/**
 * @brief Computes the set difference of two Geometry objects.
 *
 * @param A The first geometry (minuend)
 * @param B The second geometry (subtrahend)
 * @return A unique pointer to a new Geometry representing A \ B
 *
 * @details The result represents the geometric difference operation: all parts of geometry A
 * that are not contained in or overlapping with geometry B. The returned geometry maintains
 * the coordinate type of the first geometry A. The operation is performed using GEOS
 * internally and the result is wrapped in a managed Geometry object.
 *
 * @note Both input geometries are not modified by this operation.
 * @note The coordinate type is preserved from geometry A.
 * @see Difference, GeomUnion
 */
inline std::unique_ptr<Geometry> Difference(const Geometry &A, const Geometry &B)
{
    return Geometry::Create(A.getCoordinateType(), GeomDifference(A.toGEOSGeom(), B.toGEOSGeom()));
}

/**
 * @brief Computes the intersection of two GEOS geometries.
 *
 * @param A The first GEOS geometry
 * @param B The second GEOS geometry
 * @return A scoped GEOS geometry representing the intersection of A and B
 *
 * @details The result contains only the portions of geometries A and B that overlap or
 * share common area. If the geometries do not intersect, the result will be an empty
 * geometry. The returned geometry is automatically managed and will be freed when the
 * scoped pointer is destroyed.
 *
 * @note The input geometries are not modified by this operation.
 * @see GeomDifference, GeomUnion
 */
inline GEOSGeom_scoped_t GeomIntersection(const GEOSGeom_scoped_t &A, const GEOSGeom_scoped_t &B)
{
    return GEOSGeom_scoped_t{GEOSIntersection(A.get(), B.get())};
}

/**
 * @brief Computes the union of two GEOS geometries.
 *
 * @param A The first GEOS geometry
 * @param B The second GEOS geometry
 * @return A scoped GEOS geometry representing the union of A and B
 *
 * @details The result contains all points that are in either geometry A or B (or both).
 * This is the standard binary union operation. The returned geometry is automatically
 * managed and will be freed when the scoped pointer is destroyed. The result may be a
 * collection or aggregated geometry depending on the input geometries.
 *
 * @note The input geometries are not modified by this operation.
 * @see GeomUnaryUnion, GeomDifference, GeomIntersection
 */
inline GEOSGeom_scoped_t GeomUnion(const GEOSGeom_scoped_t &A, const GEOSGeom_scoped_t &B)
{
    return GEOSGeom_scoped_t{GEOSUnion(A.get(), B.get())};
}

/**
 * @brief Computes the union of two Geometry objects.
 *
 * @param A The first geometry
 * @param B The second geometry
 * @return A unique pointer to a new Geometry representing the union of A and B
 *
 * @details The result contains all points that are in either geometry A or B (or both).
 * The returned geometry maintains the coordinate type of the first geometry A. The operation
 * is performed using GEOS internally and the result is wrapped in a managed Geometry object.
 *
 * @note Both input geometries are not modified by this operation.
 * @note The coordinate type is preserved from geometry A.
 * @see GeomUnaryUnion, Difference, GeomIntersection
 */
inline std::unique_ptr<Geometry> GeomUnion(const Geometry &A, const Geometry &B)
{
    return Geometry::Create(A.getCoordinateType(), GeomUnion(A.toGEOSGeom(), B.toGEOSGeom()));
}

/**
 * @brief Computes the unary union of a single GEOS geometry.
 *
 * @param A The GEOS geometry to union with itself
 * @return A scoped GEOS geometry representing the unary union of A
 *
 * @details Performs a union operation on a single geometry, which is useful for merging
 * connected parts of a geometry into a single entity. For geometries that are not connected,
 * this effectively converts them to a multi-geometry or leaves them unchanged.
 * The returned geometry is automatically managed and will be freed when the scoped pointer
 * is destroyed.
 *
 * @note The input geometry is not modified by this operation.
 * @details Unary union is often used to clean up or normalize geometry representations.
 * @see GeomUnion
 */
inline GEOSGeom_scoped_t GeomUnaryUnion(const GEOSGeom_scoped_t &A)
{
    return GEOSGeom_scoped_t{GEOSUnaryUnion(A.get())};
}

/**
 * @brief Validates and repairs invalid GEOS geometries.
 *
 * @param A The GEOS geometry to validate and repair
 * @return A scoped GEOS geometry representing the validated/repaired geometry
 *
 * @details This function attempts to fix invalid geometries by applying repair operations.
 * Invalid geometries may result from coordinate precision issues, self-intersections, or
 * other geometric inconsistencies. The result is a valid geometry that represents the
 * original geometry as closely as possible. The returned geometry is automatically managed
 * and will be freed when the scoped pointer is destroyed.
 *
 * @note The input geometry is not modified by this operation.
 * @note This is useful for cleaning up data from external sources or after coordinate
 *       transformations that may introduce small errors.
 * @see MakeValid(const Geometry&)
 */
inline GEOSGeom_scoped_t MakeValid(const GEOSGeom_scoped_t &A)
{
    return GEOSGeom_scoped_t{GEOSMakeValid(A.get())};
}

/**
 * @brief Validates and repairs an invalid Geometry object.
 *
 * @param A The geometry to validate and repair
 * @return A unique pointer to a new Geometry representing the validated/repaired geometry
 *
 * @details This function attempts to fix invalid geometries by applying repair operations.
 * Invalid geometries may result from coordinate precision issues, self-intersections, or
 * other geometric inconsistencies. The result is a valid geometry that represents the
 * original geometry as closely as possible. The returned geometry maintains the coordinate
 * type of the input geometry A. The operation is performed using GEOS internally and the
 * result is wrapped in a managed Geometry object.
 *
 * @note The input geometry is not modified by this operation.
 * @note The coordinate type is preserved from geometry A.
 * @note This is useful for cleaning up data from external sources or after coordinate
 *       transformations that may introduce small errors.
 * @see MakeValid(const GEOSGeom_scoped_t&)
 */
inline std::unique_ptr<Geometry> MakeValid(const Geometry &A)
{
    return Geometry::Create(A.getCoordinateType(), MakeValid(A.toGEOSGeom()));
}

} // namespace alg_geos
} // namespace geom
