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
#include "tech_objects3.h"


void gceTechObjects3::init()
{
    //wxStopWatch sw;
    m_progBasic = m_storage.progs.FindOrCreate("objects3");
    if (!m_progBasic->IsOk())
    {
        gl::ProgramInfo info;
        info.addShaderFile(GL_VERTEX_SHADER, "prog/objects3.vert");
        info.addShaderFile(GL_FRAGMENT_SHADER, "prog/objects3.frag");
        if (!m_progBasic->Create(info))
        {
            gceContext::log_error("prog/objects3 load error");
        }
    }
    this->loc_position = m_progBasic->getAttribLocation("position");
    this->loc_mvp = m_progBasic->getUniformLocation("mvp");
    this->loc_scale = m_progBasic->getUniformLocation("scale");
    this->loc_fixedColor = m_progBasic->getUniformLocation("fixedColor");
    //gceContext::log_message("gceTechBasic::init {}ms", sw.Time());

    setVAO();
}

void gceTechObjects3::prepareScene(gceContext &ctx, const glm::dmat4 &proj, const glm::dvec3 &pos, const glm::ivec2 &res)
{
    if (!this->id_model.is_nil())
    {
        umodelQueryMesh2DMsg msg;
        msg.id_model = this->id_model;
        msg.sender = gce::queueId::RENDER;
        ctx.postModelQueue(msg);
    }
    collectBatches(proj, pos);

}

void gceTechObjects3::renderScene(const glm::dmat4 &proj, const glm::dvec3 &pos, const glm::ivec2 &res, bool wireFrameTerrain)
{
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    m_progBasic->Begin();

    glBindVertexArray(m_vao.name());
    for (auto &b : m_batches)
    {
        m_progBasic->setValue(loc_mvp, b.mvp);
        m_progBasic->setValue(loc_fixedColor, b.color);
        m_progBasic->setValue(loc_scale, b.scale);

        //glBindVertexArray(b.vao);
        glBindVertexBuffer(0, b.array, 0, sizeof(tech_objects3::ArrayVertex));

        if (b.elements > 0)
        {
            glVertexArrayElementBuffer(m_vao.name(), b.elements);
            glDrawElements(b.mode, b.count, GL_UNSIGNED_INT, (const GLvoid *)size_t(b.first));
        }
        else
        {
            glDrawArrays(b.mode, b.first, b.count);
        }
    }
    glBindVertexArray(0);
    m_progBasic->End();

}

void gceTechObjects3::collectBatches(const glm::dmat4 &proj, const glm::dvec3 &pos)
{
    m_batches.clear();
    for (const auto &i : m_instances.data())
    {
        if (i.id_mesh == 0) continue;

        auto &mesh = m_meshes[i.id_mesh];

        // Distance from the camera (0,0,0 in view space) to the object
        float dist = glm::distance(pos, glm::dvec3(i.modelMat[3]));

        glm::fmat4 mvp = proj * i.modelMat;
        for (auto &dc : mesh.drawCalls)
        {
            tech_objects3::BatchGL b;
            b.mvp = mvp;
            b.array = dc.array;
            b.elements = dc.elements;
            b.mode = dc.mode;
            b.first = dc.first;
            b.count = dc.count;

            b.color = dc.color;
            b.scale = std::clamp(0.015f * dist, 10.0f, 100000.0f);

            m_batches.push_back(b);
        }
    }
    // TODO: sort/optimize batches
}

void gceTechObjects3::processMsg(const urenderTechDataMsg &msg)
{
    if (msg.id_model == id_model && msg.techType == gceTechType::OBJECTS3)
    {
        processData(static_cast<const tech_objects3::DataMsg &>(msg));
    }
}
//#include <fmt/ranges.h>
void gceTechObjects3::processData(const tech_objects3::DataMsg &msg)
{
    if (msg.clearArrays)
    {
        m_arrays.clear();
    }
    for (auto &item : msg.arrays)
    {
        setArray(item);
    }

    if (msg.clearIndexes)
    {
        m_indexes.clear();
    }
    for (auto &item : msg.indexes)
    {
        setIndex(item);
    }

    if (msg.clearMeshes)
    {
        m_meshes.clear();
    }
    for (auto &item : msg.meshes)
    {
        setMesh(item);
    }

    if (msg.clearInstances)
    {
        m_instances.clear();
        //m_tree.clear();
        //gceContext::log_message("clear inst, inst {}", msg.instances.size());
    }
    for (auto &item : msg.instances)
    {
        setInstance(item);
    }
    //const uint8_t SELD_LAYER_UUID_data[] = {'g', 'c', 'e', 's', 'e', 'l', 'd', 'l', 'a', 'y', 'e', 'r', 'u', 'u', 'i', 'd'};
    //if (id_layer == gce::uuid(SELD_LAYER_UUID_data))
    //{
    //    gceContext::log_message("instances {}", m_tree);
    //    for (auto id : m_tree)
    //    {
     //       auto &i = m_instances[id];
     //       gceContext::log_message("id_instance={}, id_mesh={}", id, i.id_mesh);
     //   }
    //}
}

void gceTechObjects3::setArray(const tech_objects3::ArrayInfo &info)
{
    glNamedBufferData(m_arrays[info.id_array].name(), sizeof(tech_objects3::ArrayVertex) * info.vertexes.size(), info.vertexes.data(), GL_STATIC_DRAW);
}

void gceTechObjects3::setIndex(const tech_objects3::IndexInfo &info)
{
    glNamedBufferData(m_indexes[info.id_index].name(), sizeof(tech_objects3::IndexInfo::index_type) * info.indexes.size(), info.indexes.data(), GL_STATIC_DRAW);
}

void gceTechObjects3::setMesh(const tech_objects3::MeshInfo &info)
{
    using Vertex = tech_objects3::ArrayVertex;

    auto &meshGL = m_meshes[info.id_mesh];

    meshGL.drawCalls.clear();
    meshGL.drawCalls.reserve(info.drawCalls.size());
    for (auto &udc : info.drawCalls)
    {
        auto &pdc = meshGL.drawCalls.emplace_back();
        pdc.mode = toOpenGL(udc.mode);
        pdc.first = udc.first;
        pdc.count = udc.count;

        pdc.color = udc.color;

        pdc.array = m_arrays[udc.id_array].name();
        if (udc.id_index > 0)
        {
            pdc.elements = m_indexes[udc.id_index].name();
        }
    }
}

void gceTechObjects3::setInstance(const tech_objects3::InstanceInfo &info)
{
    auto &i = m_instances[info.id_instance];
    i.id_mesh = info.id_mesh;
    i.modelMat = info.modelMat;
    //m_tree.insert(info.id_instance);
}

void gceTechObjects3::setVAO()
{
    glVertexArrayAttribFormat(m_vao.name(), loc_position, 3, GL_FLOAT, GL_FALSE, offsetof(tech_objects3::ArrayVertex, pos));
    glVertexArrayAttribBinding(m_vao.name(), loc_position, 0);
    glEnableVertexArrayAttrib(m_vao.name(), loc_position);
}
