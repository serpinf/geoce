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
#include "config.h"
#include "type/coretypes.h"
#include "models/imagetile.h"
#include "models/geometric.h"
#include "models/selectionmodel.h"
#include "models/modeldemtile.h"
#include "models/model_objects.h"

#include "datasrc/pgnode.h"
#include "datasrc/wmsnode.h"
#include "datasrc/demdata.h"

gceConfig::gceConfig()
{}

gceConfig::~gceConfig()
{}

bool gceConfig::load(const std::string &)
{
    m_schemas.push_back(gce::buildingSchema());
    m_schemas.push_back(gce::fenceSchema());
    m_schemas.push_back(gce::treeSchema());
    m_schemas.push_back(gce::rasterSchema());
    m_schemas.push_back(gce::geometrySchema());

    m_modelSchemas.emplace_back(gceModelImageTiled::schema());
    m_modelSchemas.emplace_back(gceModelGeometric::schema());
    m_modelSchemas.emplace_back(gceSelectionModel::schema());
    m_modelSchemas.emplace_back(gceModelDEMTile::schema());
    m_modelSchemas.emplace_back(gceModelObjects::schema());

    m_datasourceSchemas.emplace_back(gceDataConnectionPG::schema());
    m_datasourceSchemas.emplace_back(gceDataConnectionWMS::schema());
    m_datasourceSchemas.emplace_back(gceDataConnectionDEM::schema());

    return true;
}

const gceTypeSchema *gceConfig::findSchema(const gce::uuid &id) const
{
    for (auto &schema : this->m_schemas)
    {
        if (schema->getId() == id)
        {
            return schema.get();
        }
    }
    return {};
}

const gceTypeSchema *gceConfig::findSchema(const std::string &name) const
{
    for (auto &schema : this->m_schemas)
    {
        if (schema->getName() == name)
        {
            return schema.get();
        }
    }
    return {};
}

std::string gceConfig::getSchemaName(const gce::uuid &id) const
{
    if (auto schema = this->findSchema(id); schema)
    {
        return schema->getName();
    }
    return "none";
}

const gceModelSchema *gceConfig::findModelSchema(const std::string &name) const
{
    for (auto &schema : this->m_modelSchemas)
    {
        if (schema->getName() == name)
        {
            return schema.get();
        }
    }
    return nullptr;
}

const gceDatasourceSchema *gceConfig::findDatasourceSchema(const std::string &name) const
{
    for (auto &schema : this->m_datasourceSchemas)
    {
        if (schema->m_name == name)
        {
            return schema.get();
        }
    }
    return nullptr;
}
