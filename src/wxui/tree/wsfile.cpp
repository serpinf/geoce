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
#include "wsfile.h"
#include "boost/uuid/uuid_io.hpp"

#include <fstream>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/version.hpp>
#include <set>
#include <filesystem>

template<class Archive>
inline void gce::datasrc_info::serialize(Archive &ar, const unsigned int)
{
    ar &BOOST_SERIALIZATION_NVP(id_datasrc);
    ar &BOOST_SERIALIZATION_NVP(type);
    ar &BOOST_SERIALIZATION_NVP(name);
    ar &BOOST_SERIALIZATION_NVP(params);
    ar &BOOST_SERIALIZATION_NVP(autoConnect);
}
BOOST_CLASS_VERSION(gce::datasrc_info, 1);

template<class Archive>
inline void gce::table_info::serialize(Archive &ar, const unsigned int)
{
    ar &BOOST_SERIALIZATION_NVP(id_table);
    ar &BOOST_SERIALIZATION_NVP(id_datasrc);
    ar &BOOST_SERIALIZATION_NVP(id_schema);
    ar &BOOST_SERIALIZATION_NVP(flags);
    ar &BOOST_SERIALIZATION_NVP(label);
    ar &BOOST_SERIALIZATION_NVP(name);
    ar &BOOST_SERIALIZATION_NVP(dbSchema);
}
BOOST_CLASS_VERSION(gce::table_info, 1);

template<class Archive>
inline void gce::model_table_input_info::serialize(Archive &ar, const unsigned int)
{
    //ar &BOOST_SERIALIZATION_NVP(name);
    ar &BOOST_SERIALIZATION_NVP(id_table);
    ar &BOOST_SERIALIZATION_NVP(column_map);
}
BOOST_CLASS_VERSION(gce::model_table_input_info, 1);

template<class Archive>
inline void gce::model_model_input_info::serialize(Archive &ar, const unsigned int)
{
    //ar &BOOST_SERIALIZATION_NVP(name);
    ar &BOOST_SERIALIZATION_NVP(id_model);
}
BOOST_CLASS_VERSION(gce::model_model_input_info, 1);

template<class Archive>
inline void gce::model_info::serialize(Archive &ar, const unsigned int)
{
    ar &BOOST_SERIALIZATION_NVP(id_model);
    ar &BOOST_SERIALIZATION_NVP(label);
    ar &BOOST_SERIALIZATION_NVP(type);
    ar &BOOST_SERIALIZATION_NVP(tableInputs);
    ar &BOOST_SERIALIZATION_NVP(modelInputs);
}
BOOST_CLASS_VERSION(gce::model_info, 1);

template<class Archive>
inline void gce::scene_info::serialize(Archive &ar, const unsigned int)
{
    ar &BOOST_SERIALIZATION_NVP(id_scene);
    ar &BOOST_SERIALIZATION_NVP(visible);
    ar &BOOST_SERIALIZATION_NVP(label);
    ar &BOOST_SERIALIZATION_NVP(srs);
    ar &BOOST_SERIALIZATION_NVP(style);
}
BOOST_CLASS_VERSION(gce::scene_info, 1);

template<class Archive>
inline void gce::layer_info::serialize(Archive &ar, const unsigned int)
{
    ar &BOOST_SERIALIZATION_NVP(id_layer);
    ar &BOOST_SERIALIZATION_NVP(id_scene);
    ar &BOOST_SERIALIZATION_NVP(id_model);
    ar &BOOST_SERIALIZATION_NVP(label);
    ar &BOOST_SERIALIZATION_NVP(visible);
    ar &BOOST_SERIALIZATION_NVP(order);
}
BOOST_CLASS_VERSION(gce::layer_info, 1);

template<class Archive>
inline void gce::workspace_info::serialize(Archive &ar, const unsigned int)
{
    ar &boost::serialization::make_nvp("datasrcs", datasrcs.get());
    ar &boost::serialization::make_nvp("tables", tables.get());
    ar &boost::serialization::make_nvp("models", models.get());
    ar &boost::serialization::make_nvp("scenes", scenes.get());
    ar &boost::serialization::make_nvp("layers", layers.get());
}
BOOST_CLASS_VERSION(gce::workspace_info, 1);

namespace gce
{
//static bool operator < (const layer_info &val, const uuid &key) { return val.id_layer < key; }
//static bool operator < (const uuid &key, const layer_info &val) { return key < val.id_layer; }
}

gceWorkspace::gceWorkspace(gceContext &ctx, const gceConfig &cfg) : m_ctx(ctx), m_cfg(cfg)
{}

gceWorkspace::~gceWorkspace()
{
    saveFile();
}

void gceWorkspace::saveFile()
{
    // save data to archive
    if (std::ofstream ofs(m_filename); ofs.good())
    {
        boost::archive::xml_oarchive oa(ofs);
        // write class instance to archive
        oa << boost::serialization::make_nvp("gceworkspace", m_info);
        // archive and stream closed when destructors are called
    }
}

