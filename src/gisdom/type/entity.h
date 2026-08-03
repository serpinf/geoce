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
#include "typeschema.hpp"

class gceEntityPacked;

class gceEntityVar final
{
public:
    gceEntityVar(const gceTypeSchema *_schema) : schema(_schema)
    {
        m_data.resize(schema->size());
    }

    explicit gceEntityVar(const gceEntityPacked &other);

    gceEntityVar(const gceEntityPacked &other, std::unique_ptr<geom::Geometry> &&pgeom);

    gceEntityVar(const gceEntityPacked &other, const geom::GEOSGeom_scoped_t &pgeom);

    gceEntityVar(gceEntityVar &&other) = default;

    void assign(const gceEntityPacked &other, unsigned index);

    geom::Geometry *getGeometry();
    const geom::Geometry *getGeometry() const;

    bool setGeometry(std::unique_ptr<geom::Geometry> &&pgeom);

    gceColumnValue &operator[](uint8_t index)
    {
        return m_data[index];
    }

    const gceColumnValue &operator[](uint8_t index) const
    {
        return m_data[index];
    }

    std::string to_string() const;

    const gceTypeSchema *const schema;
private:
    std::vector<gceColumnValue> m_data;
};
