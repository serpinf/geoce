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
#include "imagetile.h"
#include <stb_image.h>
#include "type/coretypes.h"

static constexpr char raster_svg[] = R"rawsvg(<svg xmlns="http://www.w3.org/2000/svg" width="200" height="200" viewBox="0 0 20 20"><path fill="currentColor" d="M0 0h9v9H0V0zm2 2v5h5V2H2zm-2 9h9v9H0v-9zm2 2v5h5v-5H2zm9-13h9v9h-9V0zm2 2v5h5V2h-5zm-2 9h9v9h-9v-9zm2 2v5h5v-5h-5z"/></svg>)rawsvg";

namespace
{
bool unpack_image(const gce::span<const std::byte> &buf, urenderTexData &image)
{
    int x = 0, y = 0, comp = 0;
    if (auto *ptr = stbi_load_from_memory((const stbi_uc *)buf.data(), buf.size(), &x, &y, &comp, 4); ptr != nullptr)
    {
        image.c = 4;
        image.w = x;
        image.h = y;
        image.data.assign(ptr, ptr + x * y * 4);
        stbi_image_free(ptr);
        return true;
    }
    return false;
}
#include "alg/wgsop.h"
static geom::Box2D flat_box(gce::tileid tile)
{
    double scale = std::ldexp(wgsop::M_2PI, -tile.get_z());
    double xmin = -M_PI + scale * tile.get_x();
    double ymax = M_PI - scale * tile.get_y();
    return geom::Box2D{glm::dvec2(xmin, ymax - scale), glm::dvec2(xmin + scale, ymax)};
}

// dem is a pointer to float array of size gce::sizep * gce::sizep, containing height values for the tile
static void initMesh(urenderTileDEMDataMsg &msg, gce::tileid tile, const float *dem)
{
    auto box2 = flat_box(tile);
    wgsop::xyz_grid<gce::sizep, gce::sizep, wgsop::toWGS_fromflat> AC(
        box2.cmin.x,
        box2.cmin.y,
        box2.cmax.x,
        box2.cmax.y
    );

    glm::dvec3 array[gce::sizep][gce::sizep];
    geom::Box3D box3;
    float hmax = -10000.0f;
    for (int n = 0; n < gce::sizep; ++n)
    {
        for (int m = 0; m < gce::sizep; ++m)
        {
            float h = dem != nullptr ? dem[(gce::sizep - 1 - n) * gce::sizep + m] : 1.0;
            hmax = std::max(hmax, h);
            const glm::dvec3 pos = AC.xyz_at2i(n, m, h);
            // target vertex
            array[m][n] = pos;
            box3.expand(pos);
        }
    }
    msg.hmax = hmax;
    msg.bbox = geom::psAABB{box3};
    const glm::dvec3 origin = msg.bbox.cen;


    //tex_grid<sizep, sizep> AT(box2);

    msg.patch = std::make_unique<gceQPatch>();

    //dvec3 pos;
    for (int n = 0; n < gce::sizep; ++n)
    {
        for (int m = 0; m < gce::sizep; ++m)
        {
            const glm::dvec3 &pos = array[m][n];
            // target vertex
            auto &vv = msg.patch->array[m][n];
            // source vertex for coarse LOD: current or previous
            //const psDVertex &vv_src = qp.array[m & (~1)][n & (~1)];

            // position
            vv.pos = pos - origin;

            vv.normal = glm::normalize(glm::vec3(pos));

            //vv.coo2 = vv_src.coo1;

            // texture coord
            //vv.coo4[0] = AT.uarr[m];
            //vv.coo4[1] = AT.varr[n];

            //vv.coo4[2] = vv_src.coo4[0];
            //vv.coo4[3] = vv_src.coo4[1];

            // up
            //vv.coo5[0] = AC.lona[m].cos_lon * AC.lata[n].cos_lat;
            //vv.coo5[1] = AC.lona[m].sin_lon * AC.lata[n].cos_lat;
            //vv.coo5[2] = AC.lata[n].sin_lat;

            //vv.coo6 = vv_src.coo5;
        }
    }
}

};

gceModelImageTiled::gceModelImageTiled(const gce::model_info &info) : gceModelTableBased(info)
{
    if (auto it = info.tableInputs.find("Imagery"); it != info.tableInputs.end())
    {
        this->id_table = it->second.id_table;
    }

    if (auto it = info.tableInputs.find("Terrain"); it != info.tableInputs.end())
    {
        this->id_tableDEM = it->second.id_table;
    }
}

void gceModelImageTiled::processSelectResult(gceContext &ctx, const udataSelectReplyMsg &msg)
{
    // gceContext::log_message("gceModelImageTiled::processSelectResult");
    if (msg.id_table == id_table)
    {
        for (auto &row : msg.rows)
        {
            postTileImage(row, ctx);
        }
    }

    else if (msg.id_table == id_tableDEM)
    {
        for (auto &row : msg.rows)
        {
            postTileDEM(row, ctx);
        }
    }
}

