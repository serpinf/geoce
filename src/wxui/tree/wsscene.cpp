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
#include "wslayer.hpp"
#include "wsscene.h"
#include "menucmd.h"
#include "wsscenegroup.h"

gceSceneItem::gceSceneItem(gceSceneGroup *parent, gceWorkspace &wsp, const gce::scene_info &info) : gceWorkspaceItem(parent), m_info(info)
{
    const auto &layers = wsp.getSceneLayers(m_info.id_scene);
    m_layers.reserve(layers.size());
    for (const auto &layer : layers)
    {
        m_layers.emplace_back(std::make_unique<gceLayerItem>(this, wsp, layer));
    }
}

gceSceneItem::~gceSceneItem() noexcept
{}

unsigned int gceSceneItem::getChildren(wxDataViewItemArray &array) const
{
    array.reserve(m_layers.size());
    for (const auto &item : m_layers)
    {
        array.Add(wxDataViewItem(item.get()));
    }
    return static_cast<unsigned>(array.size());
}

void gceSceneItem::onMenu(gceWorkspaceItemActionParams par)
{
    enum
    {
        Add = gceMenuCmds::IDM_ContextLow,
        DeleteScene,
        Rename
    };
    gceMenuCmds menu;
    menu.add(Rename, _("Rename..."));
    menu.add(Add, _("Add layer..."));
    menu.add(DeleteScene, _("Delete scene"));
    auto selection = menu.GetPopupMenuSelectionFromUser(par.parent, Rename, Add, wxID_SEPARATOR, DeleteScene);
    if (selection == Rename)
    {
        wxString label = wxGetTextFromUser(_("Enter new scene name"), _("Scene"), m_info.label, par.parent);
        if (!label.empty() && label != m_info.label)
        {
            m_info.label = to_string(label);
            par.wsp.updateScene(m_info);
            par.model->ItemChanged(wxDataViewItem(this));
        }
    }
    else if (selection == Add)
    {
        gce::layer_info layer;
        layer.id_layer = makeUUID();
        layer.id_scene = m_info.id_scene;
        layer.label = "new layer";
        layer.visible = true;

        par.wsp.addLayer(layer);

        auto &layerItem = m_layers.emplace_back(std::make_unique<gceLayerItem>(this, par.wsp, layer));
        par.model->ItemAdded(wxDataViewItem(this), wxDataViewItem(layerItem.get()));
    }
    else if (selection == DeleteScene)
    {
        if (!m_layers.empty())
        {
            if (wxYES == wxMessageBox("Delete scene with all layers?", "Scene", wxYES_NO | wxNO_DEFAULT | wxICON_WARNING))
            {
                par.wsp.removeScene(m_info, true);
            }
        }
        else
        {
            par.wsp.removeScene(m_info, false);
        }
    }
}

void gceSceneItem::onActivated(gceWorkspaceItemActionParams)
{}

wxIcon gceSceneItem::icon() const
{
    return wxArtProvider::GetIcon(wxART_FOLDER_OPEN, wxART_MENU);
}

wxCheckBoxState gceSceneItem::isChecked() const
{
    return m_info.visible ? wxCHK_CHECKED : wxCHK_UNCHECKED;
}

bool gceSceneItem::update(gceWorkspace &wsp, bool checked, const std::string &)
{
    if (m_info.visible != checked)
    {
        this->m_info.visible = checked;
        wsp.updateScene(this->m_info);
        return true;
    }
    return false;
}

void gceSceneItem::processMsg(gceWorkspaceItemActionParams par, const gce::MessageInfo &msg)
{
    if (std::holds_alternative<wspLayerDeletedMsg>(msg))
    {
        processLayerDeletedMsg(par, std::get<wspLayerDeletedMsg>(msg));
    }
    else
    {
        for (auto &layerItem : m_layers)
        {
            layerItem->processMsg(par, msg);
        }
    }
}

void gceSceneItem::processLayerDeletedMsg(gceWorkspaceItemActionParams par, const wspLayerDeletedMsg &msg)
{
    // Find the layer item, remove it and notify the model only if found.
    wxDataViewItem item;
    auto it = std::find_if(m_layers.begin(), m_layers.end(), [&msg](const std::unique_ptr<gceLayerItem> &obj){
        return obj->getId() == msg.id_layer;
    });

    if (it != m_layers.end())
    {
        (*it)->removeTech(par.wsp.ctx());
        item = wxDataViewItem(it->get());
        m_layers.erase(it);
        par.model->ItemDeleted(wxDataViewItem(this), item);
    }
}
