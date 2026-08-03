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
#include <variant>
#include <boost/mp11/algorithm.hpp>
#include "geom/Geometry.h"
#include <optional>

class gceEntityVar;
class gceEntityPacked;

template <typename T>
class gceFlagSet
{
    T m_flags = 0;
public:
    constexpr gceFlagSet(const T flags) : m_flags(flags) {}

    constexpr void set(const T flags) noexcept
    {
        m_flags |= flags;
    }

    void clear(const T flags) noexcept
    {
        m_flags ^= flags;
    }

    constexpr bool all(const T flags) const noexcept
    {
        return (m_flags & flags) == flags;
    }

    bool any(const T flags) const noexcept
    {
        return (m_flags & flags) != 0;
    }

    bool none(const T flags) const noexcept
    {
        return (m_flags & flags) == 0;
    }
};

enum class gceColumnType : uint8_t
{
    None,
    int16,
    int32,
    int64,
    float32,
    float64,
    string,
    geometry,
};
inline bool isInteger(gceColumnType type)
{
    return type == gceColumnType::int16 || type == gceColumnType::int32 || type == gceColumnType::int64;
}
inline bool isFloat(gceColumnType type)
{
    return type == gceColumnType::float32 || type == gceColumnType::float64;
}
inline bool isString(gceColumnType type)
{
    return type == gceColumnType::string;
}
inline bool isGeometry(gceColumnType type)
{
    return type == gceColumnType::geometry;
}

template< class T>
constexpr bool is_column_arithmetic_v = gce::is_any<T, std::int16_t, std::int32_t, std::int64_t, float, double>::value;

using gceColumnValue = std::variant<std::monostate, int16_t, int32_t, int64_t, float, double, std::string, std::unique_ptr<geom::Geometry>>;

template<typename T>
inline constexpr gceColumnType gceColumnTypeIndex()
{
    return static_cast<gceColumnType>(boost::mp11::mp_find<gceColumnValue, T>::value);
}

//using gceEntityKey = std::variant<int16_t, int32_t, int64_t, std::string>;
using gceEntityKey = std::variant<int64_t>;

template <> struct fmt::formatter<gceEntityKey>
{
    template<typename FormatParseContext>
    constexpr static auto parse(FormatParseContext &ctx)
    {
        return ctx.end();
    }

    constexpr static auto format(const gceEntityKey &value, fmt::format_context &ctx)
    {
        return std::visit([&ctx](const auto &v){
            return fmt::format_to(ctx.out(), "{}", v); },
            value
            );
    }
};

struct gceEntitySetKey
{
    gce::uuid id_table;
    gceEntityKey id_entity;

    bool operator < (const gceEntitySetKey &other) const
    {
        if (id_table == other.id_table)
        {
            return id_entity < other.id_entity;
        }
        return id_table < other.id_table;
    }
};

class gceColumnSchema final
{
public:
    using index_type = uint8_t;

    using flags_type = uint32_t;
    enum ColumnFlags : flags_type
    {
        F_PKEY = 0x01, // TODO: move PKEY definition to typeschema as column names/ids list
        F_SERIAL = 0x02, // TODO: replace serial with IDENTITY as per standard
        F_PROPERTY = 0x04 // TODO: use bool value?
    };
    gceColumnSchema(const gceColumnSchema &) = delete;
    void operator = (const gceColumnSchema &) = delete;

    gceColumnSchema(gceColumnSchema &&) = default;

    gceColumnSchema(uint8_t index, const gceColumnType type, const std::string &name) : m_index(index), m_type(type), m_name(name), m_label(name) {}

    constexpr gceColumnType getType() const
    {
        return m_type;
    }

    const std::string &getName() const
    {
        return m_name;
    }

    const std::string &getLabel() const
    {
        return m_label;
    }


    gceColumnSchema &&geometryDimension(int dimension)
    {
        m_geometryDimension = dimension;
        return std::move(*this);
    }

    int getGeometryDimension() const
    {
        return m_geometryDimension;
    }

    gceColumnSchema &&coordinateType(geom::CoordinateType type)
    {
        m_coordinateType = type;
        return std::move(*this);
    }

    geom::CoordinateType getCoordinateType() const
    {
        return m_coordinateType;
    }

