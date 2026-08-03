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
#include "geometric.h"
#include "teches/techbasic.h"
#include "geom/CoordSeq.h"
#include "geom/LineString.h"
#include "geom/Polygon.h"
#include "alg/earcut.hpp"
#include "type/coretypes.h"

static constexpr char geometry_svg[] = R"rawsvg(< svg xmlns = "http://www.w3.org/2000/svg" width = "24" height = "24" viewBox = "0 0 24 24" > <title>Geometry SVG Icon< / title><path fill = "none" stroke = "currentColor" stroke - linecap = "round" stroke - linejoin = "round" stroke - width = "2" d = "m7 21l4-12m2 0l1.48 4.439m.949 2.847L17 21M10 7a2 2 0 1 0 4 0a2 2 0 1 0-4 0m-6 5c1.526 2.955 4.588 5 8 5c3.41 0 6.473-2.048 8-5m-8-7V3" / >< / svg>)rawsvg";

gceModelGeometric::gceModelGeometric(const gce::model_info &info) : gceModelTableBased(info)
{
    if (auto it = info.tableInputs.find("Geometry"); it != info.tableInputs.end())
    {
        this->id_table = it->second.id_table;
    }
    //this->id_table = id_table;
    m_geoProj = gceProjection::create(gceProjectionType::WGS84_VISUALIZATION, {});

    // reserve zero index mesh
    uint32_t zeroIndex;
    m_data.newX(zeroIndex);
}

void gceModelGeometric::processSelectResult(gceContext &, const udataSelectReplyMsg &msg)
{
    if (msg.id_table == this->id_table)
    {
        for (auto &en : msg.rows)
        {
            uint32_t id;
            (void)addEntity(en, id);
        }
    }
    meshSent = false;
}

void gceModelGeometric::processActionNotify(gceContext &ctx, const udataMultiRowActionNotifyMsg &msg)
{
    for (auto &action : msg.m_actions)
    {
        if (action.id_table == this->id_table)
        {
            if (action.query == gceActionType::Insert)
            {
                uint32_t id;
                if (addEntity(action.newEntity, id))
                {
                    postMesh(ctx, id);
                }
            }
            else if (action.query == gceActionType::Update)
            {
                this->updateEntity(ctx, action.newEntity);
            }
            else if (action.query == gceActionType::Delete)
            {
                this->removeEntity(ctx, action.oldEntity);
            }
        }
    }
    //ctx.log().message("gceModelGeometric::processActionNotify");
}

bool gceModelGeometric::addEntity(const gceEntityPacked &en, uint32_t &index)
{
    const gceTypeSchema *schema = en.get_schema();
    uint8_t keyIndex = schema->getKeyIndex();
    auto key = en.get_arithmetic<int64_t>(keyIndex);
    if (m_data.newX(index))
    {
        m_index[key] = index;

        return setEntry(index, en); // TODO: clear index if setEntry failed ?
    }
    gceContext::log_message("ModelGeometric: data overflow");
    return false;
}
bool gceModelGeometric::setEntry(uint32_t index, const gceEntityPacked &ref)
{
    if (entry_type *pEntry = m_data.getX(index); pEntry != nullptr)
    {
        if (pEntry->g = ref.get_geometry(); pEntry->g)
        {
            project_filter pf(this->m_geoProj.get());
            pEntry->g->apply_filter_rw(pf);
            return true;
        }
    }
    return false;
}

bool gceModelGeometric::updateEntity(gceContext &ctx, const gceEntityPacked &en)
{
    const gceTypeSchema *schema = en.get_schema();
    uint8_t keyIndex = schema->getKeyIndex();
    //if (schema->getKeyInfo().type != gceColumnType::int64)
    //{
      //  return false;
    //}

    //
    auto key = en.get_arithmetic<int64_t>(keyIndex);
    if (auto it = this->m_index.find(key); it != this->m_index.end())
    {
        if (setEntry(it->second, en))
        {
            this->postMesh(ctx, it->second);
        }
    }
    return true;
}

