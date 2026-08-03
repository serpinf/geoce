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
#include "techbasicrte2.h"
#include "node/umodelnode.h"


void gceTechBasicRTE2::init()
{
    //wxStopWatch sw;
    m_progBasic = m_storage.progs.FindOrCreate("basicrte2");
    if (!m_progBasic->IsOk())
    {
        gl::ProgramInfo info;
        info.addShaderFile(GL_VERTEX_SHADER, "prog/basicrte2.vert");
        info.addShaderFile(GL_FRAGMENT_SHADER, "prog/basicrte2.frag");

        gceContext::log_error("prog/basicrte2 load {}", m_progBasic->Create(info));
    }
    this->loc_position = m_progBasic->getAttribLocation("position");
    this->loc_mvp = m_progBasic->getUniformLocation("mvp");
    this->loc_offset = m_progBasic->getUniformLocation("offset");
    this->loc_useColor = m_progBasic->getUniformLocation("useColor");
    this->loc_fixedColor = m_progBasic->getUniformLocation("fixedColor");
    this->loc_useStipple = m_progBasic->getUniformLocation("useStipple");
    //gceContext::log_message("gceTechBasic::init {}ms", sw.Time());
}

void gceTechBasicRTE2::prepareScene(gceContext &, const glm::dmat4 &, const geom::aabb2 &, const glm::ivec2 &)
{}

void gceTechBasicRTE2::collectBatches(const glm::dmat4 &proj)
{
    m_batches.clear();
    for (auto id : m_tree)
    {
        auto &i = m_instances[id];
        auto &mesh = m_meshes[i.id_mesh];
        glm::dmat4 mvp = proj * i.modelMat;
        glm::dvec2 offset{mvp[3][0] / mvp[0][0], mvp[3][1] / mvp[1][1]};
        mvp[3][0] = 0.0;
        mvp[3][1] = 0.0;
        //gceContext::log_message("{}, {}", offset.x, offset.y);
        for (auto &dc : mesh.drawCalls)
        {
            tech_basicrte2::BatchGL b;
            b.mvp = mvp;
            b.offset = offset;
            b.vao = dc.vao.name();
            b.useDrawElements = dc.useDrawElements;
            b.mode = dc.mode;
            b.first = dc.first;
            b.count = dc.count;

            b.useStipple = dc.useStipple;
            b.stipplePattern = dc.stipplePattern;
            b.stippleFactor = dc.stippleFactor;

            b.invertColor = dc.invertColor;

            b.pointSize = dc.pointSize;
            b.lineWidth = dc.lineWidth;

            b.useColor = dc.useColor;
            b.color = dc.color;

            m_batches.push_back(b);
        }
    }
    // TODO: sort/optimize batches
}

void gceTechBasicRTE2::renderScene(const glm::dmat4 &proj, const geom::aabb2 &aoi, const glm::ivec2 &res)
{
    collectBatches(proj);

    m_progBasic->Begin();
    for (auto &b : m_batches)
    {
        m_progBasic->setValue(loc_mvp, b.mvp);
        m_progBasic->setValue(loc_offset, b.offset);

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

        glBindVertexArray(b.vao);
        if (b.useDrawElements)
        {
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

void gceTechBasicRTE2::processMsg(const urenderTechDataMsg &msg)
{
    if (msg.id_model == id_model && msg.techType == gceTechType::BASIC_RTE2)
    {
        processData(static_cast<const tech_basicrte2::DataMsg &>(msg));
    }
}
#include <fmt/ranges.h>
void gceTechBasicRTE2::processData(const tech_basicrte2::DataMsg &msg)
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

void gceTechBasicRTE2::setArray(const tech_basicrte2::ArrayInfo &info)
{
    glBindBuffer(GL_ARRAY_BUFFER, m_arrays[info.id_array].name());
    glBufferData(GL_ARRAY_BUFFER, sizeof(tech_basicrte2::ArrayVertex) * info.vertexes.size(), info.vertexes.data(), GL_STATIC_DRAW);
}

void gceTechBasicRTE2::setIndex(const tech_basicrte2::IndexInfo &info)
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexes[info.id_index].name());
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(tech_basicrte2::IndexInfo::index_type) * info.indexes.size(), info.indexes.data(), GL_STATIC_DRAW);
}

void gceTechBasicRTE2::setMesh(const tech_basicrte2::MeshInfo &info)
{
    //using Vertex = BasicArrayVertexGL;
    using Vertex = tech_basicrte2::ArrayVertex;

    auto &meshGL = m_meshes[info.id_mesh];

    meshGL.drawCalls.clear();
    meshGL.drawCalls.reserve(info.drawCalls.size());
    for (auto &udc : info.drawCalls)
    {
        auto &pdc = meshGL.drawCalls.emplace_back();
        pdc.mode = toOpenGL(udc.mode);
        pdc.useDrawElements = (udc.id_index != 0);
        pdc.first = udc.first;
        pdc.count = udc.count;

        pdc.useStipple = udc.useStipple;
        pdc.stipplePattern = udc.stipplePattern;
        pdc.stippleFactor = udc.stippleFactor;

        pdc.invertColor = udc.invertColor;

        pdc.pointSize = udc.pointSize;
        pdc.lineWidth = udc.lineWidth;

        pdc.useColor = udc.useCoor;
        pdc.color = udc.color;

        glBindVertexArray(pdc.vao.name());

        glBindBuffer(GL_ARRAY_BUFFER, m_arrays[udc.id_array].name());
        glVertexAttribLPointer(loc_position, 2, GL_DOUBLE, sizeof(Vertex), (GLvoid *)offsetof(Vertex, pos));
        glEnableVertexAttribArray(loc_position);

        if (pdc.useDrawElements)
        {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexes[udc.id_index].name());
        }

        glBindVertexArray(0);
    }
}

void gceTechBasicRTE2::setInstance(const tech_basicrte2::InstanceInfo &info)
{
    auto &i = m_instances[info.id_instance];
    i.id_mesh = info.id_mesh;
    i.modelMat = info.modelMat;
    m_tree.insert(info.id_instance);
}

