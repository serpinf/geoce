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

#include "gcprec.h"
#include "entity.h"
#include "entitypck.h"
#include "fmt/std.h"
#include "fmt/ranges.h"

gceEntityVar::gceEntityVar(const gceEntityPacked &other) : gceEntityVar(other.get_schema())
{
    for (auto &col : schema->getColumns())
    {
        assign(other, col.getIndex());
    }
}

gceEntityVar::gceEntityVar(const gceEntityPacked &other, std::unique_ptr<geom::Geometry> &&pgeom) : gceEntityVar(other.get_schema())
{
    auto geometryIndex = this->schema->getGeometryIndex();
    for (auto &col : schema->getColumns())
    {
        if (!geometryIndex || col.getIndex() != *geometryIndex)
        {
            assign(other, col.getIndex());
        }
    }
    if (geometryIndex)
    {
        m_data[*geometryIndex] = std::move(pgeom);
    }
}

gceEntityVar::gceEntityVar(const gceEntityPacked &other, const geom::GEOSGeom_scoped_t &pgeom)
    : gceEntityVar(other, geom::Geometry::Create(*other.getCoordinateType(), pgeom))
{}

void gceEntityVar::assign(const gceEntityPacked &other, unsigned index)
{
    m_data[index] = other.getValue(index);
}

geom::Geometry *gceEntityVar::getGeometry()
{
    if (auto index = schema->getGeometryIndex())
    {
        return std::get<std::unique_ptr<geom::Geometry>>(m_data[*index]).get();
    }
    return nullptr;
}

const geom::Geometry *gceEntityVar::getGeometry() const
{
    if (auto index = schema->getGeometryIndex())
    {
        return std::get<std::unique_ptr<geom::Geometry>>(m_data[*index]).get();
    }
    return nullptr;
}

bool gceEntityVar::setGeometry(std::unique_ptr<geom::Geometry> &&pgeom)
{
    if (auto index = schema->getGeometryIndex())
    {
        m_data[*index] = std::move(pgeom);
        return true;
    }
    return false;
}

template <> struct fmt::formatter<std::unique_ptr<geom::Geometry>>
{
    template<typename FormatParseContext>
    constexpr static auto parse(FormatParseContext &ctx)
    {
        return ctx.end();
    }

    static auto format(const std::unique_ptr<geom::Geometry> &arg, format_context &ctx)
    {
        if (arg)
        {
            return fmt::format_to(ctx.out(), "{}", arg->toWKT());
        }
        return fmt::format_to(ctx.out(), "EMPTY GEOMETRY");
    }
};
template <> struct fmt::formatter<gceColumnValue>
{
    template<typename FormatParseContext>
    constexpr static auto parse(FormatParseContext &ctx)
    {
        return ctx.end();
    }

    constexpr static auto format(const gceColumnValue &value, fmt::format_context &ctx)
    {
        return std::visit([&ctx](const auto &v){ return fmt::format_to(ctx.out(), "{}", v); }, value);
    }
};
std::string gceEntityVar::to_string() const
{
    return fmt::format("{}", m_data);
}