bool gceModelGeometric::removeEntity(gceContext &ctx, const gceEntityPacked &en)
{
    const gceTypeSchema *schema = en.get_schema();
    uint8_t keyIndex = schema->getKeyIndex();
    //if (schema->getKeyInfo().type != gceColumnType::int64)
    //{
      //  return false;
    //}

    //
    auto key = en.get_arithmetic<int64_t>(keyIndex);
    if (auto it = this->m_index.find(key); it != this->m_index.end())
    {
        this->postRemoveMesh(ctx, it->second);
        this->m_data.delX(it->second);
        m_index.erase(it);
        return true;
    }
    return false;
}

void gceModelGeometric::queryMesh2D(gceContext &ctx, gce::tileid)
{
    // TODO: control if mesh is sent based on every layer id/client data version
    if (meshSent)
    {
        return;
    }
    meshSent = true;

    for (auto &[key, index] : m_index)
    {
        postMesh(ctx, index);
    }
    //ctx.log().message("gceModelGeometric::queryMesh2D");
}
static void place_select_result_with_limit(std::vector<SelectedInfo> &data, gceEntitySetKey key, double d, size_t limit)
{
    auto it = std::find_if(data.begin(), data.end(), [d](const SelectedInfo &val){return val.radius > d; });
    data.emplace(it, key, d);
    while (data.size() > limit)
    {
        data.pop_back();
    }
}
void gceModelGeometric::processSelect2D(const umodelSelect2DMsg &msg, umodelSelectXDResultMsg &rmsg)
{
    auto pos = glm::dvec2(msg.aoi);
    for (auto &[key, index] : m_index)
    {
        if (auto *entry = m_data.getX(index); entry->g)
        {
            double d = entry->g->distance(pos);
            if (d <= msg.aoi.z)
            {
                place_select_result_with_limit(rmsg.data, {id_table, key}, d, msg.limit);
            }
        }
    }
}
namespace
{
struct LineCallsCollector final : public  geom::GeometryFilter
{
    void operator()(const geom::Point &) final
    {
        ++vertexOffset;
    }
    void operator()(const geom::LineString &g) final
    {
        process(g.getCoordSeq());
    }
    void operator()(const geom::Polygon &poly) final
    {
        process(poly.m_shell);
        for (auto &ring : poly.m_holes)
        {
            process(ring);
        }
    }

    void process(const geom::CoordinateSeq &seq)
    {
        uint32_t numPoints = seq.size();
        lineStrips.emplace_back(vertexOffset, numPoints);
        vertexOffset += numPoints;
    }

    // put calls LineStrings and Rings of the Polynons here
    std::vector<std::pair<size_t, size_t>> lineStrips;

    uint32_t vertexOffset = 0;
};
struct PolyIndexer
{
    // The number type to use for tessellation
    using Coord = double;
    // Create array
    using Point = std::array<Coord, 2>;

    static void makeIndexRing(const geom::CoordinateSeq &seq, std::vector<std::vector<Point>> &poly)
    {
        std::vector<Point> &ring = poly.emplace_back();
        size_t N = seq.size();
        ring.reserve(N);
        for (size_t n = 0; n < N; n++)
        {
            geom::CoordinateXY c;
            seq.get(c, n);
            ring.push_back({c.pos.x, c.pos.y});
        }
    }
    static std::vector<uint32_t> makeIndex(const geom::Polygon &gpoly)
    {
        std::vector<std::vector<Point>> poly;

        poly.reserve(gpoly.m_holes.size() + 1);

        // Fill polygon structure with actual data. Any winding order works.
        // The first polyline defines the main polygon.
        // Following polylines define holes.
        makeIndexRing(gpoly.m_shell, poly);

        for (auto &gring : gpoly.m_holes)
        {
            makeIndexRing(gring, poly);
        }

        // Run tessellation
        // Returns array of indices that refer to the vertices of the input polygon.
        // e.g: the index 6 would refer to {25, 75} in this example.
        // Three subsequent indices form a triangle. Output triangles are clockwise.
        return mapbox::earcut<uint32_t>(poly);
    }
};


struct PolyIndexCollector final : public  geom::GeometryFilter
{
    PolyIndexCollector(std::vector<uint32_t> &indexes) : indexes(indexes) {}

    void operator()(const geom::Point &) final
    {
        ++vertexOffset;
    }

    void operator()(const geom::LineString &g) final
    {
        vertexOffset += g.getNumPoints();
    }

