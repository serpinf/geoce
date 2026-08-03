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
#include "urendernode.hpp"
#include <wx/glcanvas.h>
#include "teches/RendererGL.h"
#include "teches/techbasic.h"
#include "teches/techbasicrte2.h"
#include "teches/tech_tiled2.h"
#include "teches/tech_tiled3.h"
#include "teches/tech_objects2.h"
#include "teches/tech_objects3.h"
#include <glm/gtc/matrix_transform.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>

template <> struct fmt::formatter<gceEntityStatus> : formatter<string_view>
{
// parse is inherited from formatter<string_view>.

    auto format(gceEntityStatus c, format_context &ctx) const
    {
        string_view name;
        switch (c)
        {
        case gceEntityStatus::FAILED:   name = "FAILED"; break;
        case gceEntityStatus::OK: name = "OK"; break;
        case gceEntityStatus::NONE:  name = "NONE"; break;
        }
        return formatter<string_view>::format(name, ctx);
    }
};
struct TechData
{
    gce::uuid id_layer{};
    gce::uuid id_model{};
    std::unique_ptr<gceTechFlat> flat;
    std::unique_ptr<gceTechGlobe> globe;
    gceEntityStatus m_status = gceEntityStatus::NONE;
    bool visible = false;
    int order = 0;

    template <class T> void passMsg(const T &msg) const
    {
        if (flat)
        {
            flat->processMsg(msg);
        }
        if (globe)
        {
            globe->processMsg(msg);
        }
    }
};
using tech_layer_model = boost::multi_index::multi_index_container<
    TechData,
    boost::multi_index::indexed_by<
    boost::multi_index::ordered_unique<
    boost::multi_index::tag<struct TechDataByLayer>,
    boost::multi_index::member<TechData, gce::uuid, &TechData::id_layer>
    >,
    boost::multi_index::ordered_non_unique<
    boost::multi_index::tag<struct TechDataByModel>,
    boost::multi_index::member<TechData, gce::uuid, &TechData::id_model>
    >
    >
>;

template <class T> GCE_WITH_REQUIRES(gce::is_any_v<T, gceTechFlat, gceTechGlobe>)
struct TechSortData
{
    static_assert(gce::is_any_v<T, gceTechFlat, gceTechGlobe>, "T must be gceTechFlat or gceTechGlobe");
    int order = 0;
    T *tech = nullptr;
    TechSortData(int order, T *tech) : order(order), tech(tech) {}
    bool operator < (const TechSortData &other) const
    {
        return order < other.order;
    }
};

class gceRendererGL final : public boost::noncopyable
{
public:
    explicit gceRendererGL(gceContext &ctx) : m_ctx(ctx)
    {
        init();
    }
    ~gceRendererGL()
    {
        clear();
    }

    //void updateAOI(const glm::dmat4& proj);

    void renderSceneFlat()
    {
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);

        teches_sorted_flat.clear();
        for (auto &tech : m_teches)
        {
            if (tech.flat && tech.visible)
            {
                teches_sorted_flat.emplace_back(tech.order, tech.flat.get());
            }
        }
        std::sort(teches_sorted_flat.begin(), teches_sorted_flat.end());

        glClearColor(0.9f, 0.9f, 0.9f, 1.0f);
        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
        glViewport(m_viewport[0], m_viewport[1], m_viewport[2], m_viewport[3]);

