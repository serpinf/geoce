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
#include <set>
#include "geom/CoordinateFilter.h"

namespace tech_basic
{
struct ArrayVertex
{
    glm::fvec3 pos;
    //glm::bvec4 color;
};

struct ArrayInfo
{
    ArrayInfo(const std::vector<ArrayVertex> &vertexes, uint32_t id_array) : vertexes(vertexes), id_array(id_array) {}
    ArrayInfo() = default;

    std::vector<ArrayVertex> vertexes;
    uint32_t id_array = 0;
};

struct IndexInfo
{
    using index_type = uint32_t;
    std::vector<index_type> indexes;
    uint32_t id_index = 0;
};

struct DrawCall
{
    gcePimitiveType mode = gcePimitiveType::POINTS;
    bool useStipple = false;
    bool invertColor = false;
    bool useCoor = false;

    uint32_t id_array = 0;
    uint32_t id_index = 0;
    uint32_t first = 0;
    uint32_t count = 0;

    uint32_t stipplePattern = 0xffffffffu;
    float stippleFactor = 1.0;

    float pointSize = 1.0f;

    glm::fvec4 color{1.0f, 0.0f, 1.0f, 1.0f};
};

struct MeshInfo
{
    std::vector<DrawCall> drawCalls;
    uint32_t id_mesh = 0;
};

struct InstanceInfo
{
    InstanceInfo(const uint32_t id_instance, const uint32_t id_mesh, const glm::dmat4 &modelMat)
        : id_instance(id_instance), id_mesh(id_mesh), modelMat(modelMat)
    {}
    InstanceInfo() = default;

    uint32_t id_instance = 0;
    uint32_t id_mesh = 0;
    glm::dmat4 modelMat{1.0};
};

struct DataMsg : public urenderTechDataMsg
{
    DataMsg() : urenderTechDataMsg(gceTechType::BASIC) {}

    std::vector<tech_basic::ArrayInfo> arrays;
    std::vector<tech_basic::IndexInfo> indexes;
    std::vector<tech_basic::MeshInfo> meshes;
    std::vector<tech_basic::InstanceInfo> instances;

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

    bool useStipple = false;
    bool invertColor = false;
    bool useColor = false;

    GLuint stipplePattern = 0xffffffffu;
    float stippleFactor = 1.0;

    float pointSize = 1.0f;

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
    glm::mat4 mvp;

    GLuint array;
    GLuint elements;
    GLenum mode;
    GLint first;
    GLsizei count;

    bool useStipple;
    bool invertColor;
    bool useColor;

    GLuint stipplePattern;
    float stippleFactor;

    float pointSize;

    glm::fvec4 color;
};

class CollectArrayData_filter : public geom::filter_ro
{
public:
    explicit CollectArrayData_filter(ArrayInfo &data) : m_data(data) {}

    void operator()(const geom::Coordinate &coo) override
    {
        m_data.vertexes.push_back(ArrayVertex{coo.pos});
    }
private:
    ArrayInfo &m_data;
};

}




class gceTechBasic final : public gceTechFlat
{
public:
    explicit gceTechBasic(gceStorageGL &storage) : gceTechFlat(gceTechType::BASIC), m_storage(storage)
    {
        init();
    }
    void init();
    void prepareScene(gceContext &ctx, const glm::dmat4 &proj, const geom::aabb2 &aoi, const glm::ivec2 &res) override;
    void renderScene(const glm::dmat4 &proj, const geom::aabb2 &aoi, const glm::ivec2 &res) override;
    void collectBatches(const glm::dmat4 &proj);

    void processMsg(const urenderTechDataMsg &msg) final;
private:
    void processData(const tech_basic::DataMsg &msg);
    void setArray(const tech_basic::ArrayInfo &info);
    void setIndex(const tech_basic::IndexInfo &info);
    void setMesh(const tech_basic::MeshInfo &info);
    void setInstance(const tech_basic::InstanceInfo &info);

    void setVAO();

    gceStorageGL &m_storage;
    gl::HProgram m_progBasic;
    gce::store<BufferGL> m_arrays;
    gce::store<BufferGL> m_indexes;
    gce::store<tech_basic::MeshGL> m_meshes;
    gce::store<tech_basic::InstanceGL> m_instances;
    VertexArrayGL m_vao;

    std::set<uint32_t> m_tree;

    std::vector<tech_basic::BatchGL> m_batches;

    GLint loc_position;
    GLint loc_mvp;
    GLint loc_useColor;
    GLint loc_fixedColor;
    GLint loc_useStipple;
};

