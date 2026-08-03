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

#include <limits>
#include <cmath>
#include <glm/glm.hpp>
#include "cxxcompat.h"

/**
 * Constant representing positive infinity for double.
 */
inline constexpr double DoubleInfinity = std::numeric_limits<double>::infinity();


namespace gce
{

/**
 * Round a value to the nearest multiple of step.
 *
 * @param val value to round
 * @param step rounding step (must be > 0)
 * @return value rounded to nearest multiple of step
 */
inline double round2(double val, double step)
{
    return step * std::round(val / step);
}

/**
 * Round a value to the nearest multiple of step with an offset base.
 *
 * @param val value to round
 * @param step rounding step (must be > 0)
 * @param base offset base
 * @return value rounded to nearest multiple of step relative to base
 */
inline double round2_base(double val, double step, double base)
{
    return step * std::round((val - base) / step) + base;
}

/**
 * Return the smallest value greater than or equal to val that is a multiple of step.
 *
 * @param val input value
 * @param step step size (must be > 0)
 * @return smallest multiple of step that is >= val
 */
inline double ceil2(double val, double step)
{
    return step * std::ceil(val / step);
}

/**
 * Return the largest value less than or equal to val that is a multiple of step.
 *
 * @param val input value
 * @param step step size (must be > 0)
 * @return largest multiple of step that is <= val
 */
inline double floor2(double val, double step)
{
    return step * std::floor(val / step);
}

/**
 * Returns a "large" epsilon value scaled from numeric_limits<T>::epsilon().
 *
 * @tparam T floating point type
 * @return scaled epsilon (100 * machine epsilon)
 */
template <class T>
constexpr T big_epsilon() noexcept
{
    return T(100) * std::numeric_limits<T>::epsilon();
}

/**
 * Compute the elevation angle (angle around X-Y plane) of a 3D vector.
 *
 * Equivalent to atan2(z, sqrt(x^2 + y^2)).
 *
 * @param vec input 3D vector
 * @return elevation angle in radians
 */
inline double angleZ(const glm::dvec3 &vec)
{
    return std::atan2(vec.z, std::sqrt(vec.x * vec.x + vec.y * vec.y));
}

/**
 * Compute the planar angle (atan2(y, x)) for vectors with at least 2 components.
 *
 * @tparam L dimension (must be >= 2)
 * @tparam T component type
 * @tparam Q glm qualifier
 * @param vec input vector with at least 2 components
 * @return angle in radians (atan2(y, x))
 */
template<glm::length_t L, typename T, glm::qualifier Q>
double angleX(const glm::vec<L, T, Q> &vec) GCE_WITH_REQUIRES(L >= 2)
{
    static_assert(L >= 2, "vector components 2 or more required");
    return std::atan2(vec.y, vec.x);
}

/**
 * Compute the azimuth in degrees using atan2(x, y).
 *
 * Note: this convention uses (x, y) order intentionally.
 *
 * @param vec input vector with at least 2 components
 * @return azimuth in degrees
 */
template<glm::length_t L, typename T, glm::qualifier Q>
double azimuth(const glm::vec<L, T, Q> &vec) GCE_WITH_REQUIRES(L >= 2)
{
    static_assert(L >= 2, "vector components 2 or more required");
    return glm::degrees(std::atan2(vec.x, vec.y));
}

/**
 * Return a 3D vector perpendicular to the XY projection of v.
 *
 * The returned vector lies in the XY plane (z = 0).
 *
 * @param v input 3D vector
 * @return perpendicular 3D vector (-y, x, 0)
 */
inline glm::dvec3 perpVector(const glm::dvec3 &v)
{
    return glm::dvec3(-v.y, v.x, 0.0);
}

/**
 * Return a 2D vector perpendicular to v.
 *
 * @param v input 2D vector
 * @return perpendicular 2D vector (-y, x)
 */
inline glm::dvec2 perpVector(const glm::dvec2 &v)
{
    return glm::dvec2(-v.y, v.x);
}

/**
 * Compute signed angle from base to A in plane using perpendicular and dot products.
 *
 * angle = atan2( dot(A, perp(base)), dot(A, base) )
 *
 * @param A target vector
 * @param base base/reference vector
 * @return signed angle in radians
 */
inline double angle(glm::dvec2 const &A, glm::dvec2 const &base)
{
    return std::atan2(glm::dot(A, perpVector(base)), glm::dot(A, base));
}

/**
 * Squared Euclidean distance between two vectors.
 *
 * @tparam L dimension
 * @tparam T component type
 * @tparam Q glm qualifier
 * @param A first vector
 * @param B second vector
 * @return squared distance (||B - A||^2)
 */
template<glm::length_t L, typename T, glm::qualifier Q>
double distance2(const glm::vec<L, T, Q> &A, const glm::vec<L, T, Q> &B)
{
    const auto x = B - A;
    return glm::dot(x, x);
}

/**
 * Computes the scalar 2D cross product (z component) of two 2D vectors.
 *
 * Returns the signed area of the parallelogram spanned by a and b:
 * a.x * b.y − a.y * b.x
 *
 * @tparam T numeric scalar type (e.g., float, double)
 * @tparam Q glm qualifier for vector storage/precision
 * @param a first 2D vector
 * @param b second 2D vector
 * @return the scalar cross product (z component) of a and b
 */
template<typename T, glm::qualifier Q>
constexpr T cross(const glm::vec<2, T, Q> &a, const glm::vec<2, T, Q> &b)
{
    return a.x * b.y - a.y * b.x;
}

/**
 * Test whether vector magnitude is smaller than eps.
 *
 * @tparam L dimension
 * @tparam T component type
 * @tparam Q glm qualifier
 * @param A input vector
 * @param eps threshold (default big_epsilon<double>())
 * @return true if ||A||^2 < eps
 */
template<glm::length_t L, typename T, glm::qualifier Q>
bool isZeroEpsilon(const glm::vec<L, T, Q> &A, const double eps = big_epsilon<double>())
{
    return glm::dot(A, A) < eps;
}

/**
 * Test whether two vectors are approximately equal within eps.
 *
 * Comparison uses squared distance.
 *
 * @tparam L dimension
 * @tparam T component type
 * @tparam Q glm qualifier
 * @param A first vector
 * @param B second vector
 * @param eps threshold (default big_epsilon<T>())
 * @return true if squared distance < eps
 */
template<glm::length_t L, typename T, glm::qualifier Q>
bool equalsEpsilon(const glm::vec<L, T, Q> &A, const glm::vec<L, T, Q> &B, const T eps = big_epsilon<T>())
{
    return distance2(A, B) < eps;
}

/**
 * Lexicographical comparison of vector positions.
 *
 * Iterates components in increasing index order.
 *
 * @tparam L dimension
 * @tparam T component type (supports < and >)
 * @tparam Q glm qualifier
 * @param A first vector
 * @param B second vector
 * @return -1 if A < B, 1 if A > B, 0 if equal
 */
template<glm::length_t L, typename T, glm::qualifier Q>
int ComparePos(const glm::vec<L, T, Q> &A, const glm::vec<L, T, Q> &B)
{
    for (int i = 0; i < static_cast<int>(L); ++i)
    {
        if (A[i] < B[i]) return -1;
        if (A[i] > B[i]) return 1;
    }
    return 0;
}

/**
 * Check whether vectors are parallel within eps.
 *
 * Uses the 2D cross product (scalar) test - 2D case.
 * or uses cross product and tests whether resulting vector length is near zero.
 *
 * @param a first vector
 * @param b second vector
 * @param eps tolerance (default big_epsilon<T>())
 * @return true if vectors are parallel
 */
template<glm::length_t L, typename T, glm::qualifier Q>
bool parallel(const glm::vec<L, T, Q> &a, const glm::vec<L, T, Q> &b, double eps = gce::big_epsilon<T>())
{
    if constexpr (L == 2)
        return std::abs(a.x * b.y - a.y * b.x) < eps;
    else
        return isZeroEpsilon(glm::cross(a, b), eps);
}

/**
 * Check whether three points are collinear within eps.
 *
 * @param A first point
 * @param B second point
 * @param C third point
 * @param eps tolerance (default big_epsilon<T>())
 * @return true if points are collinear
 */
template<glm::length_t L, typename T, glm::qualifier Q>
bool pointsOnLine(const glm::vec<L, T, Q> &A, const glm::vec<L, T, Q> &B, const glm::vec<L, T, Q> &C, double eps = gce::big_epsilon<double>())
{
    return gce::parallel(B - A, C - B, eps);
}

/**
 * Compute the scalar projection factor of vector AC onto vector AB.
 *
 * The returned value f is the scalar such that the projection of AC onto AB
 * equals f × AB. Numerically, f = (AC · AB) / (AB · AB). If AB is (near) zero,
 * the function returns 0 to avoid division by zero (uses big_epsilon<T>() check).
 *
 * @tparam L the number of components in the vectors
 * @tparam T the scalar component type
 * @tparam Q the qualifier for glm (precision/qualifier tag)
 *
 * @param AC the vector from A to C
 * @param AB the vector from A to B
 * @return the scalar projection factor f where projection = f × AB
 */
template<glm::length_t L, typename T, glm::qualifier Q>
constexpr T projectionFactor(const glm::vec<L, T, Q> &AC, const glm::vec<L, T, Q> &AB)
{
    const T dotAB = glm::dot(AB, AB);
    if (dotAB < big_epsilon<T>()) return T(0);
    return glm::dot(AC, AB) / dotAB;
}

/**
 * Computes the projection factor r of point p onto the line defined by A and B.
 *
 * The method uses the frequent computational-geometry formula:
 *   r = (AC · AB) / (AB · AB)
 * where AC = p - A and AB = B - A.
 *
 * The value r has the following meaning
 *   r = 0    -> projected point P coincides with A
 *   r = 1    -> projected point P coincides with B
 *   r < 0    -> P lies on the extension of AB before A (backward extension)
 *   r > 1    -> P lies on the extension of AB beyond B (forward extension)
 *   0 < r < 1 -> P lies between A and B (interior to segment AB)
 *
 * Template parameters
 *   @tparam L The number of components in the vector (vec length).
 *   @tparam T The scalar numeric type (e.g., float, double).
 *   @tparam Q The qualifier for the glm vector (precision/qualifier).
 *
 * Parameters
 *   @param p The point to project.
 *   @param A The segment start point.
 *   @param B The segment end point.
 *
 * Return value
 *   @return The scalar projection factor r of p onto the line AB. If AB is
 *           degenerate (|AB|^2 is very small), the function returns 0.
 *
 * Notes
 *   - Uses dot(AB, AB) to compute AB · AB (squared length of AB).
 *   - Comparison to zero uses gce::big_epsilon<T>() to guard against degenerate AB.
 */
template<glm::length_t L, typename T, glm::qualifier Q>
constexpr T projectionFactor(const glm::vec<L, T, Q> &p, const glm::vec<L, T, Q> &A, const glm::vec<L, T, Q> &B)
{
    return projectionFactor(p - A, B - A);
}

/**
 * Create a 4x4 transform matrix from basis vectors and origin.
 *
 * Matrix columns are e1, e2, e3 and translation r. The matrix maps local coordinates
 * (x, y, z, 1) into world coordinates as r + x*e1 + y*e2 + z*e3.
 *
 * @param e1 first basis vector (world-space column)
 * @param e2 second basis vector (world-space column)
 * @param e3 third basis vector (world-space column)
 * @param r translation / origin in world space
 * @return 4x4 transform matrix
 */
inline glm::dmat4 transformBasis(const glm::dvec3 &e1, const glm::dvec3 &e2, const glm::dvec3 &e3, const glm::dvec3 &r)
{
    return glm::dmat4(glm::dvec4(e1, 0.0), glm::dvec4(e2, 0.0), glm::dvec4(e3, 0.0), glm::dvec4(r, 1.0));
}

} // namespace gce

