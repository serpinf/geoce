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

#include "wsitem.h"
#include "wsfile.h"

class gceColumnSchema;

class gceColumnItem final : public gceWorkspaceItem
{
public:
    gceColumnItem(gceWorkspaceItem *parent, const gceColumnSchema *schema);
    ~gceColumnItem() override;

    wxIcon icon() const final;

    const std::string name() const override;

private:
    wxIcon m_icon;
    const gceColumnSchema *m_schema;
};
