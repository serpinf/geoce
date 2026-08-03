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

/// @file CoordSeq.h
/// @brief Coordinate sequence container interface with geometric operations

#include "geos_c.h"
#include "Coordinate.h"

namespace geom
{
class filter_rw;
class filter_ro;

/**
 * @class CoordinateSeq
 * @brief A coordinate sequence container that manages a dynamic sequence of coordinates with support for 2D, 3D, and measured coordinates.
 *
 * CoordinateSeq provides an efficient way to store and manipulate sequences of coordinates,
 * supporting optional Z (elevation) and M (measure) dimensions. It uses a flat vector storage
 * internally with a stride-based approach for memory efficiency. The class provides iterators,
 * random access, and various geometric operations.
 *
 * @details
 * - Coordinates can be 2D (X, Y), 3D (X, Y, Z), or include M values (measure)
 * - The format is determined by a CoordinateType and stored in m_format
 * - Uses a stride-based storage model where each coordinate consumes m_stride doubles
 * - Supports initialization from std::initializer_list and std::vector
 * - Integrates with GEOS library for coordinate sequence conversion
 *
 * @see CoordinateType, Coordinate, filter_rw, filter_ro
 */
class CoordinateSeq
{
public:
    using iterator = std::vector<double>::iterator;
    using const_iterator = std::vector<double>::const_iterator;

    /**
     * @brief Constructs a CoordinateSeq with the specified coordinate format.
     *
     * @param format The coordinate type that determines whether Z and/or M values are present
     */
    explicit CoordinateSeq(CoordinateType format);

    /**
     * @brief Constructs a CoordinateSeq from an initializer list of coordinates.
     *
     * @tparam Z Template parameter indicating if Z coordinates are present
     * @tparam M Template parameter indicating if M (measure) values are present
     * @param coordinates Initializer list of coordinate objects
     *
     * @details Creates a CoordinateSeq and populates it with coordinates from the initializer list.
     * The sequence capacity is reserved before insertion.
     */
    template <bool Z, bool M>
    CoordinateSeq(std::initializer_list<detail::coo_impl<Z, M>> coordinates) : CoordinateSeq(detail::coo_impl<Z, M>::format())
    {
        reserve(coordinates.size());
        for (auto &coo : coordinates)
        {
            push_back(coo);
        }
    }

    /**
     * @brief Constructs a CoordinateSeq from another CoordinateSeq with format conversion.
     *
     * @param format The target coordinate format
     * @param other The source CoordinateSeq to copy coordinates from
     *
     * @details Creates a new CoordinateSeq with the specified format and appends all coordinates
     * from the source sequence, converting them as needed.
     */
    CoordinateSeq(CoordinateType format, const CoordinateSeq &other) : CoordinateSeq(format)
    {
        append(other);
    }

    /**
     * @brief Constructs a CoordinateSeq from a vector of coordinates with format conversion.
     *
     * @tparam Z Template parameter indicating if Z coordinates are present in the source
     * @tparam M Template parameter indicating if M values are present in the source
     * @param format The target coordinate format
     * @param vect Vector of source coordinates
     *
     * @details Creates a new CoordinateSeq with the specified format and appends all coordinates
     * from the vector, converting them as needed.
     */
    template <bool Z, bool M>
    CoordinateSeq(CoordinateType format, const std::vector<detail::coo_impl<Z, M>> &vect) : CoordinateSeq(format)
    {
        append(vect);
    }

    /**
     * @brief Default copy constructor.
     */
    CoordinateSeq(const CoordinateSeq &) = default;

    /**
     * @brief Checks if the coordinate sequence is empty.
     *
     * @return true if the sequence contains no coordinates, false otherwise
     */
    bool empty() const
    {
        return m_data.empty();
    }

    /**
     * @brief Replaces the contents with coordinates from another CoordinateSeq.
     *
     * @param other The source CoordinateSeq to copy from
     *
     * @details Clears the current sequence and appends all coordinates from the other sequence.
     */
    void assign(const CoordinateSeq &other)
    {
        m_data.clear();
        append(other);
    }

