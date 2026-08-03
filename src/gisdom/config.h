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

class gceTypeSchema;
class gceModelSchema;
class gceDatasourceSchema;

class gceConfig final
{
public:
    gceConfig();
    ~gceConfig();
    bool load(const std::string &path);
    const gceTypeSchema *findSchema(const gce::uuid &id) const;
    const gceTypeSchema *findSchema(const std::string &name) const;
    std::string getSchemaName(const gce::uuid &id) const;
    auto &getSchemas() const
    {
        return m_schemas;
    }

    const gceModelSchema *findModelSchema(const std::string &name) const;
    auto &getModelSchemas() const
    {
        return m_modelSchemas;
    }

    const gceDatasourceSchema *findDatasourceSchema(const std::string &name) const;
    const auto &getDatasourceSchemas() const
    {
        return m_datasourceSchemas;
    }

private:
    std::vector<std::unique_ptr<const gceTypeSchema>> m_schemas;
    std::vector<std::unique_ptr<const gceModelSchema>> m_modelSchemas;
    std::vector<std::unique_ptr<const gceDatasourceSchema>> m_datasourceSchemas;
};

