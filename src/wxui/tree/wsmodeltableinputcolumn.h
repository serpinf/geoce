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

class gceModelTableInputColumnItem final : public gceWorkspaceItem
{
public:
    explicit gceModelTableInputColumnItem(gceWorkspaceItem *parent, const gceColumnSchema &schema, const gce::model_info &modelInfo, const gce::model_table_input_info &tableInfo);

    wxIcon icon() const final;

    bool isContainer() const final
    {
        return false;
    }

    void onMenu(gceWorkspaceItemActionParams par) final;

    const std::string name() const final;

    void processMsg(gceWorkspaceItemActionParams par, const gce::MessageInfo &msg) final;

private:
    const gceColumnSchema &m_schema;
    gce::model_info m_info;
};