    /**
     * @brief Replaces the contents with coordinates from a vector.
     *
     * @tparam Z Template parameter indicating if Z coordinates are present
     * @tparam M Template parameter indicating if M values are present
     * @param vect Vector of coordinates to copy from
     *
     * @details Clears the current sequence and appends all coordinates from the vector.
     */
    template <bool Z, bool M>
    void assign(const std::vector<detail::coo_impl<Z, M>> &vect)
    {
        m_data.clear();
        append(vect);
    }

    /**
     * @brief Appends coordinates from another CoordinateSeq.
     *
     * @param other The source CoordinateSeq to copy from
     *
     * @details Appends all coordinates from the other sequence to the end of this sequence.
     */
    void append(const CoordinateSeq &other);

    /**
     * @brief Returns the number of coordinates in the sequence.
     *
     * @return The count of coordinates
     */
    size_t size() const
    {
        if (m_stride == 3) return m_data.size() / 3;
        if (m_stride == 4) return m_data.size() / 4;
        return m_data.size() / 2;
    }

    /**
     * @brief Resizes the coordinate sequence to contain exactly nsize coordinates.
     *
     * @param nsize The new number of coordinates
     *
     * @details If nsize is less than the current size, the sequence is truncated.
     * If nsize is greater, new coordinates are appended with default values.
     */
    void resize(size_t nsize)
    {
        m_data.resize(nsize * m_stride);
    }

    /**
     * @brief Reserves capacity for at least nsize coordinates without changing the size.
     *
     * @param nsize The number of coordinates for which to reserve space
     *
     * @details This is a performance hint; it does not change the sequence size.
     */
    void reserve(size_t nsize)
    {
        m_data.reserve(nsize * m_stride);
    }

    /**
     * @brief Removes all coordinates from the sequence.
     *
     * @details After this call, size() returns 0 and empty() returns true.
     */
    void clear()
    {
        m_data.clear();
    }

    /**
     * @brief Calculates the minimum distance from the sequence to a 2D position.
     *
     * @param pos The 2D position to measure distance to
     * @return The minimum distance from any coordinate in the sequence to the given position
     *
     * @details Uses Euclidean distance calculation considering only X and Y dimensions.
     */
    double distance(const glm::dvec2 &pos) const;

    /**
     * @brief Appends a coordinate to the end of the sequence.
     *
     * @tparam Z Template parameter indicating if Z coordinates are present
     * @tparam M Template parameter indicating if M values are present
     * @param c The coordinate to append
     *
     * @details Adds the coordinate at the end of the sequence. The internal storage
     * is expanded to accommodate the new coordinate according to the stride.
     */
    template <bool Z, bool M>
    void push_back(const detail::coo_impl<Z, M> &c)
    {
        set_at(c, m_data.insert(m_data.end(), m_stride, 0.0));
    }

    /**
     * @brief Appends a coordinate to the sequence with optional duplicate prevention.
     *
     * @tparam Z Template parameter indicating if Z coordinates are present
     * @tparam M Template parameter indicating if M values are present
     * @param c The coordinate to append
     * @param allowRep If true, allows duplicate consecutive coordinates; if false, skips duplicates
     *
     * @details If allowRep is false, the coordinate is only appended if the sequence is empty
     * or if the coordinate differs from the last coordinate (considering spatial coordinates only).
     */
    template <bool Z, bool M>
    void push_back(const detail::coo_impl<Z, M> &c, bool allowRep)
    {
        if (allowRep || m_data.empty() || !equal_if_inserted(c, std::prev(m_data.cend(), m_stride)))
        {
            push_back(c);
        }
    }

