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

class gceLayerItem;
class gceSceneGroup;
class gceSceneItem : public gceWorkspaceItem
{
public:
    explicit gceSceneItem(gceSceneGroup *parent, gceWorkspace &wsp, const gce::scene_info &info);
    ~gceSceneItem() noexcept;
    void onMenu(gceWorkspaceItemActionParams par) override;
    void onActivated(gceWorkspaceItemActionParams par) override;
    wxIcon icon() const final;
    wxCheckBoxState isChecked() const final;

    bool isContainer() const final
    {
        return true;
    }

    unsigned int getChildren(wxDataViewItemArray &array) const final;

    const std::string name() const override
    {
        return m_info.label;
    }
    gce::uuid getId() const
    {
        return m_info.id_scene;
    }
    bool update(gceWorkspace &wsp, bool checked, const std::string &name) override;
    void processMsg(gceWorkspaceItemActionParams par, const gce::MessageInfo &msg) override;
private:
    void processLayerDeletedMsg(gceWorkspaceItemActionParams par, const wspLayerDeletedMsg &msg);

    gce::scene_info m_info;
    std::vector<std::unique_ptr<gceLayerItem>> m_layers;
};
