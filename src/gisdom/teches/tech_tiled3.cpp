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
#include "tech_tiled3.h"
#include "geom/aabb.h"
#include "idxpool.h"
#include "alg/wgsop.h"
static size_t s_nodepatchesQ = 0;
static size_t s_framesCnt = 0;

namespace
{

inline double psSqMin(const geom::psAABB &box, const glm::dvec3 &pos)
{
    glm::dvec3 d = glm::max(glm::abs(pos - box.cen) - box.size, 0.0);
    return glm::dot(d, d);
}

static double tex_sq_min(const glm::dvec3 &cam_coo_tex, const geom::Box2D &tbox)
{
    const glm::dvec2 cen(tbox.GetCenter());
    const glm::dvec2 size(0.5 * tbox.size());


    double d = fabs(cam_coo_tex[0] - cen[0]);
    d = std::min(d, 1.0 - d);

    double d1 = d - size.x;
    d1 = d1 < 0.0 ? 0.0 : d1 * d1;

    double d2 = fabs(cam_coo_tex[1] - cen[1]) - size.y;
    d2 = d2 < 0.0 ? 0.0 : d2 * d2;

    double d3 = cam_coo_tex[2] * cam_coo_tex[2];

    return(d1 + d2 + d3);
}




// Based on 12.4.3 Horizon Culling from 3D Engine Design for Virtual Globes book
// All coordinates on spheroid are scaled to R==1.0 sphere
class SpheroidOcclusionTester
{
public:
    void init(const glm::dvec3 &pos, double Ra, double Rb)
    {
        m_scale = glm::dvec3(1.0 / Ra, 1.0 / Ra, 1.0 / Rb);

        m_pos0 = pos * m_scale;
        m_vh2 = glm::dot(m_pos0, m_pos0) - 1.0;
        m_vh = sqrt(m_vh2);

    }
    bool test(const geom::psAABB &box, double height) const
    {
        // Re == 1.0, Ro == 0.0, oh = Re + height
        const geom::psAABB boxc(box.cen * m_scale, box.size * m_scale);

        double mind2 = psSqMin(boxc, m_pos0);
        if (mind2 > m_vh2)
        {
            // if object is behind the horizon test using height
            double eo = 1.0 + height * m_scale[0]; // scale is approximated
            double vo = m_vh + sqrt(eo * eo - 1.0);
            return mind2 > vo * vo;
        }
        return false;
    }
private:
    glm::dvec3 m_pos0; // viewer position
    glm::dvec3 m_scale; // general scale to transform to R=1 sphere
    double m_vh = 0.0; // viewer - horizon distance
    double m_vh2 = 0.0; // viewer - horizon distance ^2
};

}

namespace tech_tiled3
{

class CameraFrutrum
{
// Plane struct: ax + by + cz + d = 0
    struct Plane
    {
        glm::dvec3 normal;
        double d;

        // Normalize the plane for consistent distance calculations
        void normalize()
        {
            double invlength = 1.0 / glm::length(normal);
            normal *= invlength;
            d *= invlength;
        }

        // Distance from point to plane
        double distance(const glm::dvec3 &point) const
        {
            return glm::dot(normal, point) + d;
        }
    };
public:
    // Extract frustum planes from combined ModelViewProjection matrix
    void extractFrustumPlanes(const glm::dmat4 &mvp)
    {
        // Left
        planes[0].normal.x = mvp[0][3] + mvp[0][0];
        planes[0].normal.y = mvp[1][3] + mvp[1][0];
        planes[0].normal.z = mvp[2][3] + mvp[2][0];
        planes[0].d = mvp[3][3] + mvp[3][0];
        planes[0].normalize();

        // Right
        planes[1].normal.x = mvp[0][3] - mvp[0][0];
        planes[1].normal.y = mvp[1][3] - mvp[1][0];
        planes[1].normal.z = mvp[2][3] - mvp[2][0];
        planes[1].d = mvp[3][3] - mvp[3][0];
        planes[1].normalize();

        // Bottom
        planes[2].normal.x = mvp[0][3] + mvp[0][1];
        planes[2].normal.y = mvp[1][3] + mvp[1][1];
        planes[2].normal.z = mvp[2][3] + mvp[2][1];
        planes[2].d = mvp[3][3] + mvp[3][1];
        planes[2].normalize();

        // Top
        planes[3].normal.x = mvp[0][3] - mvp[0][1];
        planes[3].normal.y = mvp[1][3] - mvp[1][1];
        planes[3].normal.z = mvp[2][3] - mvp[2][1];
        planes[3].d = mvp[3][3] - mvp[3][1];
        planes[3].normalize();

        // Near
        planes[4].normal.x = mvp[0][3] + mvp[0][2];
        planes[4].normal.y = mvp[1][3] + mvp[1][2];
        planes[4].normal.z = mvp[2][3] + mvp[2][2];
        planes[4].d = mvp[3][3] + mvp[3][2];
        planes[4].normalize();

        // Far plane is unused for infinite projection
        //planes[5].normal.x = mvp[0][3] - mvp[0][2];
        //planes[5].normal.y = mvp[1][3] - mvp[1][2];
        //planes[5].normal.z = mvp[2][3] - mvp[2][2];
        //planes[5].d = mvp[3][3] - mvp[3][2];
        //planes[5].normalize();
    }