        for (auto &[order, tech] : teches_sorted_flat)
        {
            tech->prepareScene(m_ctx, m_proj, m_aoi, {m_viewport[2], m_viewport[3]});
            //for (int i = 0; i < 100; i++)
            {
                tech->renderScene(m_proj, m_aoi, {m_viewport[2], m_viewport[3]});
            }
        }
    }
    void renderSceneGlobe()
    {
        teches_sorted_globe.clear();
        for (auto &tech : m_teches)
        {
            if (tech.globe && tech.visible)
            {
                teches_sorted_globe.emplace_back(tech.order, tech.globe.get());
            }
        }
        std::sort(teches_sorted_globe.begin(), teches_sorted_globe.end());

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
        glViewport(m_viewport[0], m_viewport[1], m_viewport[2], m_viewport[3]);

        for (auto &[order, tech] : teches_sorted_globe)
        {
            tech->prepareScene(m_ctx, m_proj, m_pos, {m_viewport[2], m_viewport[3]});
            //for (int i = 0; i < 100; i++)
            {
                tech->renderScene(m_proj, m_pos, {m_viewport[2], m_viewport[3]}, m_wireFrameTerrain);
            }
        }
    }
    void renderScene()
    {
        if (m_mode == gceRenderMode::FLAT)
        {
            renderSceneFlat();
        }
        else //if (m_mode == gceRenderMode::GLOBE)
        {
            renderSceneGlobe();
        }
    }

    bool isOk() const
    {
        return m_init;
    }

    void operator ()(const wspResetMsg &)
    {
        this->clear();
    }

    //#include <source_location>
    //#include <fmt/std.h>
    std::unique_ptr<gceTechFlat> createTechFlat(gceTechType techType)
    {
        switch (techType)
        {
        case gceTechType::BASIC:
            return  std::make_unique<gceTechBasic>(m_storage);
        case gceTechType::BASIC_RTE2:
            return std::make_unique<gceTechBasicRTE2>(m_storage);
        case gceTechType::TILED2:
            return std::make_unique<gceTechTiled2>(m_storage);
        case gceTechType::OBJECTS2:
            return std::make_unique<gceTechObjects2>(m_storage);
        default:
            break;
        }
        return {};
    }


    void operator ()(const urenderReplaceTechMsg &msg)
    {
        TechData data;
        data.id_layer = msg.id_layer;
        data.id_model = msg.id_model;
        data.visible = msg.visible;
        data.order = msg.order;
        data.flat = createTechFlat(msg.techFlat);
        if (data.flat)
        {
            data.flat->id_layer = msg.id_layer;
            data.flat->id_model = msg.id_model;
        }
        data.globe = createTechGlobe(msg.techGlobe);
        if (data.globe)
        {
            data.globe->id_layer = msg.id_layer;
            data.globe->id_model = msg.id_model;
        }

        auto &index = m_teches.get<TechDataByLayer>();
        if (auto it = index.find(msg.id_layer); it != index.end())
        {
            index.replace(it, std::move(data));
        }
        else
        {
            index.insert(std::move(data));
        }
    }
    void operator ()(const urenderDrawFrameMsg &msg)
    {
        m_mode = gceRenderMode::FLAT;
        m_proj = msg.proj;
        m_aoi = msg.aoi;
        m_viewport = msg.viewport;
        m_updateFlag = true;
    }
    void operator ()(const urenderDrawFrame3DMsg &msg)
    {
        m_mode = gceRenderMode::GLOBE;
        m_basePoint = msg.basePoint;
        m_pos = msg.pos;
        m_front = msg.front;
        m_up = msg.up;
        m_fovy = msg.fovy;
        m_viewport = msg.viewport;
        m_wireFrameTerrain = msg.wireFrameTerrain;

        updatePerspective();
        m_updateFlag = true;
    }
    void operator ()(const urenderUpdateTechMsg &msg)
    {
        auto &techByLayer = m_teches.get<TechDataByLayer>();
        if (auto tech = techByLayer.find(msg.id_layer); tech != techByLayer.end())
        {
            techByLayer.modify(tech, [&](TechData &tech){
                tech.visible = msg.visible;
                tech.order = msg.order;
            });
        }
    }
    void operator ()(const urenderRemoveTechMsg &msg)
    {
        if (m_teches.erase(msg.id_layer) == 0)
        {
            gceContext::log_message("gceRendererGL::operator(urenderRemoveTechMsg): no such tech to remove");
        }
    }
    void operator ()(const urenderTechStatusMsg &msg)
    {
        auto status = gceEntityStatus::NONE;
        auto &byLayer = m_teches.get<TechDataByLayer>();
        if (auto tech = byLayer.find(msg.id_layer); tech != byLayer.end())
        {
            status = tech->m_status;
        }
        post_techStatus(msg.sender, msg.id_layer, status);
    }
    void operator ()(const umodelStatusReplyMsg &msg)
    {
        auto &byModel = m_teches.get<TechDataByModel>();
        auto range = byModel.equal_range(msg.id_model);
        for (auto tech = range.first; tech != range.second; ++tech)
        {
            if (tech->m_status != msg.status)
            {
                byModel.modify(tech, [&msg](TechData &tech){
                    tech.m_status = msg.status;
                });
                gceContext::log_message("tech status set to {}", msg.status);
            }

        }
    }

    void operator ()(const urenderTileImageDataMsg &msg)
    {
        auto &cache = m_storage.tiles;
        gceTileCache::tilekey key{msg.id_model, msg.tile};

        if (cache.m_tree.find(key) != cache.m_tree.end())
        {
            gceContext::log_message("duplicate tile cache add attempted: {}", msg.tile.to_string());
            return;
        }
        cache.m_queried.erase(key);

        if (msg.tex.data.empty())
        {
            if (msg.modelOK)
            {
                cache.m_missing.insert(key);
            }
        }
        else
        {
            auto &node = cache.m_tree[key];
            node = {};

            GLuint tex = node.tex.name();
            glTextureParameteri(tex, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTextureParameteri(tex, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            glTextureStorage2D(tex, 1, GL_RGBA8, 256, 256);
            glTextureSubImage2D(tex, 0, 0, 0, msg.tex.w, msg.tex.h, GL_RGBA, GL_UNSIGNED_BYTE, msg.tex.data.data());
        }
    }

    void operator ()(const urenderTileDEMDataMsg &msg)
    {
        auto &cache = m_storage.qpatches;
        const gceQPatchCache::tilekey key{msg.id_model, msg.tile};
        //TODO: consider ignoring not queried
        if (cache.m_tree.find(key) != cache.m_tree.end())
        {
            gceContext::log_message("duplicate qpatch cache add attempted: {}", msg.tile.to_string());
            return;
        }
        cache.m_queried.erase(key);

        if (msg.patch)
        {
            auto &node = cache.m_tree[key];
            node.bbox = msg.bbox;
            node.hmax = msg.hmax;
            node.patch = *msg.patch;
        }
    }

    void operator ()(const std::shared_ptr<urenderTechDataMsg> &msg)
    {
        if (!msg) return;

        auto range = m_teches.get<TechDataByModel>().equal_range(msg->id_model);
        std::for_each(range.first, range.second, [&msg](const TechData &tech){
            tech.passMsg(*msg);
        });
    }

    //void operator ()(const T &msg)
    //{
      //  passMsgToTech(msg);
    //}
    template <class T> void operator ()(const T &)
    {
        gceContext::log_message("urenderNode: unsupported msg type {}", typeid(T).name());
    }
    bool m_updateFlag = false;
private:

    template <class T>
    void passMsgToTech(TechData *tech, const T &msg)
    {
        for (const TechData &tech : m_teches.get<TechDataByLayer>())
        {
            if (tech.flat)
            {
                tech.flat->processMsg(msg);
            }
            if (tech.globe)
            {
                tech.globe->processMsg(msg);
            }
        }
    }

    std::unique_ptr<gceTechGlobe> createTechGlobe(gceTechType techType)
    {
        switch (techType)
        {
        case gceTechType::TILED3:
            return std::make_unique<gceTechTiled3>(m_storage);
        case gceTechType::OBJECTS3:
            return std::make_unique<gceTechObjects3>(m_storage);
        default:
            break;
        }
        return {};
    }

    void queryModelStatus(const gce::uuid &id_model)
    {
        if (!id_model.is_nil())
        {
            umodelStatusMsg modelMsg;
            modelMsg.id_model = id_model;
            modelMsg.sender = gce::queueId::RENDER;
            m_ctx.postModelQueue(std::move(modelMsg));
        }
    }


    void init()
    {
        m_init = true;
    }
    void clear()
    {
        teches_sorted_flat.clear();
        teches_sorted_globe.clear();
        m_teches.clear();
        m_init = false;
    }
    void post_techStatus(uint32_t target, const gce::uuid &id_layer, gceEntityStatus status)
    {
        urenderStatusReplyMsg reply;
        reply.id_layer = id_layer;
        reply.status = status;
        m_ctx.postQueue(target, reply);
    }

    void updatePerspective()
    {
        const double _near = 0.001;
        glm::dmat4 M = glm::infinitePerspective(glm::radians(m_fovy), double(m_viewport.z) / m_viewport.w, _near);
        //glm::dmat4 M = glm::perspective(glm::radians(m_fovy), double(m_viewport.z) / m_viewport.w, _near, 12e+6);
        const glm::dvec3 as = m_pos - m_basePoint;
        m_proj = M * glm::lookAt(as, as + m_front, m_up);
    }

    bool m_init = false;
    //std::map<gce::uuid, TechData > m_teches;
    tech_layer_model m_teches;
    std::vector<TechSortData<gceTechFlat>> teches_sorted_flat;
    std::vector<TechSortData<gceTechGlobe>> teches_sorted_globe;
    gceContext &m_ctx;
    gceStorageGL m_storage;

    gceRenderMode m_mode = gceRenderMode::FLAT;
    glm::dmat4 m_proj{};
    geom::aabb2 m_aoi;
    glm::ivec4 m_viewport{};
    glm::dvec3 m_basePoint{};
    glm::dvec3 m_pos{};
    glm::dvec3 m_front{};
    glm::dvec3 m_up{};
    double m_fovy{};
    bool m_wireFrameTerrain = false;
};

static void APIENTRY procgl(GLenum /*source*/, GLenum /*type*/, GLenum /*id*/, GLenum /*severity*/, GLsizei /*length*/, const GLchar *message, const void * /*userParam*/)
{
    gceContext::log_message("msg gl {}", message);
}
#include <GL/wglew.h>

namespace gce
{
void urender_worker(gceContext *ctx, wxGLCanvas *c)
{
    glewInit();

    glDebugMessageCallback(procgl, nullptr);
    //glEnable(GL_DEBUG_OUTPUT);
    wglSwapIntervalEXT(-1);

    constexpr auto deltaT = std::chrono::milliseconds(16);
    gceRendererGL rp{*ctx};
    auto when = std::chrono::steady_clock::now() + deltaT;
    double time_ms = 10.0;
    while (true)
    {
        // this loop implementation processes all incoming messages before the frame rendering
        if (gce::MessageInfo msg; ctx->renderQueue.get_until(msg, when))
        {
            if (std::holds_alternative<gceQuitMessage>(msg)) break;

            std::visit(rp, msg);
        }
        else if (rp.m_updateFlag)
        {
            rp.m_updateFlag = false;
            using clock = std::chrono::high_resolution_clock;
            auto t_start = clock::now();

            rp.renderScene();
            c->SwapBuffers();
            when += deltaT;

            time_ms = glm::mix(time_ms, std::chrono::duration<double, std::milli>(clock::now() - t_start).count(), 0.1);
            ctx->postWorkspaceQueue(urenderFrameMsMsg{time_ms});
        }
    }
    glFinish();
    //gceContext::log_message("urender_worker exited");
}
}

