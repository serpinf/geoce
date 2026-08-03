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
#include "model.h" 
#include <set>

class gceEntityPacked;

class gceModelImageTiled : public gceModelTableBased
{
public:
    gceModelImageTiled(const gce::model_info &info);

    void processSelectResult(gceContext &ctx, const udataSelectReplyMsg &msg) override;

    void queryMesh2D(gceContext &ctx, gce::tileid tile) override;
    void processQueryImageTile(gceContext &ctx, const umodelQueryTileImageMsg &msg) override;
    void processQueryDEMTile(gceContext &ctx, const umodelQueryTileDEMMsg &msg) override;

    static std::unique_ptr<const gceModelSchema> schema();

private:
    bool isTableDEMOK() const
    {
        return !id_tableDEM.is_nil();
    }
    void postTileImage(const gceEntityPacked &row, gceContext &ctx);
    void postTileError(const gce::tileid tile, gceContext &ctx);

    void postTileDEM(const gceEntityPacked &row, gceContext &ctx);
    void postTileDEMError(const gce::tileid tile, gceContext &ctx);

    bool postQueryImage(gceContext &ctx, gce::tileid tile);
    bool postQueryDEM(gceContext &ctx, gce::tileid tile);

    gce::uuid id_tableDEM{};
    std::set<gce::tileid> m_queries;
    std::set<gce::tileid> m_queriesDEM;
};

