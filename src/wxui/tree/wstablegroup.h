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
class gceTableItem;

struct wspTableDeletedMsg;

class gceTableGroup final : public gceWorkspaceItem
{
public:
    explicit gceTableGroup(gceWorkspaceItem *parent, const gce::datasrc_info &info);
    ~gceTableGroup() override;
    unsigned int getChildren(wxDataViewItemArray &array) const final;
    void onMenu(gceWorkspaceItemActionParams par) final;

    bool isContainer() const override
    {
        return true;
    }

    const std::string name() const final
    {
        return "Tables";
    }

    wxIcon icon() const final;

    void processInit(gceWorkspaceItemActionParams par);
    void processMsg(gceWorkspaceItemActionParams par, const gce::MessageInfo &msg) override;
private:
    void processTableDeletedMsg(gceWorkspaceItemActionParams par, const wspTableDeletedMsg &msg);

    const gce::datasrc_info m_info;
    std::vector<std::unique_ptr<gceTableItem>> m_tables;
};
