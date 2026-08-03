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

class gceModelSchema;

class gceLayerItem final : public gceWorkspaceItem
{
public:
    explicit gceLayerItem(gceWorkspaceItem *parent, gceWorkspace &wsp, const gce::layer_info &info);

    wxIcon icon() const final;

    wxCheckBoxState isChecked() const final;

    bool GetAttr(gceWorkspace &wsp, wxDataViewItemAttr &attr) const final;
    bool isSceneVisible(gceWorkspace &wsp) const;
    void onMenu(gceWorkspaceItemActionParams par) final;
    void onActivated(gceWorkspaceItemActionParams par) final;
    const std::string name() const final;

    gce::uuid getId() const
    {
        return m_info.id_layer;
    }

    void processMsg(gceWorkspaceItemActionParams par, const gce::MessageInfo &msg) final;
    bool update(gceWorkspace &wsp, bool checked, const std::string &name) final;

    void removeTech(gceContext &ctx);

private:
    void resetModelAndTech(gceWorkspace &wsp);
    void resetTech(gceWorkspace &wsp);
    void updateTech(gceContext &ctx);

    void processLayerUpdatedMsg(gceWorkspaceItemActionParams par, const wspLayerUpdatedMsg &msg);
    void processModelUpdated(gceWorkspaceItemActionParams par, const wspModelUpdatedMsg &msg);
    void processSceneUpdatedMsg(gceWorkspaceItemActionParams par, const wspSceneUpdatedMsg &msg);

    gce::layer_info m_info;
    std::optional<gce::model_info> m_modelInfo;
    const gceModelSchema *m_modelSchema = nullptr;
};