    gceColumnSchema &&PKEY()
    {
        m_flags.set(F_PKEY);
        return std::move(*this);
    }

    bool isPKEY() const
    {
        return m_flags.all(F_PKEY);
    }

    gceColumnSchema &&Serial()
    {
        m_flags.set(F_SERIAL);
        return std::move(*this);
    }

    bool isSerial() const
    {
        return m_flags.all(F_SERIAL);
    }

    gceColumnSchema &&Property()
    {
        m_flags.set(F_PROPERTY);
        return std::move(*this);
    }

    constexpr bool isProperty() const
    {
        return m_flags.all(F_PROPERTY);
    }

    bool hasFlags(flags_type flags) const
    {
        return m_flags.all(flags);
    }

    bool hasNoFlags(flags_type flags) const
    {
        return m_flags.none(flags);
    }

    template <typename T>
    gceColumnSchema &&Default(const T &val)
    {
        if (m_type != gceColumnTypeIndex<T>())
        {
            throw std::logic_error("Wrong default value type");
        }
        m_defaultVal = val;
        return std::move(*this);
    }

    const gceColumnValue &getDefault() const
    {
        return m_defaultVal;
    }

    const std::string &getControlName() const
    {
        return m_ctrlName;
    }

    uint8_t getIndex() const
    {
        return m_index;
    }

    const char *getTypeIcon() const;

private:
    uint8_t m_index;
    gceColumnType m_type;
    geom::CoordinateType m_coordinateType = geom::CoordinateType::XY;
    int8_t m_geometryDimension = -1; // default no dimension
    gceFlagSet<flags_type> m_flags = 0;
    std::string m_name;
    std::string m_label;
    std::string m_category;
    std::string m_ctrlName;
    std::variant<std::monostate, int64_t, double> m_minVal;
    std::variant<std::monostate, int64_t, double> m_maxVal;
    std::variant<std::monostate, int64_t, double> m_stepVal;
    gceColumnValue m_defaultVal;
};
class gceTypeSchema final
{
public:
    template <typename... Cols>
    gceTypeSchema(const gce::uuid &id_schema, const std::string &svgIcon, const std::string &name, Cols&&... cols)
        : id_schema(id_schema), m_svgIcon(svgIcon), m_name(name)
    {
        m_columns.reserve(sizeof...(cols));
        (m_columns.emplace_back(std::forward<Cols>(cols)), ...);

        parseColumns();
    }

    void parseColumns();

    gceTypeSchema(const gceTypeSchema &) = delete;
    void operator = (const gceTypeSchema &) = delete;

    const std::string &getIcon() const
    {
        return m_svgIcon;
    }

    const std::string &getName() const
    {
        return m_name;
    }

    const gce::uuid &getId() const
    {
        return id_schema;
    }

    const std::vector<gceColumnSchema> &getColumns() const
    {
        return m_columns;
    }

    int findIndex(const std::string &colName) const
    {
        for (const auto &col : m_columns)
        {
            if (col.getName() == colName)
            {
                return col.getIndex();
            }
        }
        return -1;
    }

    size_t size() const
    {
        return m_columns.size();
    }

    const gceColumnSchema &at(size_t n) const
    {
        return m_columns.at(n);
    }
    gceEntityVar createDefaultEntityVar() const;
    gceEntityPacked createDefaultEntityPacked() const;

    uint8_t getKeyIndex() const
    {
        return m_keyIndex;
    }

    const gceColumnSchema &getKeyColumn() const
    {
        return m_columns[m_keyIndex];
    }

    /*!
     * @brief get type info for 1st geometry entry
     * @return geometry type entry info
     */
    const gceColumnSchema *getGeometryColumn() const
    {
        if (m_geometryIndex)
            return &m_columns[m_geometryIndex.value()];
        return nullptr;
    }

    std::optional<gceColumnSchema::index_type> getGeometryIndex() const
    {
        return m_geometryIndex;
    }

private:
    gceColumnSchema::index_type m_keyIndex = 0;
    std::optional<gceColumnSchema::index_type> m_geometryIndex;
    gce::uuid id_schema{};
    std::string m_svgIcon;
    std::string m_name;
    std::vector<gceColumnSchema> m_columns;
};

