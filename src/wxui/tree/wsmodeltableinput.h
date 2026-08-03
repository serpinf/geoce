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

struct gceModelTableInputSchema;
class gceModelTableInputColumnItem;

class gceModelTableInputItem final : public gceWorkspaceItem
{
public:
    explicit gceModelTableInputItem(gceWorkspaceItem *parent, const gceModelTableInputSchema &inputSchema, const gceWorkspace &wsp, const gce::model_info &info);
    void model_info_changed(const gceWorkspace &wsp);
    ~gceModelTableInputItem() override;

    wxIcon icon() const final;

    void onMenu(gceWorkspaceItemActionParams par) final;
    const std::string name() const override;

    bool isContainer() const final
    {
        return true;
    }

    unsigned int getChildren(wxDataViewItemArray &array) const override;

    void processMsg(gceWorkspaceItemActionParams par, const gce::MessageInfo &msg) final;

private:
    void processMsg(gceWorkspaceItemActionParams par, const wspModelUpdatedMsg &msg);
    const gceModelTableInputSchema &m_inputSchema;
    const gceTypeSchema *m_typeSchema = nullptr;
    mutable wxIcon m_icon;
    gce::model_info m_modelInfo;
    std::optional<gce::table_info> m_tableInfo;
    std::optional<gce::model_table_input_info> m_inputInfo;
    gceEntityStatus m_status = gceEntityStatus::NONE;
    std::vector<std::unique_ptr<gceModelTableInputColumnItem>> m_columns;
};
