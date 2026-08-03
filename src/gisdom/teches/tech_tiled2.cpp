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
#include "tech_tiled2.h"
#include <glm/gtc/matrix_transform.hpp>


void gceTechTiled2::init()
{
    //wxStopWatch sw;
    m_progBasic = m_storage.progs.FindOrCreate("textile");
    if (!m_progBasic->IsOk())
    {
        gl::ProgramInfo info;
        info.addShaderFile(GL_VERTEX_SHADER, "prog/textile.vert");
        info.addShaderFile(GL_FRAGMENT_SHADER, "prog/textile.frag");

        gceContext::log_error("prog/textile load {}", m_progBasic->Create(info));
    }
    this->loc_position = m_progBasic->getAttribLocation("position");
    this->loc_mvp = m_progBasic->getUniformLocation("mvp");
    this->loc_tex = m_progBasic->getUniformLocation("tex");
    this->loc_texbox = m_progBasic->getUniformLocation("texbox");

    setArray();
    setVAO();

    //glTextureStorage2D(m_tex.name(), 1, GL_RGBA8, 256, 256);

    //gceContext::log_message("gceTechBasic::init {}ms", sw.Time());
}
//gce::tileid part(gce::tileid tile, int n)
//{

//}
struct RenderTree
{
    geom::aabb2 aoi;
    int best_level;
    glm::dmat4 proj;
    gceTileCache &cache;
    gceContext &ctx;
    gce::uuid id_model;
    std::vector<tech_tiled2::BatchGL> &m_batches;
    //std::vector<gce::tileid> m_tileQueries;

    static glm::vec4 calc_tex_box(gce::tileid keyL, gce::tileid keyH)
    {
        // scale is <=1
        const double scale = ldexp(1.0, keyL.get_z() - keyH.get_z());

        double minx = keyH.get_x() * scale - keyL.get_x();
        double miny = keyH.get_y() * scale - keyL.get_y();

        return {minx, miny, minx + scale, miny + scale};
    }

    void collect(geom::aabb2 box, uint8_t level, uint32_t x, uint32_t y)
    {
        if (!box.overlaps(aoi)) return;

        if (level < best_level)
        {
            level++;
            x *= 2;
            y *= 2;
            collect(box.getPart2D(1), level, x, y);
            collect(box.getPart2D(2), level, x + 1, y);
            collect(box.getPart2D(3), level, x, y + 1);
            collect(box.getPart2D(4), level, x + 1, y + 1);
        }
        else
        {
            gce::tileid tile(x, y, level);
            gceTileCache::tilekey key{id_model, tile};

            if (auto it = cache.m_tree.find(key); it != cache.m_tree.end())
            {
                tech_tiled2::BatchGL b{};
                b.mvp = calc_mat(box, proj);
                b.tex = it->second.tex.name();
                b.texbox = {0.0, 0.0, 1.0, 1.0};
                m_batches.push_back(b);
            }
            else
            {
                if (cache.m_queried.find(key) == cache.m_queried.end() && cache.m_missing.find(key) == cache.m_missing.end())
                {
                    umodelQueryTileImageMsg msg;
                    msg.id_model = id_model;
                    msg.sender = gce::queueId::RENDER;
                    msg.tileId = tile;
                    ctx.postModelQueue(msg);
                    cache.m_queried.insert(key);
                }

                while (--level > 0)
                {
                    x /= 2;
                    y /= 2;
                    gce::tileid tileCoarse(x, y, level);
                    if (auto it2 = cache.m_tree.find({id_model, tileCoarse}); it2 != cache.m_tree.end())
                    {
                        tech_tiled2::BatchGL b{};
                        b.mvp = calc_mat(box, proj);
                        b.tex = it2->second.tex.name();
                        b.texbox = calc_tex_box(tileCoarse, tile);
                        m_batches.push_back(b);
                        break;
                    }
                }
            }
        }
    }
    glm::dmat4 calc_mat(const geom::aabb2 &box, const glm::dmat4 &proj)
    {
        return glm::scale(glm::translate(proj, glm::dvec3(box.cen, 0.0)), glm::dvec3(box.size, 1.0));
    }
};

void gceTechTiled2::prepareScene(gceContext &ctx, const glm::dmat4 &proj, const geom::aabb2 &aoi, const glm::ivec2 &res)
{
    constexpr geom::aabb2 prjArea{glm::dvec2(0.0), glm::dvec2(M_PI)};

    const double level_bias = 0.25;

    m_batches.clear();
    int best_level = std::min<int>(std::log2((prjArea.size.y / 256.0) / (aoi.size.y / res.y)) + level_bias, gce::tileid::max_level());

    RenderTree rt{aoi, best_level, proj, m_storage.tiles, ctx, id_model, m_batches};

    rt.collect(prjArea, 0, 0, 0);

    // TODO: sort/optimize batches
}

void gceTechTiled2::renderScene(const glm::dmat4 &proj, const geom::aabb2 &aoi, const glm::ivec2 &res)
{
    m_progBasic->Begin();
    glBindVertexArray(m_vao.name());
    for (auto &b : m_batches)
    {
        glBindTextureUnit(0, b.tex);

        m_progBasic->setValue(loc_mvp, b.mvp);
        m_progBasic->setValue(loc_texbox, b.texbox);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }
    glBindTextureUnit(0, 0);
    glBindVertexArray(0);
    m_progBasic->End();
}

void gceTechTiled2::processMsg(const urenderTechDataMsg &msg)
{}

void gceTechTiled2::setArray()
{
    const std::vector<vertex_t> vertexes{
        {{-1.0f, +1.0f}},
        {{-1.0f, -1.0f}},
        {{+1.0f, +1.0f}},
        {{+1.0f, -1.0f}}};

    glNamedBufferData(m_array.name(), sizeof(vertex_t) * vertexes.size(), vertexes.data(), GL_STATIC_DRAW);
}

void gceTechTiled2::setVAO()
{
    glVertexArrayAttribFormat(m_vao.name(), loc_position, 2, GL_FLOAT, GL_FALSE, offsetof(vertex_t, pos));
    glVertexArrayAttribBinding(m_vao.name(), loc_position, 0);
    glEnableVertexArrayAttrib(m_vao.name(), loc_position);

    glVertexArrayVertexBuffer(m_vao.name(), 0, m_array.name(), 0, sizeof(vertex_t));
}


