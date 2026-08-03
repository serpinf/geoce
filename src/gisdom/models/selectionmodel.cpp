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
#include "selectionmodel.h"
#include "teches/techbasic.h"
#include "geom/LineString.h"
#include "geom/Polygon.h"
#include <glm/gtc/matrix_transform.hpp>

gceSelectionModel::gceSelectionModel(const gce::model_info &info) : gceModelBase(info)
{
    m_status = gceEntityStatus::OK;
    m_geoProj = gceProjection::create(gceProjectionType::WGS84_VISUALIZATION, {});

    // reserve zero index mesh
    uint32_t zeroIndex;
    m_data.newX(zeroIndex);
}

void gceSelectionModel::processUpdate(gceContext &, const umodelUpdateMsg &msg)
{
    if (msg.props)
    {
        auto &newProps = static_cast<const gceSelectionModelProprties &>(*msg.props);
        if (this->m_props.drawPoints != newProps.drawPoints)
        {
            this->m_props.drawPoints = newProps.drawPoints;
            meshSent = false;
        }
        if (m_props.pose != newProps.pose)
        {
            m_props.pose = newProps.pose;
            instanceSent = false;
        }

        if (m_props.poseArray.size() != newProps.poseArray.size())
        {
            size_t sz_old = m_props.poseArray.size(), sz_new = newProps.poseArray.size();
            for (auto &[key, index] : m_index)
            {
                if (sz_old < sz_new)
                {
                    index.extraInstances.reserve(sz_new);
                    while (index.extraInstances.size() < sz_new)
                    {
                        uint32_t id;
                        if (m_instances.newX(id))
                        {
                            index.extraInstances.push_back(id);
                        }
                    }
                }
                else // if (sz_old > sz_new)
                {
                    while (index.extraInstances.size() > sz_new)
                    {
                        m_instances.delX(index.extraInstances.back());
                        index.extraInstances.pop_back();
                    }
                }
            }
        }

        if (m_props.poseArray != newProps.poseArray)
        {
            m_props.poseArray = newProps.poseArray;
            instanceSent = false;
        }
    }
}

void gceSelectionModel::queryMesh2D(gceContext &ctx, gce::tileid)
{
    if (this->instanceSent && this->arraysSent && this->meshSent)
    {
        return;
    }

    auto msg = std::make_shared<tech_basic::DataMsg>();
    msg->id_model = this->id_model;


    if (!this->arraysSent)
    {
        msg->arrays.reserve(m_index.size());
    }

    if (!this->meshSent)
    {
        msg->meshes.reserve(m_index.size());
    }

    if (!this->instanceSent)
    {
        msg->instances.reserve(m_index.size() * std::max<size_t>(m_props.poseArray.size(), 1));
        msg->clearInstances = true;
    }

    for (auto &[key, index] : m_index)
    {
        if (!this->arraysSent)
        {
            auto &info = msg->arrays.emplace_back();
            postArray(info, index.id_mesh);
        }

        if (!this->meshSent)
        {
            auto &info = msg->meshes.emplace_back();
            postMesh(info, index.id_mesh);
        }

        if (!this->instanceSent)
        {
            msg->instances.emplace_back(index.id_instance, index.id_mesh, m_props.pose);
            size_t n = 0;
            for (auto &pose : m_props.poseArray)
            {
                msg->instances.emplace_back(index.extraInstances[n++], index.id_mesh, pose);
            }
        }
    }

    ctx.postRenderQueue(std::static_pointer_cast<urenderTechDataMsg>(msg));

    this->instanceSent = true;
    this->arraysSent = true;
    this->meshSent = true;

}

void gceSelectionModel::processActionNotify(gceContext &ctx, const udataMultiRowActionNotifyMsg &msg)
{
    for (auto &action : msg.m_actions)
    {
        if (action.query == gceActionType::Delete)
        {
            this->removeEntity(ctx, {action.id_table, action.oldEntity});
        }
        else if (action.query == gceActionType::Update)
        {
            this->updateEntity(ctx, {action.id_table, action.newEntity});
        }
    }
}