    /**
     * @brief Inserts a coordinate at the specified position.
     *
     * @tparam Z Template parameter indicating if Z coordinates are present
     * @tparam M Template parameter indicating if M values are present
     * @param pos The index at which to insert the coordinate
     * @param c The coordinate to insert
     *
     * @details Inserts the coordinate before the element at position pos.
     * Existing coordinates at pos and beyond are shifted forward.
     */
    template <bool Z, bool M>
    void Insert(size_t pos, const detail::coo_impl<Z, M> &c)
    {
        set_at(c, m_data.insert(get_iterator(pos), m_stride, 0.0));
    }

    /**
     * @brief Removes the coordinate at the specified position.
     *
     * @param pos The index of the coordinate to remove
     *
     * @details Erases the coordinate at position pos.
     * Coordinates after pos are shifted backward.
     */
    void Erase(size_t pos)
    {
        m_data.erase(get_iterator(pos), get_iterator(pos) + m_stride);
    }

    /**
     * @brief Removes the last coordinate from the sequence.
     *
     * @details After this call, the sequence size is decreased by 1.
     * Behavior is undefined if the sequence is empty.
     */
    void pop_back()
    {
        m_data.erase(std::prev(m_data.end(), m_stride), m_data.end());
    }

    /**
     * @brief Appends all coordinates from another sequence with type conversion.
     *
     * @tparam Z Template parameter indicating if Z coordinates are present
     * @tparam M Template parameter indicating if M values are present
     * @param other The source CoordinateSeq to copy from
     *
     * @details Iterates through all coordinates in the other sequence and appends them
     * with appropriate type conversions based on Z and M template parameters.
     */
    template <bool Z, bool M>
    void append(const CoordinateSeq &other)
    {
        const size_t nElems = other.size();
        m_data.reserve(m_data.size() + nElems * m_stride);
        for (size_t i = 0; i < nElems; ++i)
        {
            push_back(other.get<Z, M>(i));
        }
    }

    /**
     * @brief Appends all coordinates from a vector with type conversion.
     *
     * @tparam Z Template parameter indicating if Z coordinates are present in the source
     * @tparam M Template parameter indicating if M values are present in the source
     * @param vect Vector of coordinates to append
     *
     * @details Iterates through all coordinates in the vector and appends them
     * with appropriate type conversions based on Z and M template parameters.
     */
    template <bool Z, bool M>
    void append(const std::vector<detail::coo_impl<Z, M>> &vect)
    {
        m_data.reserve(m_data.size() + vect.size() * m_stride);
        for (const auto &coo : vect)
        {
            push_back(coo);
        }
    }


    /**
     * @brief Retrieves the coordinate at the specified index.
     *
     * @tparam Z Template parameter indicating if Z coordinates are present
     * @tparam M Template parameter indicating if M values are present
     * @param c Reference to the coordinate object to fill with retrieved values
     * @param i The index of the coordinate to retrieve
     *
     * @details Copies the coordinate at position i into the provided coordinate object.
     */
    template <bool Z, bool M>
    void get(detail::coo_impl<Z, M> &c, size_t i) const
    {
        get_at(c, get_iterator(i));
    }

    /**
     * @brief Retrieves a coordinate at the specified position from the end.
     *
     * @tparam Z Template parameter indicating if Z coordinates are present
     * @tparam M Template parameter indicating if M values are present
     * @param c Reference to the coordinate object to fill with retrieved values
     * @param i Index from the end (0 = last coordinate, 1 = second-to-last, etc.)
     *
     * @details Copies the coordinate at position (size - i - 1) into the provided coordinate object.
     */
    template <bool Z, bool M>
    void get_rev(detail::coo_impl<Z, M> &c, size_t i) const
    {
        get_at(c, std::prev(m_data.cend(), (i + 1) * m_stride));
    }

    /**
     * @brief Retrieves the first coordinate in the sequence.
     *
     * @tparam Z Template parameter indicating if Z coordinates are present
     * @tparam M Template parameter indicating if M values are present
     * @param c Reference to the coordinate object to fill with retrieved values
     *
     * @details Copies the first coordinate in the sequence into the provided coordinate object.
     * Behavior is undefined if the sequence is empty.
     */
    template <bool Z, bool M>
    void getFront(detail::coo_impl<Z, M> &c) const
    {
        get_at(c, m_data.cbegin());
    }

