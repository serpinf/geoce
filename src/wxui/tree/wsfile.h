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
#include "type/typeschema.hpp"
#include "engine.hpp"
#include "config.h"
#include "key_sorted.h"


namespace gce
{
struct workspace_info
{
    table_data<datasrc_info, gce::uuid> datasrcs;
    table_data<table_info, gce::uuid> tables;
    table_data<model_info, gce::uuid> models;
    table_data<scene_info, gce::uuid> scenes;
    table_data<layer_info, gce::uuid> layers;

    template<class Archive>
    void serialize(Archive &ar, const unsigned int version);
};
} // gce

class gceWorkspace
{
public:
    explicit gceWorkspace(gceContext &ctx, const gceConfig &cfg);
    ~gceWorkspace();
    bool isOk() const
    {
        return !m_filename.empty();
    }
    bool loadFile(const std::string &path, bool fCreate);

    std::vector<gce::datasrc_info> getDataSources();
    void updateDataSource(const gce::datasrc_info &ds);
    // sets id
    void addDataSource(gce::datasrc_info &ds);
    void removeDataSource(const gce::datasrc_info &ds);

    std::vector<gce::table_info> getTables();
    std::optional<gce::table_info> getTable(const gce::uuid &id_table) const
    {
        return m_info.tables.get_optional(id_table);
    }
    std::vector<gce::table_info> getTables(const gce::datasrc_info &ds);
    void addTable(const gce::table_info &info);
    void updateTable(const gce::table_info &info);
    void removeTable(const gce::table_info &info, bool dropSourceTable);

    std::vector<gce::model_info> getModels();
    std::optional<gce::model_info> getModel(const gce::uuid &id_model);
    void addModel(const gce::model_info &info);
    void updateModel(const gce::model_info &info);
    void removeModel(const gce::model_info &info);

    std::optional<gce::scene_info> getScene(const gce::uuid &id_scene);
    std::vector<gce::scene_info> getScenes();
    void addScene(const gce::scene_info &info);
    void updateScene(const gce::scene_info &info);
    void removeScene(const gce::scene_info &info, bool recursive);

    std::optional<gce::layer_info> getLayer(const gce::uuid &id_layer);
    std::vector<gce::layer_info> getLayers();
    std::vector<gce::layer_info> getSceneLayers(const gce::uuid &id_scene);
    void addLayer(const gce::layer_info &info);
    void updateLayer(const gce::layer_info &info);
    void removeLayer(const gce::layer_info &info);


    gceContext &ctx()
    {
        return m_ctx;
    }

    const gceTypeSchema *findSchema(const gce::uuid &id) const
    {
        return m_cfg.findSchema(id);
    }
    const gceTypeSchema *findSchema(const std::string &name) const
    {
        return m_cfg.findSchema(name);
    }
    std::string getSchemaName(const gce::uuid &id) const
    {
        return m_cfg.getSchemaName(id);
    }
    const auto &schemas() const
    {
        return m_cfg.getSchemas();
    }
    const auto &modelSchemas() const
    {
        return m_cfg.getModelSchemas();
    }
    const gceModelSchema *findModelSchema(const std::string &name) const
    {
        return m_cfg.findModelSchema(name);
    }
    std::string getName() const;
private:
    void saveFile();
    gceContext &m_ctx;
    const gceConfig &m_cfg;
    std::string m_filename;
    gce::workspace_info m_info;
};