    // Check if box is culled by frustum, box defined by center and half size.
    bool isBoxCulled(const geom::psAABB &box) const
    {
        for (const Plane &plane : planes)
        {
            // Compute the projection interval radius of box onto plane normal
            double r = glm::dot(box.size, glm::abs(plane.normal));

            // Distance from box center to plane
            double s = plane.distance(box.cen);

            if (s + r < 0)
            {
                // Box is completely outside this plane => culled
                return true;
            }
        }
        return false; // Box is at least partially inside frustum
    }

private:
    Plane planes[5];
};

class QTreeData
{
public:
    QTreeData(gceContext &ctx, gceTileCache &tiles, gceQPatchCache &patches, gce::uuid &id_model)
        : ctx(ctx), m_qtiles(tiles), m_qpatches(patches), id_model(id_model)
    {
        psCalcRec();
    }

    //!calculate LOD parameters
    void psCalcRec()
    {
        const double size = 0.5; // texture space half-size
        double f = a * (size * size + size * size);

        for (int i = 0; i < NUM_LEVELS; ++i)
        {
            i_clev[i] = f - b;
            double i_cnlv = f * cfac - b;
            double i_nlev = f * 4.0f - b;
            pars[i][0] = i_cnlv;
            pars[i][1] = 1.0f / (i_nlev - i_cnlv);

            f *= 0.25f;
        }

    }
    static glm::vec4 calc_tex_box(gce::tileid keyL, gce::tileid keyH)
    {
        // scale is <=1
        const double scale = ldexp(1.0, keyL.get_z() - keyH.get_z());

        double minx = keyH.get_x() * scale - keyL.get_x();
        double miny = keyH.get_y() * scale - keyL.get_y();

        return {minx, miny, minx + scale, miny + scale};
    }

    GLuint queryTile(gce::tileid nodetile, gce::tileid textile, glm::vec4 &texbox)
    {
        GLuint texid = 0;
        const gceTileCache::tilekey key{id_model, textile};

        if (auto it = m_qtiles.m_tree.find(key); it != m_qtiles.m_tree.end())
        {
            texid = it->second.tex.name();
            texbox = calc_tex_box(textile, nodetile);
        }
        else
        {
            // TODO: remove duplication with tiled2 -> move to cache class
            // if a tile image does not exist in the database or is already queried, do not send a new query
            if (m_qtiles.m_queried.find(key) == m_qtiles.m_queried.end() && m_qtiles.m_missing.find(key) == m_qtiles.m_missing.end())
            {
                umodelQueryTileImageMsg msg;
                msg.id_model = id_model;
                msg.sender = gce::queueId::RENDER;
                msg.tileId = textile;
                ctx.postModelQueue(msg);
                m_qtiles.m_queried.insert(key);
            }

            uint32_t x = textile.get_x();
            uint32_t y = textile.get_y();
            uint8_t level = textile.get_z();

            while (--level > 0)
            {
                x /= 2;
                y /= 2;
                gce::tileid tileCoarse(x, y, level);
                if (auto it2 = m_qtiles.m_tree.find({id_model, tileCoarse}); it2 != m_qtiles.m_tree.end())
                {
                    texid = it2->second.tex.name();
                    texbox = calc_tex_box(tileCoarse, nodetile);
                    break;
                }
            }
        }
        return texid;
    }

