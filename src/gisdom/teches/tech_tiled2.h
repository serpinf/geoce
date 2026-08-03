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

namespace tech_tiled2
{
struct ArrayVertex
{
    glm::fvec2 pos;
};

struct BatchGL
{
    glm::mat4 mvp;
    glm::vec4 texbox;
    GLuint tex;
};

}


class gceTechTiled2 final : public gceTechFlat
{
public:
    using vertex_t = tech_tiled2::ArrayVertex;

    explicit gceTechTiled2(gceStorageGL &storage) : gceTechFlat(gceTechType::TILED2), m_storage(storage)
    {
        init();
    }
    void init();
    void prepareScene(gceContext &ctx, const glm::dmat4 &proj, const geom::aabb2 &aoi, const glm::ivec2 &res) override;
    void renderScene(const glm::dmat4 &proj, const geom::aabb2 &aoi, const glm::ivec2 &res) override;

    void processMsg(const urenderTechDataMsg &msg) final;
private:

    void setArray();

    void setVAO();

    gceStorageGL &m_storage;
    gl::HProgram m_progBasic;

    BufferGL m_array;
    VertexArrayGL m_vao;

    std::vector<tech_tiled2::BatchGL> m_batches;

    GLint loc_position;
    GLint loc_mvp;
    GLint loc_tex;
    GLint loc_texbox;
};

