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
#include "modeldemtile.h"
#include "type/coretypes.h"

static constexpr char raster_svg[] = R"rawsvg(<svg xmlns="http://www.w3.org/2000/svg" width="200" height="200" viewBox="0 0 20 20"><path fill="currentColor" d="M0 0h9v9H0V0zm2 2v5h5V2H2zm-2 9h9v9H0v-9zm2 2v5h5v-5H2zm9-13h9v9h-9V0zm2 2v5h5V2h-5zm-2 9h9v9h-9v-9zm2 2v5h5v-5h-5z"/></svg>)rawsvg";

namespace
{
bool unpack_image(const gce::span<const std::byte> &buf, urenderTexData &image)
{
    if (buf.size() != gce::sizep * gce::sizep * sizeof(float))
    {
        return false;
    }
    float *data = (float *)buf.data();

    int x = 256, y = 256;
    image.c = 4;
    image.w = x;
    image.h = y;
    image.data.reserve(x * y * 4);
    int s = x / (gce::sizep - 1);
    for (int j = 0; j < y; ++j)
    {
        int cj = std::clamp(int(double(j) / (y - 1) * (gce::sizep - 1)), 0, gce::sizep - 1);
        for (int i = 0; i < x; ++i)
        {
            int ci = std::clamp(int(double(i) / (x - 1) * (gce::sizep - 1)), 0, gce::sizep - 1);
            float val = data[cj * gce::sizep + ci] / 20.0;
            uint8_t uval = uint8_t(std::clamp(val, 0.0f, 255.0f));
            image.data.push_back(uval);
            image.data.push_back(uval);
            image.data.push_back(uval);
            image.data.push_back(255);
        }
    }
    return true;
}
static geom::Box2D flat_box(gce::tileid tile)
{
    double scale = std::ldexp(1.0, -tile.get_z());
    double xmin = -M_PI + scale * tile.get_x();
    double ymax = M_PI - scale * tile.get_y();
    return geom::Box2D{glm::dvec2(xmin, ymax - scale), glm::dvec2(xmin + scale, ymax)};
}
};

gceModelDEMTile::gceModelDEMTile(const gce::model_info &info) : gceModelTableBased(info)
{
    if (auto it = info.tableInputs.find("Terrain"); it != info.tableInputs.end())
    {
        this->id_table = it->second.id_table;
    }
}

void gceModelDEMTile::processSelectResult(gceContext &ctx, const udataSelectReplyMsg &msg)
{
    // gceContext::log_message("gceModelDEMTile::processSelectResult");
    if (msg.id_table == id_table)
    {
        for (auto &row : msg.rows)
        {
            postTileImage(row, ctx);
        }
    }
}

void gceModelDEMTile::postTileImage(const gceEntityPacked &row, gceContext &ctx)
{
    if (row.get_schema()->getId() == gce::raster_schema_id)
    {
        urenderTileImageDataMsg msg;
        msg.id_model = this->id_model;
        msg.tile = gce::tileid(uint64_t(row.get_arithmetic<int64_t>(0)));

        if (unpack_image(row.get_data(2), msg.tex))
        {
            m_queries.erase(msg.tile);
            ctx.postRenderQueue(std::move(msg));
        }
    }
}

void gceModelDEMTile::postTileImageError(const gce::tileid tile, gceContext &ctx)
{
    urenderTileImageDataMsg msg;
    msg.id_model = this->id_model;
    msg.tile = tile;
    msg.modelOK = false;
    ctx.postRenderQueue(std::move(msg));
}

void gceModelDEMTile::queryMesh2D(gceContext &, gce::tileid)
{}

void gceModelDEMTile::processQueryImageTile(gceContext &ctx, const umodelQueryTileImageMsg &msg)
{
    // if model not ready, do nothing
    if (!this->isOk())
    {
        postTileImageError(msg.tileId, ctx);
        return;
    }

    //auto it = m_queries.find(msg.tileId);
    // TODO: handle the case query was sent by terrain pach query - change queries set to map and store query type flags,
    // so we can reply for image or terrain query or both.
    // Try to insert once. emplace returns whether insertion happened.
    auto [it, inserted] = m_queries.emplace(msg.tileId);
    if (inserted)
    {
        // If posting fails remove the previously-inserted tile so future attempts remain possible.
        if (!postQueryDEM(ctx, msg.tileId))
        {
            postTileImageError(msg.tileId, ctx);
            m_queries.erase(it);
        }
    }
}

std::unique_ptr<const gceModelSchema> gceModelDEMTile::schema()
{
    auto schema = std::make_unique<gceModelSchema>();
    schema->m_name = "DEMTiled";
    schema->m_factory = [](const gce::model_info &info)->std::unique_ptr<gceModelBase>{return std::make_unique<gceModelDEMTile>(info); };
    schema->m_svgIcon = raster_svg;
    schema->m_tableInputs.emplace_back("Terrain", gce::raster_schema_id);
    schema->m_techFlat = gceTechType::TILED2;
    schema->m_techGlobe = gceTechType::TILED3;
    return schema;
}

bool gceModelDEMTile::postQueryDEM(gceContext &ctx, gce::tileid tile)
{
    udataSelectIDMsg dmsg;
    dmsg.key = {id_table, int64_t(tile.get_value())};
    dmsg.sender = gce::queueId::MODEL;
    return ctx.postDataQueue(dmsg, 5);
}