    /**
     * @brief Retrieves the last coordinate in the sequence.
     *
     * @tparam Z Template parameter indicating if Z coordinates are present
     * @tparam M Template parameter indicating if M values are present
     * @param c Reference to the coordinate object to fill with retrieved values
     *
     * @details Copies the last coordinate in the sequence into the provided coordinate object.
     * Behavior is undefined if the sequence is empty.
     */
    template <bool Z, bool M>
    void getBack(detail::coo_impl<Z, M> &c) const
    {
        get_at(c, std::prev(m_data.cend(), m_stride));
    }

    /**
     * @brief Retrieves a copy of the coordinate at the specified index.
     *
     * @tparam Z Template parameter indicating if Z coordinates are present
     * @tparam M Template parameter indicating if M values are present
     * @param i The index of the coordinate to retrieve
     * @return A copy of the coordinate at position i
     *
     * @details Returns the coordinate by value rather than by reference.
     */
    template <bool Z, bool M>
    const detail::coo_impl<Z, M> get(size_t i) const
    {
        return get_at<Z, M>(get_iterator(i));
    }

    /**
     * @brief Retrieves the XY coordinates at the specified index (Z and M are ignored).
     *
     * @param i The index of the coordinate to retrieve
     * @return A coordinate object containing only X and Y values
     *
     * @details Returns a 2D coordinate regardless of whether Z or M values are stored.
     */
    const auto get_xy(size_t i) const
    {
        return get_at<false, false>(get_iterator(i));
    }

    /**
     * @brief Sets the coordinate at the specified index.
     *
     * @tparam Z Template parameter indicating if Z coordinates are present
     * @tparam M Template parameter indicating if M values are present
     * @param c The coordinate to set
     * @param i The index at which to set the coordinate
     *
     * @details Replaces the coordinate at position i with the provided coordinate.
     * Behavior is undefined if index i is out of bounds.
     */
    template<bool Z, bool M>
    void set(const detail::coo_impl<Z, M> &c, size_t i)
    {
        set_at(c, get_iterator(i));
    }

    /**
     * @brief Sets the first coordinate in the sequence.
     *
     * @tparam Z Template parameter indicating if Z coordinates are present
     * @tparam M Template parameter indicating if M values are present
     * @param c The coordinate to set as the first element
     *
     * @details Replaces the first coordinate in the sequence.
     * Behavior is undefined if the sequence is empty.
     */
    template<bool Z, bool M>
    void setFront(const detail::coo_impl<Z, M> &c)
    {
        set_at(c, m_data.begin());
    }

    /**
     * @brief Sets the last coordinate in the sequence.
     *
     * @tparam Z Template parameter indicating if Z coordinates are present
     * @tparam M Template parameter indicating if M values are present
     * @param c The coordinate to set as the last element
     *
     * @details Replaces the last coordinate in the sequence.
     * Behavior is undefined if the sequence is empty.
     */
    template<bool Z, bool M>
    void setBack(const detail::coo_impl<Z, M> &c)
    {
        set_at(c, std::prev(m_data.end(), m_stride));
    }

    /**
     * @brief Iterates over consecutive pairs of coordinates (edges) and applies an operation.
     *
     * @tparam Z Template parameter indicating if Z coordinates are present
     * @tparam M Template parameter indicating if M values are present
     * @tparam _Op Type of the operation callable
     * @param op A callable that takes two consecutive coordinates as parameters
     *
     * @details Iterates through the sequence and for each edge (pair of consecutive coordinates),
     * calls the provided operation. The sequence must contain at least 2 coordinates.
     * Does nothing if the sequence is empty.
     */
    template <bool Z, bool M, typename _Op>
    void for_each_edge(_Op op) const
    {
        if (m_data.empty()) return;

        const_iterator it = m_data.cbegin();
        auto A = get_at<Z, M>(it);
        while ((it += m_stride) != m_data.cend())
        {
            auto B = get_at<Z, M>(it);
            op(A, B);
            A = B;
        }
    }