    const gceQPatchCache::QPatchCacheData *queryPatch(gce::tileid tile)
    {
        const gceTileCache::tilekey key{id_model, tile};

        if (auto it = m_qpatches.m_tree.find(key); it != m_qpatches.m_tree.end())
        {
            return &it->second;
        }
        if (m_qpatches.m_queried.find(key) == m_qpatches.m_queried.end())
        {
            umodelQueryTileDEMMsg msg;
            msg.id_model = id_model;
            msg.sender = gce::queueId::RENDER;
            msg.tileId = tile;
            ctx.postModelQueue(msg);
            m_qpatches.m_queried.insert(key);
            ++s_nodepatchesQ;
        }
        return nullptr;
    }

    void update_camera()
    {
        frustum.extractFrustumPlanes(proj);
        m_sot.init(cam_coo, wgsop::Ra, wgsop::Rb);
    }

    bool psTest(const geom::psAABB &bb0) const
    {
        return frustum.isBoxCulled(bb0);
    }

    gceContext &ctx;

    double cfac = 2.2; //!< size of mix-zone of fine and and coarse meshes
    double a = 32.0; //!< control distance to LOD zero
    double b = 1.0e-15; //!< distance delta, somewhat bigger than minimal element size to force i_clev{max level] below zero
    bool update = true; //!< control if tree update is done before rendering

    glm::dmat4 proj;
    glm::dvec3 cam_coo_tex; // u, v, h(scaled)
    glm::dvec3 cam_coo;

    CameraFrutrum frustum;

    static constexpr int NUM_LEVELS = gce::tileid::max_level() + 1;
    double i_clev[NUM_LEVELS];
    glm::fvec2 pars[NUM_LEVELS];

    SpheroidOcclusionTester m_sot;

    static constexpr int MAX_QNODES1 = 40000;
    gce::idxalloca<QTreeNode, uint16_t> qnodes{MAX_QNODES1};
    gceTileCache &m_qtiles;
    gceQPatchCache &m_qpatches;

    gce::uuid id_model;
};
inline double tex2flat_uv(double x)
{
    return (x - 0.5) * wgsop::M_2PI;
}

struct QTreeNode
{
public:
    static constexpr uint16_t MAX_NODE = QTreeData::MAX_QNODES1;

    QTreeNode(const glm::dvec3 &cen, const glm::dvec3 &size) : box(cen, size)
    {}


    QTreeNode() {}

    static geom::Box2D flat_box(gce::tileid tile)
    {
        double scale = std::ldexp(wgsop::M_2PI, -tile.get_z());
        double xmin = -M_PI + scale * tile.get_x();
        double ymax = M_PI - scale * tile.get_y();
        return geom::Box2D{glm::dvec2(xmin, ymax - scale), glm::dvec2(xmin + scale, ymax)};
    }

    static geom::Box2D tex_box(gce::tileid tile)
    {
        double scale = std::ldexp(1.0, -tile.get_z());
        double xmin = scale * tile.get_x();
        double ymax = 1.0 - scale * tile.get_y();
        return geom::Box2D{glm::dvec2(xmin, ymax - scale), glm::dvec2(xmin + scale, ymax)};
    }


