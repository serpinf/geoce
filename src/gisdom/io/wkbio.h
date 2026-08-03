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
#include "geom/Coordinate.h"
#include "geom/Geometry.h"

namespace geom
{

/**
 * @enum wkbGeometryType
 * @brief Enumeration of Well-Known Binary (WKB) geometry types.
 *
 * This enumeration defines the standard geometry type codes used in the WKB format
 * as defined by OGC (Open Geospatial Consortium).
 */
enum wkbGeometryType
{
    wkbPoint = 1,               /*!< WKB type code for Point geometry */
    wkbLineString = 2,          /*!< WKB type code for LineString geometry */
    wkbPolygon = 3,             /*!< WKB type code for Polygon geometry */
    wkbMultiPoint = 4,          /*!< WKB type code for MultiPoint geometry */
    wkbMultiLineString = 5,     /*!< WKB type code for MultiLineString geometry */
    wkbMultiPolygon = 6,        /*!< WKB type code for MultiPolygon geometry */
    wkbGeometryCollection = 7,  /*!< WKB type code for GeometryCollection */
    wkbBBoxOnly = 99            /*!< WKB type code for bounding box only */
};

/**
 * @enum wkbGeometryFlag
 * @brief Enumeration of WKB geometry flags for extended dimensions and metadata.
 *
 * These flags are used in Extended Well-Known Binary (EWKB) format to indicate
 * the presence of Z, M, and SRID information.
 */
enum wkbGeometryFlag
{
    wkbZ = 0x80000000,      /*!< Flag indicating presence of Z coordinate */
    wkbM = 0x40000000,      /*!< Flag indicating presence of M coordinate */
    wkbSRID = 0x20000000    /*!< Flag indicating presence of SRID (Spatial Reference System ID) */
};

/**
 * @enum wkbByteOrder
 * @brief Enumeration of byte order formats for WKB data.
 */
enum wkbByteOrder
{
    wkbXDR = 0,    /*!< Big Endian byte order (XDR) */
    wkbNDR = 1     /*!< Little Endian byte order (NDR, Network Data Representation) */
};

/**
 * @brief Converts an internal geometry type to WKB geometry type.
 *
 * Maps from the internal gceGeometryType enumeration to the standard WKB format type codes.
 *
 * @param gtype The internal geometry type to convert.
 * @return The corresponding WKB geometry type.
 *
 * @note Returns wkbBBoxOnly if the input type is not recognized.
 */
constexpr inline wkbGeometryType to_wkbType(gceGeometryType gtype)
{
    switch (gtype)
    {
    case gceGeometryType::Point:
        return wkbPoint;
    case gceGeometryType::LineString:
        return wkbLineString;
    case gceGeometryType::Polygon:
        return wkbLineString;
    case gceGeometryType::MultiPoint:
        return wkbMultiPoint;
    case gceGeometryType::MultiLineString:
        return wkbMultiLineString;
    case gceGeometryType::MultiPolygon:
        return wkbMultiPolygon;
    case gceGeometryType::GeometryCollection:
        return wkbGeometryCollection;
    }
    // should never be here
    return wkbBBoxOnly;
}

/**
 * @class WKBreader
 * @brief Reads and parses Well-Known Binary (WKB) format data into geometry objects.
 *
 * WKBreader is a non-copyable class that deserializes WKB-formatted byte buffers
 * into internal geometry representations. It supports all standard OGC geometry types
 * and Extended WKB (EWKB) formats with Z, M, and SRID information.
 *
 * @note This class is non-copyable (inherits from boost::noncopyable).
 * @note WKB buffer size validation should be implemented to prevent buffer overruns.
 */
class WKBreader final : boost::noncopyable
{
public:

    /**
     * @brief Reads and parses a WKB-formatted buffer into a geometry object.
     *
     * Deserializes the WKB data and creates an appropriate geometry object based
     * on the geometry type and coordinate dimensions specified in the WKB header.
     *
     * @param cooType The coordinate type (2D, 3D, etc.) for the resulting geometry.
     * @param wkb Pointer to the WKB data buffer.
     * @param size The size of the WKB data buffer in bytes.
     * @return A unique_ptr to the created Geometry object.
     *
     * @note The caller takes ownership of the returned geometry.
     */
    std::unique_ptr<Geometry> readGeometry(CoordinateType cooType, const uint8_t *wkb, size_t size);

private:
    /**
     * @brief Reads a Point geometry from the WKB buffer.
     *
     * @param geom Reference to the Point object to populate.
     * @param wkbType The WKB type code indicating coordinate format (XY, XYZ, XYM, XYZM).
     */
    void read(Point &geom, uint32_t wkbType);

    /**
     * @brief Reads a LineString geometry from the WKB buffer.
     *
     * @param geom Reference to the LineString object to populate.
     * @param wkbType The WKB type code indicating coordinate format (XY, XYZ, XYM, XYZM).
     */
    void read(LineString &geom, uint32_t wkbType);

    /**
     * @brief Reads a Polygon geometry from the WKB buffer.
     *
     * @param geom Reference to the Polygon object to populate.
     * @param wkbType The WKB type code indicating coordinate format (XY, XYZ, XYM, XYZM).
     */
    void read(Polygon &geom, uint32_t wkbType);