    /**
     * @brief Determines if the coordinate sequence is closed (first and last coordinates are equal).
     *
     * @return true if the sequence is closed, false otherwise
     *
     * @details A closed sequence has the same spatial position (X, Y) for the first and last
     * coordinates (Z coordinates are not considered). Returns false if the sequence is empty.
     */
    bool isClosed() const
    {
        if (m_data.empty()) return false;
        if (m_hasZ)
        {
            return gce::equalsEpsilon(front<true, false>().pos, back<true, false>().pos);
        }
        return gce::equalsEpsilon(front<false, false>().pos, back<false, false>().pos);
    }

    /**
     * @brief Determines if the coordinate sequence is a valid ring.
     *
     * @return true if the sequence is a ring (at least 4 coordinates and closed), false otherwise
     *
     * @details A valid ring must contain at least 4 coordinates and be closed (first and last
     * coordinates must have the same spatial position).
     */
    bool isRing() const
    {
        return (has_at_least_points(4) && isClosed());
    }

    /**
     * @brief Ensures the coordinate sequence forms a valid ring by closing it if necessary.
     *
     * @return true if the sequence was successfully ensured as a ring, false otherwise
     *
     * @details If the sequence is not already a ring, this method attempts to make it one.
     * This typically involves appending the first coordinate to the end if needed.
     */
    bool EnsureRing();


    /**
     * @brief Calculates the total length of the coordinate sequence.
     *
     * @tparam Z Template parameter indicating if Z coordinates are present
     * @return The total length of the path formed by connecting consecutive coordinates
     *
     * @details Sums the distances between consecutive coordinate pairs. If Z is true, uses
     * 3D distance calculations; otherwise uses 2D distance (X, Y only).
     */
    template<bool Z>
    double length() const
    {
        double sum = 0.0;
        for_each_edge<Z, false>([&sum](auto &a, auto &b){sum += glm::distance(a.pos, b.pos); });
        return sum;
    }

    /**
     * @brief Calculates the 2D (horizontal) length of the coordinate sequence.
     *
     * @return The total 2D length considering only X and Y coordinates
     *
     * @details Sums the 2D distances between consecutive coordinate pairs, ignoring Z values.
     */
    double length2D() const
    {
        return length<false>();
    }

    /**
     * @brief Calculates the 3D length of the coordinate sequence.
     *
     * @return The total 3D length if Z coordinates are present, otherwise the 2D length
     *
     * @details If the sequence contains Z coordinates (hasZ() returns true), calculates the
     * full 3D distance between consecutive coordinate pairs. Otherwise, returns the 2D length.
     */
    double length3D() const
    {
        if (m_hasZ)
        {
            return length<true>();
        }
        return length<false>();
    }

    /**
     * @brief Retrieves the first coordinate as a const reference.
     *
     * @tparam Z Template parameter indicating if Z coordinates are present
     * @tparam M Template parameter indicating if M values are present
     * @return A const coordinate object representing the first coordinate
     *
     * @details Returns the first coordinate by value. Behavior is undefined if the sequence is empty.
     */
    template <bool Z, bool M>
    const detail::coo_impl<Z, M> front() const
    {
        return get_at<Z, M>(m_data.cbegin());
    }

    /**
     * @brief Retrieves the last coordinate as a const reference.
     *
     * @tparam Z Template parameter indicating if Z coordinates are present
     * @tparam M Template parameter indicating if M values are present
     * @return A const coordinate object representing the last coordinate
     *
     * @details Returns the last coordinate by value. Behavior is undefined if the sequence is empty.
     */
    template <bool Z, bool M>
    const detail::coo_impl<Z, M> back() const
    {
        return  get_at<Z, M>(std::prev(m_data.cend(), m_stride));
    }

