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

/// @file Coordinate.h
/// @brief Coordinate representation with support for 2D, 3D, and measure dimensions
///
/// This file provides type-safe, constexpr-compatible coordinate classes that support
/// various coordinate dimensionalities:
/// - XY: 2D coordinates (X, Y)
/// - XYZ: 3D coordinates (X, Y, Z)
/// - XYM: 2D coordinates with measure (X, Y, M)
/// - XYZM: 3D coordinates with measure (X, Y, Z, M)
///
/// The implementation uses template specialization to minimize memory overhead and
/// provide compile-time type safety with C++20 concepts and constraints.

#include "alg/gc_algebra.h"

namespace geom
{
/// @brief Sentinel value representing "Not a Number" for invalid coordinates
constexpr inline double DoubleNotANumber = std::numeric_limits<double>::quiet_NaN();

/// @enum CoordinateType
/// @brief Enumeration of supported coordinate dimensions
///
/// Specifies which coordinate dimensions are present in a coordinate:
/// - XY: 2D coordinates (X, Y only)
/// - XYZ: 3D coordinates (X, Y, Z; no measure)
/// - XYM: 2D with measure (X, Y, M; no elevation)
/// - XYZM: Full 4D coordinates (X, Y, Z, M)
enum class CoordinateType : uint8_t
{
    XY,      ///< 2D coordinate (X, Y)
    XYZ,     ///< 3D coordinate (X, Y, Z)
    XYM,     ///< 2D coordinate with measure (X, Y, M)
    XYZM     ///< 3D coordinate with measure (X, Y, Z, M)
};

/// @brief Check if a coordinate type includes the Z dimension
/// @param coordType The coordinate type to check
/// @return true if the coordinate type is XYZ or XYZM; false otherwise
constexpr bool hasZ(CoordinateType coordType)
{
    return coordType == CoordinateType::XYZ || coordType == CoordinateType::XYZM;
}

/// @brief Check if a coordinate type includes the measure dimension
/// @param coordType The coordinate type to check
/// @return true if the coordinate type is XYM or XYZM; false otherwise
constexpr bool hasM(CoordinateType coordType)
{
    return coordType == CoordinateType::XYM || coordType == CoordinateType::XYZM;
}

/// @brief Get the number of dimensions for a coordinate type
/// @param coordType The coordinate type to query
/// @return The number of dimensions (2, 3, or 4)
constexpr std::uint8_t dimensions(CoordinateType coordType)
{
    switch (coordType)
    {
    case geom::CoordinateType::XY:
        return 2;
    case geom::CoordinateType::XYZ:
    case geom::CoordinateType::XYM:
        return 3;
    case geom::CoordinateType::XYZM:
        return 4;
    }
    return 0;
}

/// @brief Ordinate dimension constants (X, Y, Z, M)
enum class OrdinateIndex { X, Y, Z, M };

/// @brief Default Z coordinate value when not specified
constexpr inline double DEFAULT_C = 0.0;
/// @brief Default measure (M) coordinate value when not specified
constexpr inline double DEFAULT_M = 0.0;


/// @namespace detail
/// @brief Implementation details for coordinate types
/// 
/// This namespace contains template base classes that implement the core
/// position and measure storage for coordinates. Users should not directly
/// use types in this namespace; use the public aliases instead.
namespace detail
{
/// @class pos_impl
/// @brief Base class for position storage (X, Y, and optionally Z)
/// 
/// @tparam Z If true, stores 3D position (glm::dvec3); if false, stores 2D position (glm::dvec2)
/// 
/// This CRTP-style base class handles the spatial position component of a coordinate.
/// It is specialized to store either 2D or 3D positions depending on the template parameter.
template <bool Z> struct pos_impl {};

template <> struct pos_impl<true>
{
    /// @brief Number of position dimensions (2 for 2D, 3 for 3D)
    constexpr static uint8_t N = 3;

    /// @brief Vector type used for position storage
    using pos_type = glm::dvec3;

    /// @brief Constructor for 3D coordinates from a 2D position (initializes Z with DEFAULT_C)
    /// @param pos The 2D position vector; Z will be set to DEFAULT_C
    constexpr explicit pos_impl(const glm::dvec2 &pos) : pos(pos, DEFAULT_C) {}

    /// @brief Constructor for 3D coordinates
    /// @param pos The 3D position vector
    constexpr explicit pos_impl(const glm::dvec3 &pos) : pos(pos) {}

    /// @brief Equality comparison for position vectors
    /// @param rhs The other position to compare with
    /// @return true if both position vectors are equal
    constexpr bool operator==(const pos_impl<true> &rhs) const { return pos == rhs.pos; }

    /// @brief The position vector (2D or 3D depending on template parameter Z)
    pos_type pos;
};
template <> struct pos_impl<false>
{
    /// @brief Number of position dimensions (2 for 2D, 3 for 3D)
    constexpr static uint8_t N = 2;

    /// @brief Vector type used for position storage
    using pos_type = glm::dvec2;

    /// @brief Constructor for 2D coordinates
    /// @param pos The 2D position vector
    constexpr explicit pos_impl(const glm::dvec2 &pos) : pos(pos) {}

    /// @brief Constructor for 3D coordinates
    /// @param pos The 3D position vector
    constexpr explicit pos_impl(const glm::dvec3 &pos) : pos(pos) {}

    /// @brief Equality comparison for position vectors
    /// @param rhs The other position to compare with
    /// @return true if both position vectors are equal
    constexpr bool operator==(const pos_impl<false> &rhs) const { return pos == rhs.pos; }

    /// @brief The position vector (2D or 3D depending on template parameter Z)
    pos_type pos;
};

/// @class m_impl
/// @brief Base class for measure (M) dimension storage
///
/// @tparam M If true, stores a measure value; if false, measure is not stored (0 bytes overhead)
/// 
/// This class is specialized to either store the measure dimension or provide a
/// no-op implementation to avoid memory overhead when measure is not needed.
template <bool M> struct m_impl {};

/// @brief Specialization of m_impl for coordinates WITH measure dimension
template <> struct m_impl<true>
{
    /// @brief Constructor initializing the measure value
    /// @param m The measure value to store
    constexpr explicit m_impl(double m) : m(m) {}

    /// @brief Equality comparison for measure values
    /// @param rhs The other measure to compare with
    /// @return true if both measure values are equal
    constexpr bool operator==(const m_impl<true> &rhs) const { return m == rhs.m; }

    /// @brief The measure coordinate value
    double m;
};

/// @brief Specialization of m_impl for coordinates WITHOUT measure dimension
///
/// This specialization has no data members or storage, providing zero overhead
/// for coordinate types that don't use the measure dimension.
template <> struct m_impl<false>
{
    /// @brief Default constructor (no-op)
    constexpr explicit m_impl() = default;

    /// @brief Dummy parameter constructor (no-op, ignores the measure parameter)
    /// @param Measure parameter to ignore (no-op for this specialization)
    constexpr explicit m_impl(double) {}

    /// @brief Equality comparison for measure (always true, as no measure is stored)
    /// @param rhs The other measure (ignored, no-op)
    /// @return Always true since there is no measure dimension
    constexpr bool operator==(const m_impl<false> &) const { return true; }
};

/// @brief Extract the measure value from a coordinate with measure
/// @param val The coordinate's m_impl instance
/// @return The stored measure value
constexpr inline double get_m(const m_impl<true> val) { return val.m; }

/// @brief Extract the measure value from a coordinate without measure
/// @return DEFAULT_M (the default measure value)
constexpr inline double get_m(const m_impl<false>) { return DEFAULT_M; }

/// @class coo_impl
/// @brief Generic coordinate implementation class
///
/// @tparam Z If true, includes Z dimension; if false, 2D only
/// @tparam M If true, includes measure dimension; if false, no measure
/// 
/// This class combines position and measure storage via CRTP inheritance from
/// pos_impl and m_impl. It provides constructors for all combinations of dimensions
/// and conversion between coordinate types.
template <bool Z, bool M> class coo_impl : public pos_impl<Z>, public m_impl<M>
{
    using pos_type = typename pos_impl<Z>::pos_type;
public:
    /// @brief Default constructor, initializes all dimensions to default values
    constexpr coo_impl() : pos_impl<Z>(typename pos_impl<Z>::pos_type(DEFAULT_C)), m_impl<M>(DEFAULT_M) {}

    /// @brief components constructor
    /// @param pos The position vector
    /// @param m The measure value
    constexpr coo_impl(const pos_type &pos, double m = DEFAULT_M) : pos_impl<Z>(pos), m_impl<M>(m) {}


    /// @}
    /// @name Type conversion constructor
    /// @{

    /// @brief Construct from another coordinate type, converting dimensions as needed
    ///
    /// This template constructor allows creation of a coordinate of one type from
    /// another. Missing dimensions are filled with default values; extra dimensions
    /// are discarded.
    ///
    /// @tparam L The source coordinate's Z flag
    /// @tparam F The source coordinate's measure flag
    /// @param c The source coordinate to convert from
    template <bool L, bool F>
    constexpr coo_impl(const coo_impl<L, F> &c) : pos_impl<Z>(c.pos), m_impl<M>(get_m(c)) {}

    /// @}

    /// @brief Check if this coordinate type includes measure dimension
    /// @return true if M template parameter is true
    static constexpr bool hasM()
    {
        return M;
    }

    /// @brief Check if this coordinate type includes Z dimension
    /// @return true if Z template parameter is true
    static constexpr bool hasZ()
    {
        return Z;
    }

    /// @brief Get the number of dimensions in this coordinate type
    /// @return The number of position dimensions (2 or 3)
    static constexpr uint8_t ndims()
    {
        return pos_impl<Z>::N;
    }

    static constexpr CoordinateType format()
    {
        if (Z && M)
            return CoordinateType::XYZM;
        else if (Z)
            return CoordinateType::XYZ;
        else if (M)
            return CoordinateType::XYM;
        else
            return CoordinateType::XY;
    }

    /// @brief Equality comparison for coordinates
    /// 
    /// Compares both position and measure dimensions for equality.
    /// Note that NaN values are never equal to anything (including themselves),
    /// per IEEE 754 semantics.
    ///
    /// @param rhs The other coordinate to compare with
    /// @return true if both coordinates have identical position and measure values
    constexpr bool operator==(const coo_impl<Z, M> &rhs) const
    {
        //return this->pos == rhs.pos && get_m(*this) == get_m(rhs);
        return pos_impl<Z>::operator==(rhs) && m_impl<M>::operator==(rhs);
    }

    /// @brief Compare this coordinate with another using 2D comparison
    ///
    /// Compares coordinates using only their 2D projections (X and Y values),
    /// ignoring Z and M dimensions.
    ///
    /// @param other The coordinate to compare with
    /// @return Negative if this < other, zero if equal, positive if this > other
    int compareTo(const coo_impl &other) const
    {
        return gce::ComparePos(this->pos, other.pos);
    }

    void setOrdinate(OrdinateIndex idx, double value)
    {
        switch (idx)
        {
        case OrdinateIndex::X: this->pos.x = value; break;
        case OrdinateIndex::Y: this->pos.y = value; break;
        case OrdinateIndex::Z: if constexpr (Z) this->pos.z = value; break;
        case OrdinateIndex::M: if constexpr (M) this->m = value; break;
        }
    }
    /// @brief Create a null/invalid coordinate with NaN values
    /// @return A coordinate where all dimensions are set to quiet NaN
    static constexpr coo_impl getNull() { return coo_impl(pos_impl<Z>::pos_type(DoubleNotANumber), DoubleNotANumber); }
};
} // namespace detail

/// @name Coordinate type aliases
/// @{

/// @brief 2D coordinate type (X, Y)
/// 
/// No Z dimension, no measure. Use for planar 2D geometry.
using CoordinateXY = detail::coo_impl<false, false>;

/// @brief 3D coordinate type (X, Y, Z)
/// 
/// Includes Z dimension, no measure. Use for 3D geometry without linear referencing.
using CoordinateXYZ = detail::coo_impl<true, false>;

/// @brief 2D coordinate with measure (X, Y, M)
/// 
/// No Z dimension, includes measure. Use for 2D linear referencing (LRS).
using CoordinateXYM = detail::coo_impl<false, true>;

/// @brief 3D coordinate with measure (X, Y, Z, M)
/// 
/// Includes both Z dimension and measure. Use for 3D linear referencing.
using CoordinateXYZM = detail::coo_impl<true, true>;

/// @}

/// @brief Default coordinate type alias
/// 
/// Provides the most feature-complete coordinate type (XYZM) as the default
/// for general use. Can be specialized based on application needs.
typedef CoordinateXYZM Coordinate;

/// @brief Linear interpolation between two coordinates
/// 
/// Performs element-wise linear interpolation (lerp) between two coordinates,
/// including the measure dimension if present.
///
/// @param c1 The start coordinate
/// @param c2 The end coordinate
/// @param t The interpolation factor (0.0 = c1, 1.0 = c2, 0.5 = midpoint)
/// @return A new coordinate at the interpolated position
inline Coordinate mix(const Coordinate &c1, const Coordinate &c2, double t)
{
    return Coordinate(glm::mix(c1.pos, c2.pos, t), glm::mix(c1.m, c2.m, t));
}

/// @brief Inequality comparison for XYZM coordinates
/// 
/// Compares both position and measure dimensions for inequality.
///
/// @param A The first coordinate
/// @param B The second coordinate
/// @return true if coordinates differ in position or measure
inline bool operator!=(const CoordinateXYZM &A, const CoordinateXYZM &B)
{
    return !(A == B);
}

/// @brief Compute 2D Euclidean distance between two XYZ coordinates
/// 
/// Calculates the distance using only the X and Y components, ignoring
/// the Z dimension.
///
/// @param c1 The first coordinate
/// @param c2 The second coordinate
/// @return The Euclidean distance in the XY plane
template <bool Z, bool M>
inline double distance2d(const detail::coo_impl<Z, M> &c1, const detail::coo_impl<Z, M> &c2)
{
    return glm::distance(glm::dvec2(c1.pos), glm::dvec2(c2.pos));
}

/// @brief Compute 3D Euclidean distance between two XYZM coordinates
/// 
/// Calculates the distance using X, Y, and Z components, ignoring the
/// measure dimension.
///
/// @param c1 The first coordinate
/// @param c2 The second coordinate
/// @return The Euclidean distance in 3D space
template <bool Z, bool M>
inline double distance3d(const detail::coo_impl<Z, M> &c1, const detail::coo_impl<Z, M> &c2)
{
    return glm::distance(c1.pos, c2.pos);
}

} // namespace geom