    void operator()(const geom::Polygon &poly) final
    {
        auto indexArr = PolyIndexer::makeIndex(poly);
        if (vertexOffset > 0)
        {
            for (auto &idx : indexArr)
            {
                idx += vertexOffset;
            }
        }
        indexes.insert(indexes.end(), indexArr.begin(), indexArr.end());
        vertexOffset += poly.getNumPoints();
    }

    // put calls LineStrings and Rings of the Polynons here
    std::vector<uint32_t> &indexes;

    uint32_t vertexOffset = 0;
};
}

void gceModelGeometric::postMesh(gceContext &ctx, int index)
{
    auto *entry = m_data.getX(index);

    if (entry == nullptr || !entry->g) return;

    const geom::Geometry *g = entry->g.get();

    size_t indexSize = 0;

    auto msg = std::make_shared<tech_basic::DataMsg>();
    msg->id_model = this->id_model;

    // post arrays (index for polygons)
    auto &array = msg->arrays.emplace_back();
    array.id_array = index;
    array.vertexes.reserve(g->getNumPoints());

    tech_basic::CollectArrayData_filter f(array);
    g->apply_filter_ro(f);

    if (g->getDimension() == 2)
    {
        auto &indexes = msg->indexes.emplace_back();
        indexes.id_index = index;
        PolyIndexCollector pic(indexes.indexes);
        g->apply_geometry_filter(pic);
        indexSize = indexes.indexes.size();
    }

    // post mesh
    auto &mesh = msg->meshes.emplace_back();
    mesh.id_mesh = index;

    if (g->getDimension() == 0)
    {
        auto &dc = mesh.drawCalls.emplace_back();
        dc.id_array = index;
        dc.mode = gcePimitiveType::POINTS;
        dc.first = 0;
        dc.count = (int32_t)g->getNumPoints();
        //dc.useCoor = true;
        dc.pointSize = 4.0f;
        //dc.color = { 0.0f, 0.0f, 0.7f, 1.0f };
    }
    else
    {
        LineCallsCollector lcc;
        lcc.lineStrips.reserve(g->getNumGeometries());
        g->apply_geometry_filter(lcc);
        mesh.drawCalls.reserve(lcc.lineStrips.size());
        for (auto &strip : lcc.lineStrips)
        {
            auto &dc = mesh.drawCalls.emplace_back();
            dc.id_array = index;
            dc.mode = gcePimitiveType::LINE_STRIP;
            dc.first = strip.first;
            dc.count = strip.second;
            //dc.useCoor = true;
            //dc.color = entry->color;
        }
    }

    if (g->getDimension() == 2)
    {
        // draw triangulated
        auto &dc = mesh.drawCalls.emplace_back();
        dc.id_array = index;
        dc.id_index = index;
        dc.mode = gcePimitiveType::TRIANGLES;
        dc.first = 0;
        dc.count = indexSize;
        //dc.useCoor = true;
        //dc.color = entry->color;
    }

    // post instance info
    auto &inst = msg->instances.emplace_back();
    inst.id_instance = index;
    inst.id_mesh = index;
    //inst.modelMat = glm::identity<glm::dmat4>();
    ctx.postRenderQueue(std::static_pointer_cast<urenderTechDataMsg>(std::move(msg)));

}

void gceModelGeometric::postRemoveMesh(gceContext &ctx, uint32_t index)
{
    auto msg = std::make_shared<tech_basic::DataMsg>();
    //msg->sender = &ctx.modelQueue;
    msg->id_model = this->id_model;
    auto &i = msg->instances.emplace_back();
    i.id_instance = index;
    i.id_mesh = 0;
    //i.modelMat = glm::identity<glm::dmat4>();
    ctx.postRenderQueue(std::static_pointer_cast<urenderTechDataMsg>(std::move(msg)));
}

std::unique_ptr<const gceModelSchema> gceModelGeometric::schema()
{
    auto schema = std::make_unique<gceModelSchema>();
    schema->m_name = "Geometric";
    schema->m_svgIcon = geometry_svg;
    schema->m_factory = [](const gce::model_info &info)->std::unique_ptr<gceModelBase>{return std::make_unique<gceModelGeometric>(info); };
    schema->m_tableInputs.emplace_back("Geometry", gce::geometry_schema_id);
    schema->m_techFlat = gceTechType::BASIC;
    return schema;
}