    static geom::psAABB make_box(const gce::tileid tile, double hmin, double hmax)
    {
        auto box2 = flat_box(tile);
        wgsop::xyz_grid<2, gce::sizep, wgsop::toWGS_fromflat> AC(box2.cmin.x, box2.cmin.y, box2.cmax.x, box2.cmax.y);

        geom::Box3D bbox;

        glm::dvec2 posxy;
        for (int n = 0; n < 2; ++n)
        {
            const double g1_hmin = (AC.lata[n].Rn + hmin) * AC.lata[n].cos_lat;
            const double g2_hmin = ((AC.lata[n].Rn * (1.0 - wgsop::sqr_e)) + hmin) * AC.lata[n].sin_lat;

            const double g1_hmax = (AC.lata[n].Rn + hmax) * AC.lata[n].cos_lat;
            const double g2_hmax = ((AC.lata[n].Rn * (1.0 - wgsop::sqr_e)) + hmax) * AC.lata[n].sin_lat;

            for (int m = 0; m < gce::sizep; ++m)
            {
                posxy[0] = g1_hmin * AC.lona[m].cos_lon;
                posxy[1] = g1_hmin * AC.lona[m].sin_lon;

                //bbox.expand(posxy);
                if (posxy.x < bbox.cmin.x) bbox.cmin.x = posxy.x;
                if (posxy.y < bbox.cmin.y) bbox.cmin.y = posxy.y;
                if (posxy.x > bbox.cmax.x) bbox.cmax.x = posxy.x;
                if (posxy.y > bbox.cmax.y) bbox.cmax.y = posxy.y;

                posxy[0] = g1_hmax * AC.lona[m].cos_lon;
                posxy[1] = g1_hmax * AC.lona[m].sin_lon;

                //bbox.expand(posxy);
                if (posxy.x < bbox.cmin.x) bbox.cmin.x = posxy.x;
                if (posxy.y < bbox.cmin.y) bbox.cmin.y = posxy.y;
                if (posxy.x > bbox.cmax.x) bbox.cmax.x = posxy.x;
                if (posxy.y > bbox.cmax.y) bbox.cmax.y = posxy.y;
            }
            // 
            if (g2_hmin < bbox.cmin.z) bbox.cmin.z = g2_hmin;
            if (g2_hmin > bbox.cmax.z) bbox.cmax.z = g2_hmin;

            if (g2_hmax < bbox.cmin.z) bbox.cmin.z = g2_hmax;
            if (g2_hmax > bbox.cmax.z) bbox.cmax.z = g2_hmax;
        }

        return geom::psAABB(bbox);
    }

    void initMesh(const gceQPatchCache::QPatchCacheData &patch)
    {
        this->box = patch.bbox;
        this->hmax = patch.hmax;
        if (!m_array.isBuffer())
        {
            m_array = BufferGL();
        }
        glNamedBufferData(m_array.name(), sizeof(patch.patch), &patch.patch, GL_STATIC_DRAW);
    }