    /**
     * @brief Equality comparison operator.
     *
     * @param other The other CoordinateSeq to compare with
     * @return true if both sequences have the same format and coordinate data, false otherwise
     *
     * @details Two coordinate sequences are equal if they have the same coordinate format
     * and contain identical coordinate values.
     */
    bool operator == (const CoordinateSeq &other) const
    {
        return m_format == other.m_format && m_data == other.m_data;
    }

    /**
     * @brief Converts this coordinate sequence to a GEOS coordinate sequence.
     *
     * @return A GEOSCoordSeq object containing a copy of the coordinates
     *
     * @details Creates a new GEOS coordinate sequence from this sequence's data.
     * The returned object must be freed by the caller using the appropriate GEOS function.
     * @see GEOSCoordSeq_destroy
     */
    GEOSCoordSeq toGEOSCoordSeq() const
    {
        return GEOSCoordSeq_copyFromBuffer(m_data.data(), (unsigned int)size(), hasZ(), hasM());
    }

    /**
     * @brief Populates this coordinate sequence from a GEOS coordinate sequence.
     *
     * @param geosSeq A GEOS coordinate sequence to copy from
     * @return true if the conversion was successful, false otherwise
     *
     * @details Copies all coordinates from the given GEOS sequence into this sequence,
     * preserving Z and M dimensions according to the GEOS sequence's format.
     */
    bool fromGEOSCoordSeq(const GEOSCoordSequence *geosSeq)
    {
        unsigned int len;
        GEOSCoordSeq_getSize(geosSeq, &len);
        m_data.resize(len * m_stride);
        return GEOSCoordSeq_copyToBuffer(geosSeq, m_data.data(), hasZ(), hasM()) == 1;
    }

    /**
     * @brief Checks if the coordinate sequence contains Z coordinates.
     *
     * @return true if Z coordinates are present, false otherwise
     *
     * @details Returns true if the coordinate format includes elevation/Z values.
     */
    bool hasZ() const
    {
        return m_hasZ;
    }

    /**
     * @brief Checks if the coordinate sequence contains M (measure) values.
     *
     * @return true if M values are present, false otherwise
     *
     * @details Returns true if the coordinate format includes measure values.
     */
    bool hasM() const
    {
        return m_offsetM != 0;
    }

    /**
     * @brief Returns the stride (number of doubles per coordinate) in the internal storage.
     *
     * @return The stride value: 2 for 2D, 3 for 3D or 2D+M, 4 for 3D+M
     *
     * @details The stride determines the spacing between coordinates in the internal
     * flat vector storage. Use this for custom low-level access to the coordinate data.
     */
    uint8_t stride() const
    {
        return m_stride;
    }

    /**
     * @brief Applies a read-write filter to the coordinate sequence.
     *
     * @param filter The read-write filter to apply
     *
     * @details Applies a filter operation that can both read and modify coordinates in the sequence.
     * The filter is called for each coordinate, allowing custom transformations or validations.
     * @see filter_rw
     */
    void apply_filter_rw(filter_rw &filter);

    /**
     * @brief Applies a read-only filter to the coordinate sequence.
     *
     * @param filter The read-only filter to apply
     *
     * @details Applies a filter operation that can read but not modify coordinates in the sequence.
     * The filter is called for each coordinate, allowing custom analysis or processing.
     * @see filter_ro
     */
    void apply_filter_ro(filter_ro &filter) const;

    /**
     * @brief Calculates the winding number for a 2D position with respect to this coordinate sequence.
     *
     * @param pos The 2D position to test
     * @return The winding number (typically 0 if outside, non-zero if inside)
     *
     * @details Uses the ray-casting algorithm to determine the winding number, which indicates
     * whether a point is inside or outside a closed polygon, and if inside, how many times it is wrapped.
     */
    int windingNumber(const glm::dvec2 &pos) const;

