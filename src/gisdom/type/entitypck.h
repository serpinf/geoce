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
#include <vector>
#include "typeschema.hpp"
#include <boost/endian/conversion.hpp>
namespace gce
{
template <typename T> GCE_WITH_REQUIRES(std::is_arithmetic_v<T>) T read_be(const std::byte *src) noexcept
{
    static_assert(std::is_arithmetic_v<T>, "");
    return boost::endian::endian_load<T, sizeof(T), boost::endian::order::big>((const unsigned char *)src);
}

template <typename T> GCE_WITH_REQUIRES(std::is_arithmetic_v<T>) void write_be(std::byte *dest, const T src)
{
    static_assert(std::is_arithmetic_v<T>, "");
    boost::endian::endian_store<T, sizeof(T), boost::endian::order::big>((unsigned char *)dest, src);
}

template <class Visitor>
auto visit_column(Visitor &&obj, gceColumnType type)
{
    switch (type)
    {
    case gceColumnType::None:
        break;
    case gceColumnType::int16:
        return obj.template operator() < int16_t > ();
    case gceColumnType::int32:
        return obj.template operator() < int32_t > ();
    case gceColumnType::int64:
        return obj.template operator() < int64_t > ();
    case gceColumnType::float32:
        return obj.template operator() < float > ();
    case gceColumnType::float64:
        return obj.template operator() < double > ();
    case gceColumnType::string:
        return obj.template operator() < std::string > ();
    case gceColumnType::geometry:
        return obj.template operator() < std::unique_ptr<geom::Geometry> > ();
    }
    return obj.template operator() < std::monostate > ();
}
}

// 'e':uint8_t | 'p':uint8_t | bytes_per_index:uint8_t | columns number:uint8_t | offset:bytes_per_index[columns number + 1] | data:byte[]
using gceEntityBuffer = std::vector<std::byte>;

class gceEntityPacked final
{
public:
    explicit gceEntityPacked(const gceEntityVar &other);

    gceEntityPacked() = default;

    void assign(const gceTypeSchema *schema, gce::span<std::byte> *data);

    bool empty() const
    {
        return m_buffer.empty();
    }

    template<typename T> GCE_WITH_REQUIRES(is_column_arithmetic_v<T>)
        T get_arithmetic(uint8_t index) const
    {
        static_assert(is_column_arithmetic_v<T>, "");
        holds_type_check<T>(index);
        return get_arithmetic_unchecked<T>(index);
    }

    template<typename T> GCE_WITH_REQUIRES(is_column_arithmetic_v<T>)
        T get_arithmetic_unchecked(uint8_t index) const
    {
        static_assert(is_column_arithmetic_v<T>, "");
        auto r = get_data(index);
        return r.size() == sizeof(T) ? gce::read_be<T>(r.data()) : T{};
    }

    std::string get_string(uint8_t index) const
    {
        holds_type_check<std::string>(index);
        return get_string_unchecked(index);
    }

    std::string get_string_unchecked(uint8_t index) const
    {
        auto src = get_data(index);
        return {reinterpret_cast<const char *>(src.data()), src.size()};
    }

    /*std::unique_ptr<geom::Geometry> get_geometry(uint8_t index) const
    {
        if (isGeometry(getType(index)))
        {
            return get_geometry_unchecked(index);
        }
        return {};
    }*/

    std::unique_ptr<geom::Geometry> get_geometry() const
    {
        if (auto index = m_schema->getGeometryIndex())
        {
            return get_geometry_unchecked(*index);
        }
        return {};
    }

    std::unique_ptr<geom::Geometry> get_geometry_unchecked(uint8_t index) const;

    geom::GEOSGeom_scoped_t get_geometry_GEOS() const
    {
        if (auto index = m_schema->getGeometryIndex())
        {
            return get_geometry_GEOS(*index);
        }
        return {};
    }

    geom::GEOSGeom_scoped_t get_geometry_GEOS(uint8_t index) const;

    gce::span<const std::byte> get_data(uint8_t index) const;

    gceColumnValue getValue(uint8_t index) const;

    template<typename T>
    bool holds_type(uint8_t index) const noexcept
    {
        return getType(index) == gceColumnTypeIndex<T>();
    }

    template<typename T>
    void holds_type_check(uint8_t index) const
    {
        if (!holds_type<T>(index))
        {
            throw std::runtime_error("gceEntityPacked: invalid type access");
        }
    }

    std::string to_string() const;
    std::string pkey_string() const;

    gceEntityKey get_pkey() const;

    gceColumnType getType(uint8_t index) const
    {
        return m_schema->at(index).getType();
    }

    std::optional<geom::CoordinateType> getCoordinateType() const
    {
        if (auto geomCol = m_schema->getGeometryColumn())
        {
            return geomCol->getCoordinateType();
        }
        return {};
    }

    const gceTypeSchema *get_schema() const noexcept
    {
        return this->m_schema;
    }

    /*!
     * @brief compare two entities of the same schema, throws if empty or the schema is different
     * @param other entity
     * @return id comparison result
     */
    bool lessId(const gceEntityPacked &other) const
    {
        return get_pkey() < other.get_pkey();
    }

    /*!
     * @brief compares the fields in entities with the same schema, throws if empty or schema is different
     * @param other entity to compare with
     * @param index column index
     * @return true if values are equal
     */
    bool equals(const gceEntityPacked &other, uint8_t index) const;

    bool hasValue(uint8_t index) const;

private:
    const gceTypeSchema *m_schema = nullptr;
    gceEntityBuffer m_buffer;
};

struct gceEntityPackedRef
{
    gce::uuid id_table{};
    gceEntityPacked entity;

    //bool operator == (const gceEntityRef& other) const;

    bool operator < (const gceEntityPackedRef &other) const
    {
        if (this->id_table == other.id_table)
        {
            return entity.lessId(other.entity);
        }
        return this->id_table < other.id_table;
    }

    bool empty() const
    {
        return entity.empty();
    }

    void reset()
    {
        id_table = gce::uuid{};
        entity = {};
    }

    bool isTableRow() const
    {
        return !id_table.is_nil();
    }

    gceEntitySetKey get_key() const
    {
        return {id_table, entity.get_pkey()};
    }
};
