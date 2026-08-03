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
#include <map>
#include "umodelnode.h"
#include "models/modeltest.hpp"
#include "models/geometric.h"
#include "models/selectionmodel.h"
#include "models/imagetile.h"
#include "config.h"

namespace gce
{

class umodelProcessor
{
public:
    explicit umodelProcessor(gceContext &ctx) : m_ctx(ctx) {}

    void operator()(const umodelReplaceMsg &msg)
    {
        if (const gceModelSchema *modelSchema = m_ctx.cfg.findModelSchema(msg.info.type); modelSchema != nullptr)
        {
            if (modelSchema->m_factory)
            {
                m_models[msg.info.id_model] = modelSchema->m_factory(msg.info);
            }
            else
            {
                gceContext::log_error("umodelProcessor(umodelReplaceMsg): model schema factory is null for model type {}", msg.info.type);
            }
        }
        else
        {
            m_models.erase(msg.info.id_model);
            gceContext::log_message("umodelProcessor(umodelReplaceMsg): unsupported model type {}", msg.info.type);
        }
    }

    void operator()(const umodelUpdateMsg &msg)
    {
        invokeModel(msg.id_model, &gceModelBase::processUpdate, m_ctx, msg);
    }

    void operator()(const umodelStatusMsg &msg)
    {
        gceEntityStatus status = gceEntityStatus::NONE;
        if (auto it = m_models.find(msg.id_model); it != m_models.end() && it->second)
        {
            status = it->second->m_status;
        }
        m_ctx.postQueue(msg.sender, umodelStatusReplyMsg{msg.id_model, status});
    }

    void operator()(const udataTableStatusMsg &msg)
    {
        for (auto &it : this->m_models)
        {
            it.second->processTableStatus(m_ctx, msg);
        }
    }

    void operator()(const udataSelectReplyMsg &msg)
    {
        for (auto &it : this->m_models)
        {
            it.second->processSelectResult(m_ctx, msg);
        }
    }

    void operator()(const udataMultiRowActionNotifyMsg &msg)
    {
        for (auto &it : this->m_models)
        {
            it.second->processActionNotify(m_ctx, msg);
        }
    }

    void operator()(const umodelQueryMesh2DMsg &msg)
    {
        invokeModel(msg.id_model, &gceModelBase::queryMesh2D, m_ctx, msg.tileId);
    }

    void operator()(const umodelEntityAddMsg &msg)
    {
        invokeModel(msg.id_model, &gceModelBase::processEntityAdd, m_ctx, msg);
    }

    void operator()(const umodelEntityUpdateMsg &msg)
    {
        invokeModel(msg.id_model, &gceModelBase::processEntityUpdate, m_ctx, msg);
    }

    void operator()(const umodelEntityRemoveMsg &msg)
    {
        invokeModel(msg.id_model, &gceModelBase::processEntityRemove, m_ctx, msg);
    }

    void operator()(const umodelSelect2DMsg &msg)
    {
        umodelSelectXDResultMsg rmsg;
        rmsg.data.reserve(msg.limit + 1);

        if (!msg.id_modelActive.is_nil())
        {
            invokeModel(msg.id_modelActive, &gceModelBase::processSelect2D, msg, rmsg);
        }
        if (rmsg.data.empty())
        {
            for (auto &id_model : msg.model_ids)
            {
                invokeModel(id_model, &gceModelBase::processSelect2D, msg, rmsg);
            }
        }
        m_ctx.postQueue(msg.sender, std::move(rmsg));
    }

    void operator()(const umodelQueryTileImageMsg &msg)
    {
        invokeModel(msg.id_model, &gceModelBase::processQueryImageTile, m_ctx, msg);
    }

    void operator()(const umodelQueryTileDEMMsg &msg)
    {
        invokeModel(msg.id_model, &gceModelBase::processQueryDEMTile, m_ctx, msg);
    }

    void simulate(double dt)
    {
        for (auto &model : m_models)
        {
            model.second->processIdle(m_ctx);
        }
        //m_ctx->log().message(fmt::format("up.simulate({}s)", dt));
    }
    template <class T> void operator()(const T &)
    {
        gceContext::log_message("umodelProcessor: unsupported msg type {}", typeid(T).name());
    }
private:
    template< class F, class... Args >
    void invokeModel(const gce::uuid &id, F gceModelBase:: *member, Args&&... args)
    {
        if (auto it = m_models.find(id); it != m_models.end())
        {
            ((*it->second).*member)(std::forward<Args>(args)...);
        }
        else
        {
            gceContext::log_message("no such model");
        }
    }

    gceContext &m_ctx;
    std::map<gce::uuid, std::unique_ptr<gceModelBase>> m_models;
};

void umodel_worker(gceContext *ctx)
{
    constexpr auto deltaT = std::chrono::milliseconds(100);
    umodelProcessor up{*ctx};
    auto when = std::chrono::steady_clock::now() + deltaT;
    while (true)
    {
#if 0
        auto msgs = ctx->modelQueue.get_all_until(when);
        if (msgs.empty())
        {
            up.simulate(std::chrono::duration<double>(deltaT).count());
            when += deltaT;
        }
        while (!msgs.empty())
        {
            if (std::holds_alternative<gceQuitMessage>(msgs.front()))
            {
                return;
            }
            std::visit(up, msgs.front());
            msgs.pop();
        }
#else
        // this loop implementation processes all incoming messages before running the simulation
        if (std::chrono::steady_clock::now() >= when)
        {
            up.simulate(std::chrono::duration<double>(deltaT).count());
            when += deltaT;
        }
        gce::MessageInfo msg;
        if (ctx->modelQueue.get_until(msg, when))
        {
            if (std::holds_alternative<gceQuitMessage>(msg))
            {
                break;
            }
            std::visit(up, msg);
        }
        //else
        //{
          //  up.simulate(std::chrono::duration<double>(deltaT).count());
            //when += deltaT;
        //}
#endif
    }
}
}

