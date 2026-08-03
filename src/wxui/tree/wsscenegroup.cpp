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
#include "gcprec.h"
#include "wsscenegroup.h"
#include "wsfile.h"
#include "wsscene.h"
#include "menucmd.h"

gceSceneGroup::gceSceneGroup(gceWorkspace &wsp) : gceWorkspaceItem(nullptr)
{
    if (wsp.isOk())
    {
        /*auto layers = wsp.getLayers();
        for (auto &layer : layers)
        {
            m_layerObjects.emplace_back(std::make_unique<gceLayerObject>(wsp, layer));
        }*/

        auto srcs = wsp.getScenes();
        for (auto &src : srcs)
        {
            m_scenes.emplace_back(std::make_unique<gceSceneItem>(this, wsp, src));
        }
    }
}
gceSceneGroup::~gceSceneGroup() {}

unsigned int gceSceneGroup::getChildren(wxDataViewItemArray &array) const
{
    array.reserve(m_scenes.size());
    for (const auto &sc : m_scenes)
    {
        array.Add(wxDataViewItem(sc.get()));
    }
    return static_cast<unsigned>(m_scenes.size());
}
void gceSceneGroup::onMenu(gceWorkspaceItemActionParams par)
{
    enum
    {
        Add = gceMenuCmds::IDM_ContextLow
    };
    gceMenuCmds menu;
    menu.add(Add, "Add scene...");
    auto selection = menu.GetPopupMenuSelectionFromUser(par.parent, Add);
    if (selection == Add)
    {
        wxString sceneName = wxGetTextFromUser(_("Enter scene name"), _("New scene"), "Scene", par.parent);
        if (!sceneName.empty())
        {
            gce::scene_info info{makeUUID(), 0, to_string(sceneName), "", ""};
            par.wsp.addScene(info);

            par.model->ItemChanged(wxDataViewItem(this));
            m_scenes.push_back(std::make_unique<gceSceneItem>(this, par.wsp, info));
            par.model->ItemAdded(wxDataViewItem(this), wxDataViewItem(m_scenes.back().get()));
        }
    }
}

const std::string gceSceneGroup::name() const
{
    return "Scenes";
}
wxIcon gceSceneGroup::icon() const
{
    return wxArtProvider::GetIcon(wxART_FLOPPY, wxART_MENU);
}

void gceSceneGroup::processMsg(gceWorkspaceItemActionParams par, const gce::MessageInfo &msg)
{
    if (std::holds_alternative<wspSceneDeletedMsg>(msg))
    {
        processSceneDeletedMsg(par, std::get<wspSceneDeletedMsg>(msg));
    }
    else
    {
        for (auto &s : m_scenes)
        {
            s->processMsg(par, msg);
        }
    }
}

void gceSceneGroup::processSceneDeletedMsg(gceWorkspaceItemActionParams par, const wspSceneDeletedMsg &msg)
{
    wxDataViewItem item;
    size_t count = gce::erase_if(m_scenes, [&msg, &item](std::unique_ptr<gceSceneItem> &obj){
        if (obj->getId() == msg.id_scene)
        {
            item = wxDataViewItem(obj.get());
            return true;
        }
        return false;
    });
    assert(count == 1);
    par.model->ItemDeleted(wxDataViewItem(this), item);
}
/*
gceLayerObject *gceSceneGroup::getLayerObject(const gce::uuid &id_layer)
{
    for (auto &lo : m_layerObjects)
    {
        if (lo->getId() == id_layer)
        {
            return lo.get();
        }
    }
    return nullptr;
}

gceLayerObject *gceSceneGroup::addLayerObject(gceWorkspace &wsp, const gce::layer_info &info)
{
    return m_layerObjects.emplace_back(std::make_unique<gceLayerObject>(wsp, info)).get();
}
*/
