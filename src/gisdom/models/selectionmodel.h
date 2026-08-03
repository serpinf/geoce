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
#include "node/umodelnode.h"
#include <map>
#include "idxpool.h"
#include "alg/geoproj.h"
#include "teches/techbasic.h"



struct gceSelectionModelProprties : public umodelCustomPropertyBase
{
    bool drawPoints = false;
    glm::dmat4 pose{1.0};
    std::vector<glm::dmat4> poseArray;
};

class gceSelectionModel : public gceModelBase
{
public:
    gceSelectionModel(const gce::model_info &info);

    void processUpdate(gceContext &ctx, const umodelUpdateMsg &msg) override;
    void queryMesh2D(gceContext &, gce::tileid) override;
    void processActionNotify(gceContext &ctx, const udataMultiRowActionNotifyMsg &msg) override;
    void processEntityAdd(gceContext &ctx, const umodelEntityAddMsg &msg) override;
    void processEntityUpdate(gceContext &ctx, const umodelEntityUpdateMsg &msg) override;
    void processEntityRemove(gceContext &ctx, const umodelEntityRemoveMsg &msg) override;
    bool addEntity(gceContext &ctx, const gceEntityPackedRef &ref);
    bool setEntry(uint32_t index, const gceEntityPackedRef &ref);
    bool updateEntity(gceContext &ctx, const gceEntityPackedRef &ref);
    bool removeEntity(gceContext &ctx, const gceEntityPackedRef &ref);
    bool removeEntityAll(gceContext &ctx);
    void postArray(tech_basic::ArrayInfo &info, uint32_t index);
    void postMesh(tech_basic::MeshInfo &info, uint32_t index);
    void postInstance(tech_basic::InstanceInfo &info, uint32_t index);

    void processSelect2D(const umodelSelect2DMsg &msg, umodelSelectXDResultMsg &rmsg) override;

    static std::unique_ptr<const gceModelSchema> schema();

    using key_type = gceEntitySetKey;

    struct indexes_type
    {
        uint32_t id_mesh = 0;
        uint32_t id_instance = 0;
        std::vector<uint32_t> extraInstances;
    };
    std::map<key_type, indexes_type> m_index;

    void postRemoveMesh(gceContext &ctx, const indexes_type &i);
    void free_indexes(indexes_type &i);

    struct entry_type
    {
        std::unique_ptr<geom::Geometry> g;
        glm::fvec4 color{1.0f, 0.0f, 1.0f, 1.0f};
    };

    gce::idxalloca<entry_type> m_data{1000};
    gce::idxpool<> m_instances{10000};
    bool arraysSent = false;
    bool meshSent = false;
    bool instanceSent = false;
    //bool posesChanged = false;
    std::unique_ptr<gceProjection> m_geoProj;

    gceSelectionModelProprties m_props;
};
