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
#include "teches/tech_objects2.h"
#include "teches/tech_objects3.h"
#include "model_objects.h"
#include "glm/gtc/matrix_transform.hpp"
#include "alg/wgsop.h"

//static constexpr char none_svg[] = R"rawsvg()rawsvg";

static constexpr char objects_svg[] = R"rawsvg(<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24"><title>Bxs Plane SVG Icon</title><path d="M22 16.21v-1.895L14 8V4a2 2 0 0 0-4 0v4.105L2 14.42v1.789l8-2.81V18l-3 2v2l5-2l5 2v-2l-3-2v-4.685l8 2.895z" fill="currentColor"/></svg>)rawsvg";

gceModelObjects::gceModelObjects(const gce::model_info &info) : gceModelBase(info)
{
    //this->id_table = id_table;
    m_geoProj = gceProjection::create(gceProjectionType::WGS84_VISUALIZATION, {});

    for (size_t i = 0; i < maxInstances; ++i)
    {
        int cx = i % 100;
        int cy = i / 100;
        m_data[i].wgspos = glm::dvec3(glm::mix(-180, +180, static_cast<double>(cx) / 100.0), glm::mix(-70.0, 70.0, static_cast<double>(cy) / 100.0), 6000.0);
        m_data[i].v = glm::normalize(glm::dvec2((i * 100) % 37 - 18, i % 27 - 13));
        m_data[i].v *= 1.0 + double((i * 100) % 37) / 10.0;
        //m_data[i].v = glm::dvec2(1.0, 1.0);
        //m_geoProj->toInternal(m_data[i].v, glm::dvec2(m_data[i].wgspos));
    }
}

void gceModelObjects::postFlat(gceContext &ctx)
{
    using vertex_t = tech_objects2::ArrayVertex;

    auto msg = std::make_shared<tech_objects2::DataMsg>();
    //msg->sender = gce::queueId::MODEL;
    msg->id_model = this->id_model;

    if (!this->meshSent2)
    {
        const std::vector<vertex_t> verts
        {
            {{0.0f, 1.0f, 0.0f}}, {{-0.25f, -1.0f, 0.0f}}, {{0.25f, -1.0f, 0.0f}},
            {{0.0f, 0.3f, 0.0f}}, {{-0.95f, -0.2f, 0.0f}}, {{0.95f, -0.2f, 0.0f}}
        };

        msg->arrays.emplace_back(verts, 1);

        auto &mesh = msg->meshes.emplace_back();
        mesh.id_mesh = 1;

        auto &dc = mesh.drawCalls.emplace_back();
        dc.id_array = 1;
        dc.mode = gcePimitiveType::TRIANGLES;
        dc.color = glm::fvec4(1.0f, 1.0f, 0.0f, 1.0f);
        dc.first = 0;
        dc.count = verts.size();

        this->meshSent2 = true;
    }
    msg->instances.reserve(maxInstances);
    for (size_t i = 0; i < maxInstances; ++i)
    {
        const auto &entry = m_data[i];
        auto &inst = msg->instances.emplace_back();
        inst.id_instance = i;
        inst.id_mesh = 1;
        glm::dvec2 pos;
        m_geoProj->fromInternal(pos, glm::dvec2(entry.wgspos));
        glm::dmat4 rot = glm::rotate(glm::translate(glm::identity<glm::dmat4>(), glm::dvec3(pos, 0.0)), gce::angleX(entry.v) - M_PI / 2, glm::dvec3(0.0, 0.0, 1.0));
        inst.modelMat = rot;
    }

    ctx.postRenderQueue(std::static_pointer_cast<urenderTechDataMsg>(msg));
}
void gceModelObjects::postGlobe(gceContext &ctx)
{
    using vertex_t = tech_objects3::ArrayVertex;

    auto msg = std::make_shared<tech_objects3::DataMsg>();
    //msg->sender = gce::queueId::MODEL;
    msg->id_model = this->id_model;

    if (!this->meshSent3)
    {
        const std::vector<vertex_t> verts
        {
            {{0.0f, 1.0f, 0.0f}}, {{-0.25f, -1.0f, 0.0f}}, {{0.25f, -1.0f, 0.0f}},
            {{0.0f, 0.3f, 0.0f}}, {{-0.95f, -0.2f, 0.0f}}, {{0.95f, -0.2f, 0.0f}}
        };

        msg->arrays.emplace_back(verts, 1);

        auto &mesh = msg->meshes.emplace_back();
        mesh.id_mesh = 1;

        auto &dc = mesh.drawCalls.emplace_back();
        dc.id_array = 1;
        dc.mode = gcePimitiveType::TRIANGLES;
        //dc.mode = gcePimitiveType::POINTS;
        dc.color = glm::fvec4(1.0f, 1.0f, 0.0f, 1.0f);
        dc.first = 0;
        dc.count = verts.size();

        this->meshSent3 = true;
    }
    msg->instances.reserve(maxInstances);
    for (size_t i = 0; i < maxInstances; ++i)
    {
        const auto &entry = m_data[i];
        //entry_type entry;
        //entry.wgspos = glm::dvec3(45.0, 45.0, 1000.0);
        wgsop::Location placer(glm::radians(entry.wgspos.x), glm::radians(entry.wgspos.y), entry.wgspos.z);
        auto &inst = msg->instances.emplace_back();
        inst.id_instance = i;
        inst.id_mesh = 1;
        glm::dmat4 rot = glm::rotate(placer.getPlacementMatrix(0.0), gce::angleX(entry.v) - M_PI / 2, glm::dvec3(0.0, 0.0, 1.0));
        inst.modelMat = rot;// placer.getPlacementMatrix(0.0);
    }

    ctx.postRenderQueue(std::static_pointer_cast<urenderTechDataMsg>(msg));
}