void gceSelectionModel::processEntityAdd(gceContext &ctx, const umodelEntityAddMsg &msg)
{
    if (addEntity(ctx, msg.data))
    {
        // TODO: do not invalidate the whole scene, send only new entity
        this->arraysSent = false;
        this->meshSent = false;
        this->instanceSent = false;
    }
}

void gceSelectionModel::processEntityUpdate(gceContext &ctx, const umodelEntityUpdateMsg &msg)
{
    if (!msg.data.empty())
    {
        this->updateEntity(ctx, msg.data);
    }
}

void gceSelectionModel::processEntityRemove(gceContext &ctx, const umodelEntityRemoveMsg &msg)
{
    if (msg.data.empty())
    {
        this->removeEntityAll(ctx);
    }
    else
    {
        this->removeEntity(ctx, msg.data);
    }
}

bool gceSelectionModel::addEntity(gceContext &, const gceEntityPackedRef &ref)
{
    auto &index = m_index[ref.get_key()];
    if (m_data.newX(index.id_mesh) && m_instances.newX(index.id_instance))
    {
        index.extraInstances.reserve(m_props.poseArray.size());
        std::generate_n(std::back_inserter(index.extraInstances), m_props.poseArray.size(), [this](){
            uint32_t id; m_instances.newX(id); return id;
        });

        (void)setEntry(index.id_mesh, ref);
    }
    else
    {
        gceContext::log_message("ModelSelection: data overflow");
        return false;
    }

    return true;
}

bool gceSelectionModel::setEntry(uint32_t index, const gceEntityPackedRef &ref)
{
    if (entry_type *pEntry = m_data.getX(index); pEntry != nullptr)
    {
        pEntry->g = ref.entity.get_geometry();
        if (pEntry->g)
        {
            project_filter pf(this->m_geoProj.get());
            pEntry->g->apply_filter_rw(pf);
        }
        pEntry->color = ref.isTableRow() ? glm::fvec4(0.0f, 0.0f, 0.7f, 1.0f) : glm::fvec4(0.0f, 0.8f, 0.8f, 1.0f);
        return true;
    }
    return false;
}

bool gceSelectionModel::updateEntity(gceContext &ctx, const gceEntityPackedRef &ref)
{
    if (auto it = this->m_index.find(ref.get_key()); it != this->m_index.end())
    {
        auto id_mesh = it->second.id_mesh;
        if (this->setEntry(id_mesh, ref))
        {
            // posted/processed as one message or batches can be incompatible with array on rendering
            auto msg = std::make_shared<tech_basic::DataMsg>();
            msg->id_model = this->id_model;

            auto &array = msg->arrays.emplace_back();
            this->postArray(array, id_mesh);
            auto &mesh = msg->meshes.emplace_back();
            this->postMesh(mesh, id_mesh);

            ctx.postRenderQueue(std::static_pointer_cast<urenderTechDataMsg>(msg));
        }
    }
    return true;
}

bool gceSelectionModel::removeEntity(gceContext &ctx, const gceEntityPackedRef &ref)
{
    if (auto it = this->m_index.find(ref.get_key()); it != this->m_index.end())
    {
        postRemoveMesh(ctx, it->second);
        free_indexes(it->second);
        m_index.erase(it);
        return true;
    }
    return false;
}

bool gceSelectionModel::removeEntityAll(gceContext &ctx)
{
    if (this->m_index.empty())
    {
        return false;
    }

    auto msg = std::make_shared<tech_basic::DataMsg>();
    msg->id_model = this->id_model;
    msg->clearArrays = true;
    msg->clearIndexes = true;
    msg->clearMeshes = true;
    msg->clearInstances = true;
    ctx.postRenderQueue(std::static_pointer_cast<urenderTechDataMsg>(msg));

    for (auto start = m_index.begin(), end = m_index.end(); start != end; )
    {
        free_indexes(start->second);
        start = m_index.erase(start);
    }
    return true;
}

void gceSelectionModel::postArray(tech_basic::ArrayInfo &info, uint32_t index)
{
    auto *entry = m_data.getX(index);
    info.id_array = index;

    if (entry->g)
    {
        info.vertexes.reserve(entry->g->getNumPoints());
        tech_basic::CollectArrayData_filter f(info);
        entry->g->apply_filter_ro(f);
    }
}

