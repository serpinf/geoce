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
#include "model.h"

void gceModelBase::processUpdate(gceContext &, const umodelUpdateMsg &)
{}


void gceModelBase::queryMesh2D(gceContext &ctx, gce::tileid tile) {}

//////////////////////////////////////
// gceModelBase
//////////////////////////////////////
void gceModelBase::processTableStatus(gceContext &, const udataTableStatusMsg &)
{}

void gceModelBase::processSelectResult(gceContext &, const udataSelectReplyMsg &)
{}

void gceModelBase::processActionNotify(gceContext &, const udataMultiRowActionNotifyMsg &)
{}

void gceModelBase::processSelect2D(const umodelSelect2DMsg &, umodelSelectXDResultMsg &)
{}

void gceModelBase::processEntityAdd(gceContext &, const umodelEntityAddMsg &)
{}

void gceModelBase::processEntityUpdate(gceContext &, const umodelEntityUpdateMsg &)
{}

void gceModelBase::processEntityRemove(gceContext &, const umodelEntityRemoveMsg &)
{}

void gceModelBase::processQueryImageTile(gceContext &, const umodelQueryTileImageMsg &)
{}

void gceModelBase::processQueryDEMTile(gceContext &, const umodelQueryTileDEMMsg &)
{}

//////////////////////////////////////
// gceModelTableBased
//////////////////////////////////////
void gceModelTableBased::processIdle(gceContext &ctx)
{
    if (!tableStatusRequested)
    {
        testTable(ctx);
        tableStatusRequested = true;
    }

    if (m_status == gceEntityStatus::OK && !this->dataRequested)
    {
        postSelect(ctx);
        this->dataRequested = true;
    }
}
void gceModelTableBased::processTableStatus(gceContext &ctx, const udataTableStatusMsg &msg)
{
    if (msg.id_table == this->id_table && msg.result)
    {
        m_status = gceEntityStatus::OK;
    }
}
void gceModelTableBased::testTable(gceContext &ctx)
{
    ctx.postDataQueue(udataSelectTestMsg{id_table, gce::queueId::MODEL});
}

void gceModelTableBased::postSelect(gceContext &ctx)
{
    udataSelectAllMsg msg;
    msg.id_table = id_table;
    msg.sender = gce::queueId::MODEL;
    ctx.postDataQueue(msg);
}
