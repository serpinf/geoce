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
#include "modeltest.hpp"
#include "teches/techbasic.h"
#include "glm/gtc/matrix_transform.hpp"

void gceModelTest::processIdle(gceContext &)
{
    m_status = gceEntityStatus::OK;
    cnt++;
}

void gceModelTest::queryMesh2D(gceContext &ctx, gce::tileid)
{
    using vertex_t = tech_basic::ArrayVertex;

    auto msg = std::make_shared<tech_basic::DataMsg>();
    //msg->sender = gce::queueId::MODEL;
    msg->id_model = this->id_model;

    if (!this->meshSent)
    {
        const std::vector<vertex_t> verts{{{0.0f, 0.5f, 0.0f}}, {{1.0f, 0.5f, 0.0f}}, {{0.0f, 0.0f, 0.0f}}};

        msg->arrays.emplace_back(verts, 1);

        auto &mesh = msg->meshes.emplace_back();
        mesh.id_mesh = 1;

        auto &dc = mesh.drawCalls.emplace_back();
        dc.id_array = 1;
        dc.mode = gcePimitiveType::TRIANGLES;
        dc.first = 0;
        dc.count = 3;

        this->meshSent = true;
    }

    msg->instances.reserve(2);
    {
        auto &inst = msg->instances.emplace_back();
        inst.id_instance = 1;
        inst.id_mesh = 1;
        inst.modelMat = glm::scale(glm::identity<glm::dmat4>(), glm::dvec3(0.1));
    }

    {
        auto &inst = msg->instances.emplace_back();
        inst.id_instance = 2;
        inst.id_mesh = 1;
        inst.modelMat = glm::translate(glm::scale(glm::identity<glm::dmat4>(), glm::dvec3(0.1)), glm::dvec3(-0.5 + 0.01 * cnt, 0.0, 0.0));
    }
    ctx.postRenderQueue(std::static_pointer_cast<urenderTechDataMsg>(msg));
}
