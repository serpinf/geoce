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
class gceWorkspace;
class gceModelItem;

class gceModelGroup final : public gceWorkspaceItem
{
public:
    explicit gceModelGroup(gceWorkspace &wsp);
    ~gceModelGroup() override;
    unsigned int getChildren(wxDataViewItemArray &array) const final;
    void onMenu(gceWorkspaceItemActionParams par) final;

    bool isContainer() const override
    {
        return true;
    }

    const std::string name() const final;
    wxIcon icon() const final;

    void processMsg(gceWorkspaceItemActionParams par, const gce::MessageInfo &msg) final;

    void processMsg(gceWorkspaceItemActionParams par, const wspModelDeletedMsg &msg);

    std::vector<std::unique_ptr<gceModelItem>> m_models;
};