    /**
     * @brief Calculates the signed area enclosed by the coordinate sequence.
     *
     * @return The signed area (positive for counter-clockwise, negative for clockwise)
     *
     * @details Uses the shoelace formula to compute the area. The sign indicates the orientation
     * of the polygon formed by the coordinate sequence.
     */
    double signedArea() const;

    /**
     * @brief Reverses the order of coordinates and ensures Right-Hand Rule orientation.
     *
     * @details If the coordinate sequence has a negative signed area (clockwise), reverses
     * the order of coordinates to make it counter-clockwise (positive signed area).
     * This ensures the sequence follows the Right-Hand Rule convention for rings.
     */
    void ForceRHR()
    {
        if (this->signedArea() < 0)
        {
            this->reverse();
        }
    }

    /**
     * @brief Reverses the order of coordinates in the sequence.
     *
     * @return A reference to this CoordinateSeq for method chaining
     *
     * @details The first coordinate becomes the last, the second becomes second-to-last, etc.
     * Useful for changing the orientation of a polygon or path.
     */
    CoordinateSeq &reverse();


    /**
     * @brief Checks if the sequence contains at least the specified number of coordinates.
     *
     * @param n The minimum number of coordinates to check for
     * @return true if size() >= n, false otherwise
     *
     * @details Efficient way to check if the sequence has enough coordinates without
     * explicitly calculating the size.
     */
    bool has_at_least_points(size_t n) const
    {
        return m_data.size() >= n * m_stride;
    }
private:
    // Private member functions for internal coordinate access

    /**
     * @brief Gets an iterator to the coordinate at the specified index.
     *
     * @param i The coordinate index
     * @return An iterator to the first double of the coordinate at index i
     */
    iterator get_iterator(size_t i)
    {
        return m_data.begin() + i * m_stride;
    }

    const_iterator get_iterator(size_t i) const
    {
        return m_data.cbegin() + i * m_stride;
    }

    /**
     * @brief Retrieves a coordinate from the internal storage into a coordinate object.
     *
     * @tparam Z Template parameter indicating if Z coordinates are present
     * @tparam M Template parameter indicating if M values are present
     * @param c Reference to the coordinate object to fill
     * @param i Iterator to the first double of the coordinate in internal storage
     */
    template <bool Z, bool M>
    void get_at(detail::coo_impl<Z, M> &c, const_iterator i) const
    {
        c.pos.x = get_x(i);
        c.pos.y = get_y(i);
        if constexpr (Z) c.pos.z = get_z(i);
        if constexpr (M) c.m = get_m(i);
    }

    /**
     * @brief Retrieves a coordinate from the internal storage.
     *
     * @tparam Z Template parameter indicating if Z coordinates are present
     * @tparam M Template parameter indicating if M values are present
     * @param i Iterator to the first double of the coordinate in internal storage
     * @return A coordinate object containing the retrieved values
     */
    template <bool Z, bool M>
    const detail::coo_impl<Z, M> get_at(const_iterator i) const
    {
        detail::coo_impl<Z, M> c;
        get_at(c, i);
        return c;
    }

    /**
     * @brief Stores a coordinate into the internal storage.
     *
     * @tparam Z Template parameter indicating if Z coordinates are present
     * @tparam M Template parameter indicating if M values are present
     * @param c The coordinate object to store
     * @param i Iterator to the first double of the coordinate position in internal storage
     */
    template <bool Z, bool M>
    void set_at(const detail::coo_impl<Z, M> &c, iterator i)
    {
        set_x(c.pos.x, i);
        set_y(c.pos.y, i);
        if constexpr (Z) set_z(c.pos.z, i);
        if constexpr (M) set_m(c.m, i);
    }

    /**
     * @brief Retrieves the X coordinate value at an iterator position.
     *
     * @param i Iterator to the coordinate's position in internal storage
     * @return The X coordinate value
     */
    double get_x(const_iterator i) const
    {
        return *i;
    }

