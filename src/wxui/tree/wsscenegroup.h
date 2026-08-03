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
#include "wsitem.h"
class gceWorkspace;
class gceSceneItem;
class gceLayerObject;

class gceSceneGroup final : public gceWorkspaceItem
{
public:
    explicit gceSceneGroup(gceWorkspace &wsp);
    ~gceSceneGroup() override;
    unsigned int getChildren(wxDataViewItemArray &array) const final;

    bool isContainer() const override
    {
        return !m_scenes.empty();
    }

    void onMenu(gceWorkspaceItemActionParams par) final;

    const std::string name() const final;
    wxIcon icon() const final;
    void processMsg(gceWorkspaceItemActionParams par, const gce::MessageInfo &msg) final;
    void processSceneDeletedMsg(gceWorkspaceItemActionParams par, const wspSceneDeletedMsg &msg);

    //gceLayerObject *getLayerObject(const gce::uuid &id_layer);
    //gceLayerObject *addLayerObject(gceWorkspace &wsp, const gce::layer_info &info);


    //std::vector<std::unique_ptr<gceLayerObject>> m_layerObjects;
    std::vector<std::unique_ptr<gceSceneItem>> m_scenes;
};