void gceModelImageTiled::postTileImage(const gceEntityPacked &row, gceContext &ctx)
{
    if (row.get_schema()->getId() == gce::raster_schema_id)
    {
        urenderTileImageDataMsg msg;
        msg.id_model = this->id_model;
        msg.tile = gce::tileid(uint64_t(row.get_arithmetic<int64_t>(0)));
        m_queries.erase(msg.tile);

        if (unpack_image(row.get_data(2), msg.tex))
        {
            ctx.postRenderQueue(std::move(msg));
        }
    }
}

void gceModelImageTiled::postTileError(const gce::tileid tile, gceContext &ctx)
{
    urenderTileImageDataMsg msg;
    msg.id_model = this->id_model;
    msg.tile = tile;
    msg.modelOK = false;
    ctx.postRenderQueue(std::move(msg));
}

void gceModelImageTiled::postTileDEM(const gceEntityPacked &row, gceContext &ctx)
{
    if (row.get_schema()->getId() == gce::raster_schema_id)
    {
        urenderTileDEMDataMsg msg;
        msg.id_model = this->id_model;
        msg.tile = gce::tileid(uint64_t(row.get_arithmetic<int64_t>(0)));
        m_queriesDEM.erase(msg.tile);

        auto data = row.get_data(2);
        if (data.size() == gce::sizep * gce::sizep * sizeof(float))
        {
            initMesh(msg, msg.tile, (const float *)data.data());
        }
        else
        {
            // TODO: try init from lower res
            initMesh(msg, msg.tile, nullptr);
        }
        ctx.postRenderQueue(std::move(msg));
    }
}

void gceModelImageTiled::postTileDEMError(const gce::tileid tile, gceContext &ctx)
{
    urenderTileDEMDataMsg msg;
    msg.id_model = this->id_model;
    msg.tile = tile;
    msg.modelOK = false;
    ctx.postRenderQueue(std::move(msg));
}

void gceModelImageTiled::queryMesh2D(gceContext &, gce::tileid)
{}

void gceModelImageTiled::processQueryImageTile(gceContext &ctx, const umodelQueryTileImageMsg &msg)
{
    // if model not ready, do nothing
    if (!this->isOk())
    {
        postTileError(msg.tileId, ctx);
        return;
    }

    // Try to insert once. emplace returns whether insertion happened.
    auto [it, inserted] = m_queries.emplace(msg.tileId);
    if (inserted)
    {
        // If posting fails remove the previously-inserted tile so future attempts remain possible.
        if (!postQueryImage(ctx, msg.tileId))
        {
            postTileError(msg.tileId, ctx);
            m_queries.erase(it);
        }
    }
}


void gceModelImageTiled::processQueryDEMTile(gceContext &ctx, const umodelQueryTileDEMMsg &msg)
{
        // if model not ready, do nothing
    if (!this->isTableDEMOK())
    {
        postTileDEMError(msg.tileId, ctx);
        return;
    }

    //auto it = m_queries.find(msg.tileId);
    // TODO: handle the case query was sent by terrain patch query - change queries set to map and store query type flags,
    // so we can reply for image or terrain query or both.
    // Try to insert once. emplace returns whether insertion happened.
    auto [it, inserted] = m_queriesDEM.emplace(msg.tileId);
    if (inserted)
    {
        // If posting fails remove the previously-inserted tile so future attempts remain possible.
        if (!postQueryDEM(ctx, msg.tileId))
        {
            postTileDEMError(msg.tileId, ctx);
            m_queriesDEM.erase(it);
        }
    }
}

std::unique_ptr<const gceModelSchema> gceModelImageTiled::schema()
{
    auto schema = std::make_unique<gceModelSchema>();
    schema->m_name = "ImageTiled";
    schema->m_factory = [](const gce::model_info &info)->std::unique_ptr<gceModelBase>{return std::make_unique<gceModelImageTiled>(info); };
    schema->m_svgIcon = raster_svg;
    schema->m_tableInputs.emplace_back("Imagery", gce::raster_schema_id);
    schema->m_tableInputs.emplace_back("Terrain", gce::raster_schema_id);
    schema->m_techFlat = gceTechType::TILED2;
    schema->m_techGlobe = gceTechType::TILED3;
    return schema;
}

bool gceModelImageTiled::postQueryImage(gceContext &ctx, gce::tileid tile)
{
    udataSelectIDMsg dmsg;
    dmsg.key = {id_table, int64_t(tile.get_value())};
    dmsg.sender = gce::queueId::MODEL;
    return ctx.postDataQueue(dmsg, 5);
}

bool gceModelImageTiled::postQueryDEM(gceContext &ctx, gce::tileid tile)
{
    udataSelectIDMsg dmsg;
    dmsg.key = {id_tableDEM, int64_t(tile.get_value())};
    dmsg.sender = gce::queueId::MODEL;
    return ctx.postDataQueue(dmsg, 80);
}