    void initMesh()
    {
        auto box2 = flat_box(tile);
        wgsop::xyz_grid<gce::sizep, gce::sizep, wgsop::toWGS_fromflat> AC(
            box2.cmin.x,
            box2.cmin.y,
            box2.cmax.x,
            box2.cmax.y
        );

        const glm::dvec3 origin = this->box.cen;

        //tex_grid<sizep, sizep> AT(box2);

        auto qp = std::make_unique<QPatch>();

        //dvec3 pos;
        for (int n = 0; n < gce::sizep; ++n)
        {
            // lat-dependent constants
            const double g1 = AC.lata[n].Rn * AC.lata[n].cos_lat;
            const double g2 = AC.lata[n].Rn * (1.0 - wgsop::sqr_e) * AC.lata[n].sin_lat;
            const float local_z = g2 - origin.z;

            for (int m = 0; m < gce::sizep; ++m)
            {
                const double x = g1 * AC.lona[m].cos_lon;
                const double y = g1 * AC.lona[m].sin_lon;

                // target vertex
                auto &vv = qp->array[m][n];
                // source vertex for coarse LOD: current or previous
                //const psDVertex &vv_src = qp.array[m & (~1)][n & (~1)];

                // position
                vv.pos[0] = x - origin.x;
                vv.pos[1] = y - origin.y;
                vv.pos[2] = local_z;

                vv.normal = glm::normalize(glm::vec3(x, y, g2));

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
        if (!m_array.isBuffer())
        {
            m_array = BufferGL();
        }
        glNamedBufferData(m_array.name(), sizeof(qp->array), &qp->array[0][0], GL_STATIC_DRAW);

        //mesh = std::move(qp);
    }

    void Update(std::vector<tech_tiled3::BatchGL> &batches, QTreeData &data, int lev)
    {
        geom::Box2D texbox = tex_box(tile);

        if (lev < gce::tileid::max_level() && tex_sq_min(data.cam_coo_tex, texbox) < data.i_clev[lev])
        {
            for (int i = 0; i < 4; i++)
            {
                auto &n = chld[i];
                if (auto *node = data.qnodes.getX(n); node != nullptr)
                {
                    node->Update(batches, data, lev + 1);
                }
                else if (data.qnodes.newX(n))
                {
                    gce::tileid newTile = this->tile.get_part(i + 1);

                    node = data.qnodes.getX(n);
                    node->box = make_box(newTile, 0.0, 1.0);
                    //node->box2 = this->box2.getPart2D(i + 1);
                    node->tile = newTile;

                    node->Update(batches, data, lev + 1);
                }
                else
                {
                    // unable to allocate child node, clear children and use current node
                }
            }
            return;
        }

        KillChildren(data);

        bool isCulled = data.psTest(box) || data.m_sot.test(box, 1.0);
        if (isCulled)
        {
            return;
        }

        if (!m_array.isBuffer())
        {
            initMesh();
        }
        auto &b = batches.emplace_back();
        b.array = m_array.name();
        b.mvp = data.proj;// *calc_mat(box);
        // TODO: do not query tile every frame?
        b.tex = data.queryTile(tile, tile.parent(), b.texbox);// it->second.m_tex.name();
        b.origin = box.cen;

        //b.texbox = {0.0, 0.0, 1.0, 1.0};
    }

    void BatchNodeIfVisible(std::vector<tech_tiled3::BatchGL> &batches, QTreeData &data)
    {
        bool isCulled = data.psTest(box) || data.m_sot.test(box, 1.0);
        if (isCulled)
        {
            return;
        }
        if (!m_array.isBuffer())
        {
            return;
        }

        auto &b = batches.emplace_back();
        b.array = m_array.name();
        b.mvp = data.proj;// *calc_mat(box);
        // TODO: do not query tile every frame?
        b.tex = data.queryTile(tile, tile.parent(), b.texbox);// it->second.m_tex.name();
        b.origin = box.cen;
    }

    void Update2(std::vector<tech_tiled3::BatchGL> &batches, QTreeData &data, unsigned int lev)
    {
        if (lev < gce::tileid::max_level() && tex_sq_min(data.cam_coo_tex, tex_box(tile)) < data.i_clev[lev])
        {
            // we need to unfold tree, check if childeren are ready
            int chld_allocated = 0;
            for (int i = 0; i < 4; i++)
            {
                if (data.qnodes.isId(chld[i]))
                {
                    ++chld_allocated;
                }
                else if (data.qnodes.newX(chld[i]))
                {
                    gce::tileid newTile = this->tile.get_part(i + 1);

                    auto *node = data.qnodes.getX(chld[i]);
                    node->box = make_box(newTile, 0.0, 1.0);
                    node->tile = newTile;
                    ++chld_allocated;
                }
            }
            if (chld_allocated != 4)
            {
                // failed to allocate some nodes
                KillChildren(data);
                BatchNodeIfVisible(batches, data);
                return;
            }

            // all nodes are allocated
            int chld_ready = 0;
            for (int i = 0; i < 4; i++)
            {
                auto *node = data.qnodes.getX(chld[i]);
                if (node->hasData())
                {
                    ++chld_ready;
                }
                // init from cache if available, otherwise send query for data
                else if (const auto *patch = data.queryPatch(node->tile))
                {
                    node->initMesh(*patch);
                    ++chld_ready;
                }
            }
            if (chld_ready != 4)
            {
                // draw current and wait for data
                BatchNodeIfVisible(batches, data);
                return;
            }

            for (int i = 0; i < 4; i++)
            {
                auto *node = data.qnodes.getX(chld[i]);
                node->Update2(batches, data, lev + 1);
            }
            return;
        }

        // fold subtree and draw current node
        KillChildren(data);
        BatchNodeIfVisible(batches, data);

        //b.texbox = {0.0, 0.0, 1.0, 1.0};
    }

    bool hasData() const
    {
        return m_array.isBuffer();
    }

//private:

    void kill(QTreeData &data)
    {
        KillChildren(data);
        //mesh.reset();
        m_array = BufferGL{BufferGL::nocreate};
    }

    void KillChildren(QTreeData &data)
    {
        for (auto &n : chld)
        {
            if (auto *node = data.qnodes.getX(n); node != nullptr)
            {
                node->kill(data);
                data.qnodes.delX(n);
            }
        }

    }

    geom::psAABB  box; //!< geometry, geocentric space
    gce::tileid tile; //!< current tile id
    float hmax = 0.0; // max height over ellipsoid
    uint16_t  chld[4] = {MAX_NODE, MAX_NODE, MAX_NODE, MAX_NODE}; //!< child nodes
    BufferGL m_array = BufferGL::nocreate;
    //std::unique_ptr<tech_tiled3::QPatch> mesh;
};

}
constexpr size_t index_len(int size)
{
    size_t len = 0;
    for (int k = 0; k < size - 1; k += 8)
    {
        for (int i = 0; i < size - 1; i++)
        {
            for (int j = 0; j <= 8; j++)
            {
                len += 2;
            }
            len++;
        }
    }
    return len;
}

gceTechTiled3::gceTechTiled3(gceStorageGL &storage) : gceTechGlobe(gceTechType::TILED2), m_storage(storage)
{
    init();

    // pass coordintes of the Earth box in geocentric CS
    m_tree = std::make_unique<tech_tiled3::QTreeNode>(glm::dvec3{0.0}, glm::dvec3{6.5e+6});
}
gceTechTiled3::~gceTechTiled3()
{}
void gceTechTiled3::init()
{
    //wxStopWatch sw;
    m_progBasic = m_storage.progs.FindOrCreate("textile3");
    if (!m_progBasic->IsOk())
    {
        gl::ProgramInfo info;
        info.addShaderFile(GL_VERTEX_SHADER, "prog/textile3.vert");
        info.addShaderFile(GL_FRAGMENT_SHADER, "prog/textile3.frag");

        gceContext::log_error("prog/textile3 load {}", m_progBasic->Create(info));
    }
    this->loc_position = m_progBasic->getAttribLocation("position");
    this->loc_normal = m_progBasic->getAttribLocation("normal");
    this->loc_texp = m_progBasic->getAttribLocation("texp");
    this->loc_mvp = m_progBasic->getUniformLocation("mvp");
    this->loc_tex = m_progBasic->getUniformLocation("tex");
    this->loc_texbox = m_progBasic->getUniformLocation("texbox");

    setTexposArray();
    setElements();
    setVAO();

    //gceContext::log_message("gceTechBasic::init {}ms", sw.Time());
}

void gceTechTiled3::prepareScene(gceContext &ctx, const glm::dmat4 &proj, const glm::dvec3 &pos, const glm::ivec2 &res)
{
    ensureTreeDataInit(ctx);

    m_treeData->proj = proj;
    m_treeData->cam_coo = pos;

    glm::dvec3 wgspos = wgsop::xyz2wgs(pos);
    double latRad = glm::radians(wgspos.y);
    m_treeData->cam_coo_tex.x = wgspos.x / 360.0 + 0.5;
    m_treeData->cam_coo_tex.y = wgsop::wgs2flat_Y(latRad) / (2.0 * M_PI) + 0.5;
    // TODO: may apply some filter for smoother change or camera speed reaction
    m_treeData->cam_coo_tex.z = wgspos.z / (2.0 * M_PI * wgsop::wgs2scale(latRad));

    m_treeData->update_camera();

    //gceContext::log_message("{}; {}; {} == {}", m_treeData->cam_coo_tex.x, m_treeData->cam_coo_tex.y, m_treeData->cam_coo_tex.z, wgspos.z);

    m_batches.clear();
    m_tree->Update2(m_batches, *m_treeData, 0);

    s_framesCnt++;
    if (s_framesCnt < 1000)
    {
        //gceContext::log_message("Tiled3: {} batches, {} queries", m_batches.size(), s_nodepatchesQ);
        //gceContext::log_message("render: {} model: {} data: {}", ctx.renderQueue.size(), ctx.modelQueue.size(), ctx.dataQueue.size());
    }

    // TODO: sort/optimize batches

}

void gceTechTiled3::ensureTreeDataInit(gceContext &ctx)
{
    if (!m_treeData)
    {
        m_treeData = std::make_unique<tech_tiled3::QTreeData>(ctx, m_storage.tiles, m_storage.qpatches, id_model);
    }
}

inline glm::mat4 make_mvp(const glm::dmat4 &dmvp, const glm::dvec3 &origin, const glm::dvec3 &base)
{
    return glm::mat4(dmvp[0], dmvp[1], dmvp[2], dmvp * glm::dvec4(origin - base, 1.0));
}
void gceTechTiled3::renderScene(const glm::dmat4 &proj, const glm::dvec3 &aoi, const glm::ivec2 &res, bool wireFrameTerrain)
{
    static size_t s_tiles = 0;
    static size_t s_frames = 0;

    if (wireFrameTerrain)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glDisable(GL_CULL_FACE);
    }
    else
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glEnable(GL_CULL_FACE);
    }
    //glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_PRIMITIVE_RESTART);
    glPrimitiveRestartIndex(0xffff);
    m_progBasic->Begin();
    glBindVertexArray(m_vao.name());
    for (auto &b : m_batches)
    {
        glBindTextureUnit(0, b.tex);
        glBindVertexBuffer(0, b.array, 0, sizeof(tech_tiled3::ArrayVertex));

        m_progBasic->setValue(loc_mvp, make_mvp(proj, b.origin, glm::dvec3{0.0}));
        m_progBasic->setValue(loc_texbox, b.texbox);

        glDrawElements(GL_TRIANGLE_STRIP, index_len(gce::sizep), GL_UNSIGNED_SHORT, (const GLvoid *)0);
    }
    glBindTextureUnit(0, 0);
    glBindVertexArray(0);
    m_progBasic->End();

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_CULL_FACE);

    s_tiles += m_batches.size();
    if (++s_frames == 100)
    {
        //gceContext::log_message("Terrain tiles: {}", s_tiles / s_frames);
        s_frames = 0;
        s_tiles = 0;
    }
}

