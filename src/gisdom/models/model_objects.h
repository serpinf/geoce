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
#include "alg/geoproj.h"

class gceEntityPacked;

class gceModelObjects : public gceModelBase
{
public:
    gceModelObjects(const gce::model_info &info);

    void postFlat(gceContext &ctx);
    void postGlobe(gceContext &ctx);

    void processIdle(gceContext &ctx) override;

    void queryMesh2D(gceContext &ctx, gce::tileid tile) override;
    //void processSelect2D(const umodelSelect2DMsg &aoi, umodelSelectXDResultMsg &rmsg) override;

    std::map<int64_t, uint32_t> m_index;
    struct entry_type
    {
        glm::dvec3 wgspos;
        glm::dvec2 v;
    };
    bool meshSent2 = false;
    bool meshSent3 = false;
    std::unique_ptr<gceProjection> m_geoProj;

    static std::unique_ptr<const gceModelSchema> schema();
    int cnt = 0;

    constexpr static size_t maxInstances = 10000;
    std::array<entry_type, maxInstances> m_data;
};

