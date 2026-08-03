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
#include "techbasic.h"


void gceTechBasic::init()
{
    //wxStopWatch sw;
    m_progBasic = m_storage.progs.FindOrCreate("basic");
    if (!m_progBasic->IsOk())
    {
        gl::ProgramInfo info;
        info.addShaderFile(GL_VERTEX_SHADER, "prog/basic.vert");
        info.addShaderFile(GL_FRAGMENT_SHADER, "prog/basic.frag");
        if (!m_progBasic->Create(info))
        {
            gceContext::log_error("prog/basic load error");
        }
    }
    this->loc_position = m_progBasic->getAttribLocation("position");
    this->loc_mvp = m_progBasic->getUniformLocation("mvp");
    this->loc_useColor = m_progBasic->getUniformLocation("useColor");
    this->loc_fixedColor = m_progBasic->getUniformLocation("fixedColor");
    this->loc_useStipple = m_progBasic->getUniformLocation("useStipple");
    //gceContext::log_message("gceTechBasic::init {}ms", sw.Time());

    setVAO();
}

void gceTechBasic::prepareScene(gceContext &ctx, const glm::dmat4 &proj, const geom::aabb2 &aoi, const glm::ivec2 &res)
{
    if (!this->id_model.is_nil())
    {
        umodelQueryMesh2DMsg msg;
        msg.id_model = this->id_model;
        msg.sender = gce::queueId::RENDER;
        ctx.postModelQueue(msg);
    }
    collectBatches(proj);
}

void gceTechBasic::collectBatches(const glm::dmat4 &proj)
{
    m_batches.clear();
    for (auto id : m_tree)
    {
        auto &i = m_instances[id];
        auto &mesh = m_meshes[i.id_mesh];
        glm::dmat4 mvp = proj * i.modelMat;
        for (auto &dc : mesh.drawCalls)
        {
            tech_basic::BatchGL b;
            b.mvp = mvp;
            b.array = dc.array;
            b.elements = dc.elements;
            b.mode = dc.mode;
            b.first = dc.first;
            b.count = dc.count;

            b.useStipple = dc.useStipple;
            b.stipplePattern = dc.stipplePattern;
            b.stippleFactor = dc.stippleFactor;

            b.invertColor = dc.invertColor;

            b.pointSize = dc.pointSize;

            b.useColor = dc.useColor;
            b.color = dc.color;

            m_batches.push_back(b);
        }
    }
    // TODO: sort/optimize batches
}

void gceTechBasic::renderScene(const glm::dmat4 &proj, const geom::aabb2 &aoi, const glm::ivec2 &res)
{
    //glEnable(GL_DEPTH_TEST);
    m_progBasic->Begin();
    glBindVertexArray(m_vao.name());
    for (auto &b : m_batches)
    {
        m_progBasic->setValue(loc_mvp, b.mvp);

        if (b.invertColor)
        {
            glEnable(GL_BLEND);
            glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ZERO);
        }
        else
        {
            glDisable(GL_BLEND);
        }

        m_progBasic->setValue(loc_useColor, b.useColor);
        if (b.useColor)
        {
            m_progBasic->setValue(loc_fixedColor, b.color);
        }


        m_progBasic->setValue(loc_useStipple, b.useStipple);
        //if (b.useStipple) {

        //}
        if (b.mode == GL_POINTS)
        {
            glPointSize(b.pointSize);
        }

        if (b.mode == GL_LINE_STRIP)
        {
            //glLineWidth(b.lineWidth);
        }

        //glBindVertexArray(b.vao);
        glBindVertexBuffer(0, b.array, 0, sizeof(tech_basic::ArrayVertex));

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
    glDisable(GL_BLEND);
    m_progBasic->End();
}

void gceTechBasic::processMsg(const urenderTechDataMsg &msg)
{
    if (msg.id_model == id_model && msg.techType == gceTechType::BASIC)
    {
        processData(static_cast<const tech_basic::DataMsg &>(msg));
    }
}
//#include <fmt/ranges.h>
void gceTechBasic::processData(const tech_basic::DataMsg &msg)
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
        m_tree.clear();
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

void gceTechBasic::setArray(const tech_basic::ArrayInfo &info)
{
    glNamedBufferData(m_arrays[info.id_array].name(), sizeof(tech_basic::ArrayVertex) * info.vertexes.size(), info.vertexes.data(), GL_STATIC_DRAW);
}

void gceTechBasic::setIndex(const tech_basic::IndexInfo &info)
{
    glNamedBufferData(m_indexes[info.id_index].name(), sizeof(tech_basic::IndexInfo::index_type) * info.indexes.size(), info.indexes.data(), GL_STATIC_DRAW);
}

void gceTechBasic::setMesh(const tech_basic::MeshInfo &info)
{
    using Vertex = tech_basic::ArrayVertex;

    auto &meshGL = m_meshes[info.id_mesh];

    meshGL.drawCalls.clear();
    meshGL.drawCalls.reserve(info.drawCalls.size());
    for (auto &udc : info.drawCalls)
    {
        auto &pdc = meshGL.drawCalls.emplace_back();
        pdc.mode = toOpenGL(udc.mode);
        pdc.first = udc.first;
        pdc.count = udc.count;

        pdc.useStipple = udc.useStipple;
        pdc.stipplePattern = udc.stipplePattern;
        pdc.stippleFactor = udc.stippleFactor;

        pdc.invertColor = udc.invertColor;

        pdc.pointSize = udc.pointSize;

        pdc.useColor = udc.useCoor;
        pdc.color = udc.color;

        pdc.array = m_arrays[udc.id_array].name();
        if (udc.id_index > 0)
        {
            pdc.elements = m_indexes[udc.id_index].name();
        }
    }
}

void gceTechBasic::setInstance(const tech_basic::InstanceInfo &info)
{
    auto &i = m_instances[info.id_instance];
    i.id_mesh = info.id_mesh;
    i.modelMat = info.modelMat;
    m_tree.insert(info.id_instance);
}

void gceTechBasic::setVAO()
{
    glVertexArrayAttribFormat(m_vao.name(), loc_position, 3, GL_FLOAT, GL_FALSE, offsetof(tech_basic::ArrayVertex, pos));
    glVertexArrayAttribBinding(m_vao.name(), loc_position, 0);
    glEnableVertexArrayAttrib(m_vao.name(), loc_position);
}