void gceModelObjects::processIdle(gceContext &ctx)
{
    for (auto &entry : m_data)
    {
        entry.wgspos.x += entry.v.x * 0.01;
        entry.wgspos.y += entry.v.y * 0.01;
        if (entry.wgspos.y < -80.0 || entry.wgspos.y > 80.0)
        {
            entry.v.y = -entry.v.y;
        }
    }
    postFlat(ctx);
    postGlobe(ctx);
    ++cnt;
}


void gceModelObjects::queryMesh2D(gceContext &ctx, gce::tileid)
{
    // TODO: control if mesh is sent based on every layer id/client data version
    if (meshSent2)
    {
        return;
    }
    meshSent2 = true;

    for (auto &[key, index] : m_index)
    {
        //postMesh(ctx, index);
    }
    //ctx.log().message("gceModelGeometric::queryMesh2D");
}

/*void gceModelObjects::processSelect2D(const umodelSelect2DMsg &msg, umodelSelectXDResultMsg &rmsg)
{
    auto pos = glm::dvec2(msg.aoi);
    for (auto &[key, index] : m_index)
    {
        if (auto *entry = m_data.getX(index); entry->g)
        {
            double d = entry->g->distance(pos);
            if (d <= msg.aoi.z)
            {
                place_select_result_with_limit(rmsg.data, {id_table, key}, d, msg.limit);
            }
        }
    }
}*/



std::unique_ptr<const gceModelSchema> gceModelObjects::schema()
{
    auto schema = std::make_unique<gceModelSchema>();
    schema->m_name = "Objects";
    schema->m_svgIcon = objects_svg;
    schema->m_factory = [](const gce::model_info &info)->std::unique_ptr<gceModelBase>{return std::make_unique<gceModelObjects>(info); };
    //schema->m_tableInputs.emplace_back("Geometry", gce::geometry_schema_id);
    schema->m_techFlat = gceTechType::OBJECTS2;
    schema->m_techGlobe = gceTechType::OBJECTS3;
    return schema;
}
