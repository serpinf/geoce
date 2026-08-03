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

class gceModelBase;

struct gceModelTableInputSchema
{
    gceModelTableInputSchema(const std::string &name, const gce::uuid &typeSchema) : name(name), typeSchema(typeSchema)
    {}

    std::string name;
    gce::uuid typeSchema;
};

struct gceModelModelInputSchema
{
    std::string name;
};

using gceModelFactoryFn = std::unique_ptr<gceModelBase>(*)(const gce::model_info &);

class gceModelSchema
{
public:
    gceModelSchema() = default;
    ~gceModelSchema() = default;

    const std::string &getName() const
    {
        return m_name;
    }

    std::string m_name;
    std::string m_svgIcon;
    gceModelFactoryFn m_factory = nullptr;
    std::vector<gceModelTableInputSchema> m_tableInputs;
    std::vector<gceModelModelInputSchema> m_modelInputs;
    gceTechType m_techFlat = gceTechType::NONE;
    gceTechType m_techGlobe = gceTechType::NONE;
    bool internal = false;
};
