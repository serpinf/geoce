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
#include <map>
#include "idxpool.h"
#include "alg/geoproj.h"

class gceEntityPacked;

class gceModelGeometric : public gceModelTableBased
{
public:
    gceModelGeometric(const gce::model_info &info);

    void processSelectResult(gceContext &ctx, const udataSelectReplyMsg &msg) override;
    void processActionNotify(gceContext &ctx, const udataMultiRowActionNotifyMsg &msg) override;
    bool addEntity(const gceEntityPacked &en, uint32_t &index);
    bool setEntry(uint32_t index, const gceEntityPacked &ref);
    bool updateEntity(gceContext &ctx, const gceEntityPacked &en);
    bool removeEntity(gceContext &ctx, const gceEntityPacked &en);

    void queryMesh2D(gceContext &ctx, gce::tileid tile) override;
    void processSelect2D(const umodelSelect2DMsg &aoi, umodelSelectXDResultMsg &rmsg) override;

    void postMesh(gceContext &ctx, int index);
    void postRemoveMesh(gceContext &ctx, uint32_t index);

    std::map<int64_t, uint32_t> m_index;
    struct entry_type
    {
        std::unique_ptr<geom::Geometry> g;
    };
    gce::idxalloca<entry_type> m_data{1000};
    bool meshSent = false;
    std::unique_ptr<gceProjection> m_geoProj;

    static std::unique_ptr<const gceModelSchema> schema();
};