    /**
     * @brief Retrieves the Y coordinate value at an iterator position.
     *
     * @param i Iterator to the coordinate's position in internal storage
     * @return The Y coordinate value
     */
    double get_y(const_iterator i) const
    {
        return *(i + 1);
    }

    /**
     * @brief Retrieves the Z coordinate value at an iterator position.
     *
     * @param i Iterator to the coordinate's position in internal storage
     * @return The Z coordinate value, or 0.0 if Z is not present in the format
     */
    double get_z(const_iterator i) const
    {
        return m_hasZ ? *(i + 2) : 0.0;
    }

    /**
     * @brief Retrieves the M (measure) value at an iterator position.
     *
     * @param i Iterator to the coordinate's position in internal storage
     * @return The M value, or 0.0 if M is not present in the format
     */
    double get_m(const_iterator i) const
    {
        return m_offsetM ? *(i + m_offsetM) : 0.0;
    }

    /**
     * @brief Sets the X coordinate value at an iterator position.
     *
     * @param x The X value to set
     * @param i Iterator to the coordinate's position in internal storage
     */
    void set_x(double x, iterator i)
    {
        *i = x;
    }

    /**
     * @brief Sets the Y coordinate value at an iterator position.
     *
     * @param y The Y value to set
     * @param i Iterator to the coordinate's position in internal storage
     */
    void set_y(double y, iterator i)
    {
        *(i + 1) = y;
    }

    /**
     * @brief Sets the Z coordinate value at an iterator position.
     *
     * @param z The Z value to set
     * @param i Iterator to the coordinate's position in internal storage
     *
     * @details Only updates the Z value if Z coordinates are present in the format.
     */
    void set_z(double z, iterator i)
    {
        if (m_hasZ) *(i + 2) = z;
    }

    /**
     * @brief Sets the M (measure) value at an iterator position.
     *
     * @param m The M value to set
     * @param i Iterator to the coordinate's position in internal storage
     *
     * @details Only updates the M value if M values are present in the format.
     */
    void set_m(double m, iterator i)
    {
        if (m_offsetM) *(i + m_offsetM) = m;
    }

    /**
     * @brief Checks if a coordinate at an iterator position equals a given coordinate.
     *
     * @tparam Z Template parameter indicating if Z coordinates are present in the comparison
     * @tparam M Template parameter indicating if M values are present in the comparison
     * @param c The coordinate to compare with
     * @param i Iterator to the coordinate's position in internal storage
     * @return true if spatial coordinates (X, Y, optionally Z) are equal, false otherwise
     *
     * @details Comparison considers X, Y, and optionally Z based on template parameters.
     * M values are ignored in the comparison. Returns false if only spatial coordinates differ.
     */
    template <bool Z, bool M>
    bool equal_if_inserted(const detail::coo_impl<Z, M> &c, const_iterator i) const
    {
        if (get_x(i) != c.pos.x) return false;
        if (get_y(i) != c.pos.y) return false;
        if (hasZ())
        {
            if constexpr (Z)
            {
                if (get_z(i) != c.pos.z) return false;
            }
            else
            {
                if (get_z(i) != 0.0) return false;
            }
        }
        // ignore M - consder identical spatial coordinates
        return true;
    }

    /**
     * @brief Member variables for internal coordinate storage
     *
     * @var m_format The coordinate type format (determines presence of Z and/or M)
     * @var m_hasZ Flag indicating if Z coordinates are present
     * @var m_offsetM Offset to M value in the stride; 0 if M is not present
     * @var m_stride Number of doubles per coordinate (2, 3, or 4)
     * @var m_data Flat vector storage for all coordinate values
     */
    CoordinateType m_format;
    bool m_hasZ;
    std::uint8_t m_offsetM;
    std::uint8_t m_stride;
    std::vector<double> m_data;
};
} // namespace geom
