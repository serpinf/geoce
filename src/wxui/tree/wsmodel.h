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

class gceModelTableInputGroup;

class gceModelItem final : public gceWorkspaceItem
{
public:
    explicit gceModelItem(gceWorkspaceItem *parent, gceWorkspace &wsp, const gce::model_info &info);
    ~gceModelItem() override;
    unsigned int getChildren(wxDataViewItemArray &array) const final;

    wxIcon icon() const final;

    bool isContainer() const final
    {
        return true;
    }
    void onMenu(gceWorkspaceItemActionParams par) final;
    const std::string name() const override
    {
        return m_info.label;
    }

    void processMsg(gceWorkspaceItemActionParams par, const gce::MessageInfo &msg) final;

    gce::uuid getId() const
    {
        return m_info.get_key();
    }
private:
    void processMsg(gceWorkspaceItemActionParams par, const wspModelUpdatedMsg &msg);


    bool isConnectionOk() const
    {
        return m_status == gceEntityStatus::OK;
    }

    wxIcon m_icon0;
    gce::model_info m_info;
    const gceModelSchema *m_schema;
    gceEntityStatus m_status = gceEntityStatus::NONE;
    std::unique_ptr<gceModelTableInputGroup> m_tableInputs;
};