namespace
{
struct LineCallsCollector final : public  geom::GeometryFilter
{
    void operator()(const geom::Point &) final
    {
        ++vertexOffset;
    }
    void operator()(const geom::LineString &g) final
    {
        process(g.getCoordSeq());
    }
    void operator()(const geom::Polygon &poly) final
    {
        process(poly.m_shell);
        for (auto &ring : poly.m_holes)
        {
            process(ring);
        }
    }

    void process(const geom::CoordinateSeq &seq)
    {
        uint32_t numPoints = seq.size();
        lineStrips.emplace_back(vertexOffset, numPoints);
        vertexOffset += numPoints;
    }

    // put calls LineStrings and Rings of the Polynons here
    std::vector<std::pair<size_t, size_t>> lineStrips;

    uint32_t vertexOffset = 0;
};
}
void gceSelectionModel::postMesh(tech_basic::MeshInfo &info, uint32_t index)
{
    auto *entry = m_data.getX(index);

    info.id_mesh = index;

    if (entry->g)
    {
        const geom::Geometry *g = entry->g.get();
        if (g->getDimension() == 0)
        {
            auto &dc = info.drawCalls.emplace_back();
            dc.id_array = index;
            dc.mode = gcePimitiveType::POINTS;
            dc.first = 0;
            dc.count = (int32_t)g->getNumPoints();
            dc.useCoor = true;
            dc.pointSize = 4.0f;
            dc.color = entry->color;
        }
        else
        {
            LineCallsCollector lcc;
            lcc.lineStrips.reserve(g->getNumGeometries());
            g->apply_geometry_filter(lcc);
            info.drawCalls.reserve(lcc.lineStrips.size());
            for (auto &strip : lcc.lineStrips)
            {
                auto &dc = info.drawCalls.emplace_back();
                dc.id_array = index;
                dc.mode = gcePimitiveType::LINE_STRIP;
                dc.first = strip.first;
                dc.count = strip.second;
                dc.useCoor = true;
                dc.color = entry->color;
            }
        }

        if (this->m_props.drawPoints)
        {
            auto &dc = info.drawCalls.emplace_back();
            dc.id_array = index;
            dc.mode = gcePimitiveType::POINTS;
            dc.first = 0;
            dc.count = (int32_t)entry->g->getNumPoints();
            dc.useCoor = true;
            dc.pointSize = 8.0f;
            dc.color = {0.99f, 0.99f, 0.0f, 1.0f};
        }
    }
}

void gceSelectionModel::postInstance(tech_basic::InstanceInfo &info, uint32_t index)
{
    info.id_instance = index;
    info.id_mesh = index;
    info.modelMat = glm::identity<glm::dmat4>();
}

void gceSelectionModel::postRemoveMesh(gceContext &ctx, const indexes_type &i)
{
    // TODO: use proper remove message instead of update using zero mesh index
    auto msg = std::make_shared<tech_basic::DataMsg>();
    msg->id_model = this->id_model;
    msg->indexes.reserve(i.extraInstances.size() + 1);

    auto &inst = msg->instances.emplace_back();
    inst.id_instance = i.id_instance;
    inst.id_mesh = 0;

    for (auto &id_instance : i.extraInstances)
    {
        auto &einst = msg->instances.emplace_back();
        einst.id_instance = id_instance;
        einst.id_mesh = 0;
    }

    ctx.postRenderQueue(std::static_pointer_cast<urenderTechDataMsg>(msg));
}

void gceSelectionModel::processSelect2D(const umodelSelect2DMsg &, umodelSelectXDResultMsg &)
{}

void gceSelectionModel::free_indexes(indexes_type &i)
{
    m_data.delX(i.id_mesh);
    m_instances.delX(i.id_instance);
    for (auto &id_instance : i.extraInstances)
    {
        m_instances.delX(id_instance);
    }
}

std::unique_ptr<const gceModelSchema> gceSelectionModel::schema()
{
    auto schema = std::make_unique<gceModelSchema>();
    schema->m_name = gceModelType_SELECTION;
    schema->m_factory = [](const gce::model_info &info)->std::unique_ptr<gceModelBase>{return std::make_unique<gceSelectionModel>(info); };
    schema->m_techFlat = gceTechType::BASIC_RTE2;
    schema->internal = true;
    return schema;
}
