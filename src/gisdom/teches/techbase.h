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
#include "geom/aabb.h"

class gceTechBase
{
public:
    gceTechBase(gceTechType techType, gceRenderMode techMode) : m_type(techType), m_mode(techMode) {}
    virtual ~gceTechBase() = default;

    virtual void processMsg(const urenderTechDataMsg &msg);

    void queryModelStatus(gceContext &ctx);

    const gceTechType m_type;
    const gceRenderMode m_mode;
    gce::uuid id_layer = {};
    gce::uuid id_model = {};
};

class gceTechFlat : public gceTechBase
{
public:
    gceTechFlat(gceTechType techType) : gceTechBase(techType, gceRenderMode::FLAT) {}

    virtual void prepareScene(gceContext &ctx, const glm::dmat4 &proj, const geom::aabb2 &aoi, const glm::ivec2 &res) = 0;
    virtual void renderScene(const glm::dmat4 &proj, const geom::aabb2 &aoi, const glm::ivec2 &res) = 0;
};

class gceTechGlobe : public gceTechBase
{
public:
    gceTechGlobe(gceTechType techType) : gceTechBase(techType, gceRenderMode::GLOBE) {}

    virtual void prepareScene(gceContext &ctx, const glm::dmat4 &proj, const glm::dvec3 &pos, const glm::ivec2 &res) = 0;
    virtual void renderScene(const glm::dmat4 &proj, const glm::dvec3 &pos, const glm::ivec2 &res, bool wireFrameTerrain) = 0;
};