bool gceWorkspace::loadFile(const std::string &path, bool fCreate)
{
    if (m_filename == path)
    {
        return false;
    }

    bool res = false;
    if (!m_filename.empty())
    {
        m_ctx.postQueue(gce::queueId::DATA | gce::queueId::MODEL | gce::queueId::RENDER, wspResetMsg{});
        saveFile();
        m_info = {};
    }

    // open the archive
    if (std::ifstream ifs(path); ifs.good())
    {
        try
        {
            boost::archive::xml_iarchive ia(ifs);
            // restore the workspace from the archive
            ia >> BOOST_SERIALIZATION_NVP(m_info);
            m_filename = path;
            res = true;
        }
        catch (std::exception &e)
        {
            m_info = {}; // reset to empty state to avoid partial data
            gceContext::log_error("{}", e.what());
        }
        catch (...)
        {
            m_info = {}; // reset to empty state to avoid partial data
            gceContext::log_error("Unknown error while loading workspace file");
        }
    }
    else if (fCreate)
    {
        m_filename = path;
        saveFile();
        res = true;
    }
    return res;
}
std::vector<gce::datasrc_info> gceWorkspace::getDataSources()
{
    return m_info.datasrcs.get();
}

void gceWorkspace::addDataSource(gce::datasrc_info &ds)
{
    m_info.datasrcs.insert(ds);
}

void gceWorkspace::updateDataSource(const gce::datasrc_info &ds)
{
    m_info.datasrcs.update(ds);
}

void gceWorkspace::removeDataSource(const gce::datasrc_info &info)
{
    if (m_info.datasrcs.erase(info.id_datasrc))
    {
        udataSvcRemoveMsg msg;
        msg.id_datasrc = info.id_datasrc;
        m_ctx.postWorkspaceQueue(msg); // TODO: post also to data svc?
    }
}

std::vector<gce::table_info> gceWorkspace::getTables()
{
    return m_info.tables.get();
}
std::vector<gce::table_info> gceWorkspace::getTables(const gce::datasrc_info &ds)
{
    return m_info.tables.select_if([id_datasrc = ds.id_datasrc](const gce::table_info &info){return info.id_datasrc == id_datasrc; });
}
void gceWorkspace::addTable(const gce::table_info &info)
{
    m_info.tables.insert(info);
}
void gceWorkspace::updateTable(const gce::table_info &info)
{
    if (m_info.tables.update(info))
    {
        m_ctx.postWorkspaceQueue(wspTableUpdatedMsg{info});
    }
}
void gceWorkspace::removeTable(const gce::table_info &info, bool dropSourceTable)
{
    if (m_info.tables.erase(info.id_table))
    {
        m_ctx.postWorkspaceQueue(wspTableDeletedMsg{info.id_table});
        if (dropSourceTable)
        {
            m_ctx.postDataQueue(udataDropTableMsg{info.id_table});
        }
    }
}

std::vector<gce::model_info> gceWorkspace::getModels()
{
    return m_info.models.get();
}

std::optional<gce::model_info> gceWorkspace::getModel(const gce::uuid &id_model)
{
    return m_info.models.get_optional(id_model);
}

void gceWorkspace::addModel(const gce::model_info &info)
{
    m_info.models.insert(info);
}

void gceWorkspace::updateModel(const gce::model_info &info)
{
    if (m_info.models.update(info))
    {
        m_ctx.postWorkspaceQueue(wspModelUpdatedMsg{info});
    }
}

void gceWorkspace::removeModel(const gce::model_info &info)
{
    if (m_info.models.erase(info.id_model))
    {
        m_ctx.postQueue(gce::queueId::MODEL | gce::queueId::WORKSPACE | gce::queueId::RENDER, wspModelDeletedMsg{info.id_model});
    }
}

std::optional<gce::scene_info> gceWorkspace::getScene(const gce::uuid &id_scene)
{
    return m_info.scenes.get_optional(id_scene);
}
std::vector<gce::scene_info> gceWorkspace::getScenes()
{
    return m_info.scenes.get();
}
void gceWorkspace::addScene(const gce::scene_info &info)
{
    m_info.scenes.insert(info);
}
void gceWorkspace::updateScene(const gce::scene_info &info)
{
    if (m_info.scenes.update(info))
    {
        m_ctx.postWorkspaceQueue(wspSceneUpdatedMsg{info});
    }
}
void gceWorkspace::removeScene(const gce::scene_info &info, bool recursive)
{
    // TODO: check how it works
    if (recursive)
    {
        // remove scene_layer_ref objects referencing the scene
        m_info.layers.erase_if([id_scene = info.id_scene](const gce::layer_info &sl){return id_scene == sl.id_scene; });
    }
    // remove scene
    m_info.scenes.erase(info.id_scene);

    m_ctx.postWorkspaceQueue(wspSceneDeletedMsg{info.id_scene});
}

std::optional<gce::layer_info> gceWorkspace::getLayer(const gce::uuid &id_layer)
{
    return m_info.layers.get_optional(id_layer);

}
std::vector<gce::layer_info> gceWorkspace::getLayers()
{
    return m_info.layers.get();
}
void gceWorkspace::addLayer(const gce::layer_info &info)
{
    m_info.layers.insert(info);
}
void gceWorkspace::updateLayer(const gce::layer_info &info)
{
    if (m_info.layers.update(info))
    {
        m_ctx.postWorkspaceQueue(wspLayerUpdatedMsg{info});
    }
}
void gceWorkspace::removeLayer(const gce::layer_info &info)
{
    if (m_info.layers.erase(info.id_layer))
    {
        m_ctx.postWorkspaceQueue(wspLayerDeletedMsg{info.id_layer});
    }
}

std::vector<gce::layer_info> gceWorkspace::getSceneLayers(const gce::uuid &id_scene)
{
    return m_info.layers.select_if([id_scene](const gce::layer_info &info){
        return info.id_scene == id_scene;
    });
}

std::string gceWorkspace::getName() const
{
    if (!m_filename.empty())
    {
        return std::filesystem::path(m_filename).stem().string();
    }
    return {};
}
