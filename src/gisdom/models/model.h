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

#include "engine.hpp"
#include "modelschema.h"

struct umodelUpdateMsg;
struct udataSelectReplyMsg;
struct udataMultiRowActionNotifyMsg;
struct umodelSelectXDResultMsg;
struct umodelEntityAddMsg;
struct umodelEntityUpdateMsg;
struct umodelEntityRemoveMsg;
struct umodelSelect2DMsg;

class gceModelBase
{
public:
    gceModelBase(const gce::model_info &info) : id_model(info.id_model)
    {}
    virtual ~gceModelBase() = default;
    virtual void processUpdate(gceContext &ctx, const umodelUpdateMsg &msg);
    virtual void processIdle(gceContext &) {}
    virtual void queryMesh2D(gceContext &ctx, gce::tileid tile);
    virtual void processTableStatus(gceContext &ctx, const udataTableStatusMsg &msg);
    virtual void processSelectResult(gceContext &ctx, const udataSelectReplyMsg &msg);
    virtual void processActionNotify(gceContext &ctx, const udataMultiRowActionNotifyMsg &msg);
    virtual void processSelect2D(const umodelSelect2DMsg &aoi, umodelSelectXDResultMsg &rmsg);

    virtual void processEntityAdd(gceContext &ctx, const umodelEntityAddMsg &msg);
    virtual void processEntityUpdate(gceContext &ctx, const umodelEntityUpdateMsg &msg);
    virtual void processEntityRemove(gceContext &ctx, const umodelEntityRemoveMsg &msg);
    virtual void processQueryImageTile(gceContext &ctx, const umodelQueryTileImageMsg &msg);
    virtual void processQueryDEMTile(gceContext &ctx, const umodelQueryTileDEMMsg &msg);

    bool isOk() const
    {
        return m_status == gceEntityStatus::OK;
    }
    gce::uuid id_model = {};
    gceEntityStatus m_status = gceEntityStatus::NONE;
};

class gceModelTableBased : public gceModelBase
{
public:
    gceModelTableBased(const gce::model_info &info) : gceModelBase(info)
    {}
    void processIdle(gceContext &ctx) override;
    void processTableStatus(gceContext &ctx, const udataTableStatusMsg &msg) override;

    void testTable(gceContext &ctx);
    void postSelect(gceContext &ctx);
    gce::uuid id_table{};
    bool tableStatusRequested = false;
    bool dataRequested = false;
};
