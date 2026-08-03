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
#include "techbase.h"
#include "RendererGL.h"

namespace tech_objects3
{
struct ArrayVertex
{
    glm::fvec3 pos;
    //glm::bvec4 color;
};

struct ArrayInfo
{
    ArrayInfo(const std::vector<ArrayVertex> &vertexes, uint32_t id_array) : vertexes(vertexes), id_array(id_array) {}
    std::vector<ArrayVertex> vertexes;
    uint32_t id_array = 0;
};

struct IndexInfo
{
    using index_type = uint8_t;
    std::vector<index_type> indexes;
    uint32_t id_index = 0;
};

struct DrawCall
{
    gcePimitiveType mode = gcePimitiveType::POINTS;

    uint32_t id_array = 0;
    uint32_t id_index = 0;
    uint32_t first = 0;
    uint32_t count = 0;

    glm::fvec4 color{1.0f, 0.0f, 1.0f, 1.0f};
};

struct MeshInfo
{
    std::vector<DrawCall> drawCalls;
    uint32_t id_mesh = 0;
};

struct InstanceInfo
{
    uint32_t id_instance = 0;
    uint32_t id_mesh = 0;
    glm::dmat4 modelMat{1.0};
};

struct DataMsg : public urenderTechDataMsg
{
    DataMsg() : urenderTechDataMsg(gceTechType::OBJECTS3) {}

    std::vector<tech_objects3::ArrayInfo> arrays;
    std::vector<tech_objects3::IndexInfo> indexes;
    std::vector<tech_objects3::MeshInfo> meshes;
    std::vector<tech_objects3::InstanceInfo> instances;

    bool clearArrays = false;
    bool clearIndexes = false;
    bool clearMeshes = false;
    bool clearInstances = false;
};

struct DrawCallGL
{
    GLuint array = 0;
    GLuint elements = 0;
    GLenum mode = 0;
    GLint first = 0;
    GLsizei count = 0;
    glm::fvec4 color{1.0f};
};
struct MeshGL
{
    std::vector<DrawCallGL> drawCalls;
};

struct InstanceGL
{
    uint32_t id_mesh = 0;
    glm::dmat4 modelMat{1.0};
};

struct BatchGL
{
    glm::fmat4 mvp;

    GLuint array;
    GLuint elements;
    GLenum mode;
    GLint first;
    GLsizei count;
    glm::fvec4 color;
    float scale;
};

}


class gceTechObjects3 final : public gceTechGlobe
{
public:
    explicit gceTechObjects3(gceStorageGL &storage) : gceTechGlobe(gceTechType::OBJECTS3), m_storage(storage)
    {
        init();
    }
    void init();
    void prepareScene(gceContext &ctx, const glm::dmat4 &proj, const glm::dvec3 &pos, const glm::ivec2 &res) override;
    void renderScene(const glm::dmat4 &proj, const glm::dvec3 &pos, const glm::ivec2 &res, bool wireFrameTerrain) override;
    void collectBatches(const glm::dmat4 &proj, const glm::dvec3 &pos);

    void processMsg(const urenderTechDataMsg &msg) final;
private:
    void processData(const tech_objects3::DataMsg &msg);
    void setArray(const tech_objects3::ArrayInfo &info);
    void setIndex(const tech_objects3::IndexInfo &info);
    void setMesh(const tech_objects3::MeshInfo &info);
    void setInstance(const tech_objects3::InstanceInfo &info);
    void setVAO();

    gceStorageGL &m_storage;
    gl::HProgram m_progBasic;
    gce::store<BufferGL> m_arrays;
    gce::store<BufferGL> m_indexes;
    gce::store<tech_objects3::MeshGL> m_meshes;
    gce::store<tech_objects3::InstanceGL> m_instances;
    VertexArrayGL m_vao;

    std::vector<tech_objects3::BatchGL> m_batches;

    GLint loc_position;
    GLint loc_scale;
    GLint loc_mvp;
    GLint loc_fixedColor;
};