    /**
     * @brief Reads a multi-geometry (collection) from the WKB buffer.
     *
     * Template method for reading multi-geometry types such as MultiPoint,
     * MultiLineString, and MultiPolygon.
     *
     * @tparam Geom The multi-geometry type to read.
     * @param geom Reference to the MGeometry object to populate.
     * @param wkbType The WKB type code indicating coordinate format (XY, XYZ, XYM, XYZM).
     */
    template <typename Geom>
    void read(MGeometry &geom, uint32_t wkbType);

    /**
     * @brief Reads a single coordinate from the WKB buffer.
     *
     * Parses coordinate data in the specified format (XY, XYZ, XYM, or XYZM)
     * from the current buffer position.
     *
     * @param c Reference to the Coordinate object to populate.
     * @param wkbType The WKB format flags indicating which dimensions are present
     *                (see wkbGeometryFlag for Z, M flags).
     */
    void read(Coordinate &c, uint32_t wkbType);

    /**
     * @brief Reads a sequence of coordinates from the WKB buffer.
     *
     * Parses multiple coordinate points in the specified format from the buffer
     * and stores them in the coordinate sequence.
     *
     * @param seq Reference to the CoordinateSeq object to populate.
     * @param wkbType The WKB format flags indicating which dimensions are present
     *                (see wkbGeometryFlag for Z, M flags).
     * @param len The number of coordinates to read.
     */
    void read(CoordinateSeq &seq, uint32_t wkbType, size_t len);

    /** @brief Current read position in the WKB buffer. */
    const uint8_t *m_ptr = nullptr;
};

/**
 * @class WKBwriter
 * @brief Writes geometry objects to Well-Known Binary (WKB) format data.
 *
 * WKBwriter is a non-copyable class that serializes internal geometry representations
 * into Extended Well-Known Binary (EWKB) format. It supports all standard OGC geometry types
 * and includes Z, M, and SRID information in the output.
 *
 * @note This class is non-copyable (inherits from boost::noncopyable).
 * @note WKB buffer size validation should be implemented to prevent buffer overruns.
 */
class WKBwriter : boost::noncopyable
{
public:
    /**
     * @brief Constructs a WKBwriter with default SRID (4326 - WGS84).
     *
     * Initializes the writer with the WGS84 spatial reference system ID.
     */
    WKBwriter() : m_SRID(4326)
    {}

    /**
     * @brief Calculates the required buffer size for a WKB representation.
     *
     * Computes the exact number of bytes needed to write the given geometry
     * in EWKB format, including all header information, coordinates, and metadata.
     *
     * @param geom Pointer to the geometry object to measure.
     * @return The required buffer size in bytes.
     */
    static size_t sizeGeometry(const Geometry *geom);

    /**
     * @brief Writes a geometry object to EWKB format in a user-provided buffer.
     *
     * Serializes the geometry into Extended Well-Known Binary format with
     * SRID, Z, and M information as applicable.
     *
     * @param buf Pointer to the user-provided output buffer. Must be large enough
     *            to hold the entire WKB representation (use sizeGeometry to determine size).
     * @param geom Reference to the geometry object to serialize.
     * @return The number of bytes written to the buffer.
     *
     * @note The buffer must be pre-allocated and large enough. Use sizeGeometry() first.
     */
    size_t write(char *buf, const Geometry &geom);

private:
    /**
     * @brief Writes a generic geometry to the WKB buffer.
     *
     * @param geom Reference to the geometry to write.
     */
    void write(const Geometry &geom);

    /**
     * @brief Writes a Point geometry to the WKB buffer.
     *
     * @param geom Reference to the Point to write.
     */
    void write(const Point &geom);

    /**
     * @brief Writes a LineString geometry to the WKB buffer.
     *
     * @param geom Reference to the LineString to write.
     */
    void write(const LineString &geom);

    /**
     * @brief Writes a Polygon geometry to the WKB buffer.
     *
     * @param g Reference to the Polygon to write.
     */
    void write(const Polygon &g);

    /**
     * @brief Writes a multi-geometry (collection) to the WKB buffer.
     *
     * @param g Reference to the MGeometry to write.
     * @param collectionType The WKB type for the collection (e.g., wkbMultiPoint).
     */
    void write(const MGeometry &g, wkbGeometryType collectionType);

    /**
     * @brief Writes a single coordinate to the WKB buffer.
     *
     * @param c Reference to the Coordinate to write.
     */
    void write(const Coordinate &c);

    /**
     * @brief Writes a sequence of coordinates to the WKB buffer.
     *
     * @param c Reference to the CoordinateSeq to write.
     */
    void write(const CoordinateSeq &c);

    /** @brief Current write position in the output buffer. */
    char *m_ptr = nullptr;

    /** @brief WKB flags for Z, M, and SRID presence. */
    uint32_t m_wkbFlags = 0;

    /** @brief Spatial Reference System ID (default is 4326 for WGS84). */
    const uint32_t m_SRID;
};
}