void gceTechTiled3::processMsg(const urenderTechDataMsg &msg)
{
    if (msg.id_model == id_model && msg.techType == gceTechType::TILED3)
    {
        //processData(static_cast<const tech_tiled3::DataMsg &>(msg));
    }
}

void gceTechTiled3::setTexposArray()
{
    constexpr auto size = gce::sizep;

    std::vector<glm::fvec2> buffer;
    buffer.reserve(size * size);

    for (int n = 0; n < size; ++n)
    {
        float tx = n / float(size - 1);
        for (int m = 0; m < size; ++m)
        {
            float ty = 1.0f - m / float(size - 1);
            buffer.emplace_back(tx, ty);
        }
    }

    glNamedBufferData(m_texcoords.name(), sizeof(glm::fvec2) * buffer.size(), buffer.data(), GL_STATIC_DRAW);

    glVertexArrayVertexBuffer(m_vao.name(), 1, m_texcoords.name(), 0, sizeof(glm::fvec2));
}

void gceTechTiled3::setElements()
{
    constexpr auto size = gce::sizep;
    const int RESTART_INDEX = 0xFFFF;
    constexpr auto elements_len = index_len(gce::sizep);

    std::vector<unsigned short> buffer(elements_len);
    unsigned short len = 0;
    for (unsigned short k = 0; k < size - 1; k += 8)
    {
        for (unsigned short i = 0; i < size - 1; i++)
        {
            for (unsigned short j = 0; j <= 8; j++)
            {
                buffer[len++] = i * size + j + k;
                buffer[len++] = i * size + j + k + size;
            }
            buffer[len++] = RESTART_INDEX;
        }
    }

    glNamedBufferData(m_elements.name(), sizeof(unsigned short) * buffer.size(), buffer.data(), GL_STATIC_DRAW);
}

void gceTechTiled3::setVAO()
{
    glVertexArrayAttribFormat(m_vao.name(), loc_position, 3, GL_FLOAT, GL_FALSE, offsetof(vertex_t, pos));
    glVertexArrayAttribBinding(m_vao.name(), loc_position, 0);
    glEnableVertexArrayAttrib(m_vao.name(), loc_position);

    glVertexArrayAttribFormat(m_vao.name(), loc_normal, 3, GL_FLOAT, GL_FALSE, offsetof(vertex_t, normal));
    glVertexArrayAttribBinding(m_vao.name(), loc_normal, 0);
    glEnableVertexArrayAttrib(m_vao.name(), loc_normal);

    glVertexArrayAttribFormat(m_vao.name(), loc_texp, 2, GL_FLOAT, GL_FALSE, offsetof(glm::fvec2, x));
    glVertexArrayAttribBinding(m_vao.name(), loc_texp, 1);
    glEnableVertexArrayAttrib(m_vao.name(), loc_texp);

    glVertexArrayElementBuffer(m_vao.name(), m_elements.name());
}


