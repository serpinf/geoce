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

namespace tech_tiled3
{
struct QTreeNode;
class QTreeData;

struct ArrayVertex
{
    glm::fvec3 pos;
    glm::fvec3 normal;
};

struct QPatch
{
    ArrayVertex array[gce::sizep][gce::sizep];
};

struct DataMsg : public urenderTechDataMsg
{
    DataMsg() : urenderTechDataMsg(gceTechType::TILED3) {}
};

struct BatchGL
{
    glm::mat4 mvp;
    glm::dvec3 origin;
    glm::vec4 texbox;
    GLuint array;
    GLuint tex;
};
}


class gceTechTiled3 final : public gceTechGlobe
{
public:
    ~gceTechTiled3() override;
    using vertex_t = tech_tiled3::ArrayVertex;

    explicit gceTechTiled3(gceStorageGL &storage);
    void init();
    void prepareScene(gceContext &ctx, const glm::dmat4 &proj, const glm::dvec3 &pos, const glm::ivec2 &res) override;
    void ensureTreeDataInit(gceContext &ctx);
    void renderScene(const glm::dmat4 &proj, const glm::dvec3 &pos, const glm::ivec2 &res, bool wireFrameTerrain) override;

    void processMsg(const urenderTechDataMsg &msg) final;
private:

    void setTexposArray();

    void setElements();

    void setVAO();

    gceStorageGL &m_storage;
    gl::HProgram m_progBasic;

    BufferGL m_elements;
    BufferGL m_texcoords;
    VertexArrayGL m_vao;

    std::vector<tech_tiled3::BatchGL> m_batches;

    GLint loc_position;
    GLint loc_normal;
    GLint loc_texp;
    GLint loc_mvp;
    GLint loc_tex;
    GLint loc_texbox;

    std::unique_ptr<tech_tiled3::QTreeNode> m_tree;
    std::unique_ptr<tech_tiled3::QTreeData> m_treeData;
};

