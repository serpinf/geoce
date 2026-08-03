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

#include <wx/numdlg.h>
#include "wslayer.hpp"
#include "menucmd.h"
#include "models/modelschema.h"
#include "dlg/IconSingleChoiceDialog.h"

gceLayerItem::gceLayerItem(gceWorkspaceItem *parent, gceWorkspace &wsp, const gce::layer_info &info)
    : gceWorkspaceItem(parent), m_info(info)
{
    resetModelAndTech(wsp);
}

wxIcon gceLayerItem::icon() const
{
    if (m_modelSchema != nullptr)
    {
        return iconFromSVG(m_modelSchema->m_svgIcon);
    }
    return wxArtProvider::GetIcon(wxART_QUESTION, wxART_MENU);
}

wxCheckBoxState gceLayerItem::isChecked() const
{
    return m_info.visible ? wxCHK_CHECKED : wxCHK_UNCHECKED;
}

bool gceLayerItem::GetAttr(gceWorkspace &wsp, wxDataViewItemAttr &attr) const
{
    bool res = false;
    if (!isSceneVisible(wsp))
    {
        attr.SetItalic(true);
        attr.SetColour(*wxLIGHT_GREY);
        res = true;
    }
    return res;
}

bool gceLayerItem::isSceneVisible(gceWorkspace &wsp) const
{
    auto scene = wsp.getScene(this->m_info.id_scene);
    return scene && scene->visible;
}

void gceLayerItem::onMenu(gceWorkspaceItemActionParams par)
{
    enum
    {
        Rename = gceMenuCmds::IDM_ContextLow,
        Delete,
        SetModel,
        SetOrder
    };

    gceMenuCmds menu(to_wxstring(fmt::format("LayerItem:{}", m_info.order)));

    menu.add(Rename, "Rename...").Enable(true);
    menu.add(Delete, "Delete").Enable(true);
    menu.add(SetModel, "Set model...").Enable(true);
    menu.add(SetOrder, "Set order...").Enable(true);

    auto selection = menu.GetPopupMenuSelectionFromUser(par.parent, Rename, wxID_SEPARATOR, Delete, wxID_SEPARATOR, SetModel, SetOrder);
    if (selection == Rename)
    {
        wxString label = wxGetTextFromUser(_("Enter new layer name"), _("Layer"), to_wxstring(m_info.label), par.parent);
        if (!label.empty() && to_string(label) != m_info.label)
        {
            auto newInfo = m_info;
            newInfo.label = to_string(label);
            par.wsp.updateLayer(newInfo);
        }
    }
    else if (selection == Delete)
    {
        par.wsp.removeLayer(m_info);
    }
    else if (selection == SetModel)
    {
        std::vector<wxDataViewIconText> model_items;
        const auto &models = par.wsp.getModels();
        model_items.reserve(models.size());
        for (auto &model : models)
        {
            const gceModelSchema *modelScema = par.wsp.findModelSchema(model.type);
            wxIcon icon = modelScema ? iconFromSVG(modelScema->m_svgIcon) : wxIcon();
            model_items.emplace_back(to_wxstring(model.label), icon);
        }

        if (int choice = gceGetSingleChoiceIndex("Select model", "Models", model_items, 0, par.parent); choice > -1)
        {
            if (m_info.id_model != models[choice].id_model)
            {
                gce::layer_info newLayerInfo = m_info;
                newLayerInfo.id_model = models[choice].id_model;
                par.wsp.updateLayer(newLayerInfo);
            }
        }
    }
    else if (selection == SetOrder)
    {
        int order = (int)wxGetNumberFromUser("Set layer order", "", "", m_info.order);
        if (order != m_info.order)
        {
            gce::layer_info newLayerInfo = m_info;
            newLayerInfo.order = order;
            par.wsp.updateLayer(newLayerInfo);
        }
    }
}

void gceLayerItem::onActivated(gceWorkspaceItemActionParams par)
{
    gceContext::log_message("Activate layer: {}", m_info.label);

    wspActiveLayerMsg msg;

    msg.info.id_layer = m_info.id_layer;
    msg.info.id_model = m_info.id_model;
    if (m_modelInfo && !m_modelInfo->tableInputs.empty())
    {
        // TODO: use passthrough table input name from model info instead of first table input
        if (auto tableInfo = par.wsp.getTable(m_modelInfo->tableInputs.begin()->second.id_table))
        {
            msg.info.id_table = tableInfo->id_table;
            msg.info.id_schema = tableInfo->id_schema;
        }
    }
    msg.info.id_scene = m_info.id_scene;
    msg.info.layerName = m_info.label;
    if (auto pSceneInfo = par.wsp.getScene(m_info.id_scene))
    {
        msg.info.sceneName = pSceneInfo->label;
    }
    par.wsp.ctx().postWorkspaceQueue(std::move(msg));
}

const std::string gceLayerItem::name() const
{
    std::string modelLabel = m_modelInfo ? m_modelInfo->label : "not set";
    return fmt::format("({}){} [{}]", m_info.order, m_info.label, modelLabel);
}

void gceLayerItem::processMsg(gceWorkspaceItemActionParams par, const gce::MessageInfo &msg)
{
    if (std::holds_alternative<wspLayerUpdatedMsg>(msg))
    {
        processLayerUpdatedMsg(par, std::get<wspLayerUpdatedMsg>(msg));
    }
    else if (std::holds_alternative<wspSceneUpdatedMsg>(msg))
    {
        processSceneUpdatedMsg(par, std::get<wspSceneUpdatedMsg>(msg));
    }
    else if (std::holds_alternative<wspModelUpdatedMsg>(msg))
    {
        processModelUpdated(par, std::get<wspModelUpdatedMsg>(msg));
    }
}
void gceLayerItem::processLayerUpdatedMsg(gceWorkspaceItemActionParams par, const wspLayerUpdatedMsg &msg)
{
    if (m_info.id_layer == msg.info.id_layer)
    {
        bool needResetTech = m_info.id_model != msg.info.id_model;
        bool needUpdateTech = !needResetTech && (m_info.visible != msg.info.visible || m_info.order != msg.info.order);

        m_info = msg.info;

        if (needResetTech)
        {
            resetModelAndTech(par.wsp);
        }
        if (needUpdateTech)
        {
            updateTech(par.wsp.ctx());
        }
        par.model->ItemChanged(wxDataViewItem(this));
    }
}
void gceLayerItem::processModelUpdated(gceWorkspaceItemActionParams par, const wspModelUpdatedMsg &msg)
{
    if (m_info.id_model == msg.info.id_model)
    {
        if (m_modelInfo->label != msg.info.label)
        {
            m_modelInfo->label = msg.info.label;
            par.model->ItemChanged(wxDataViewItem(this));
        }
        if (m_modelInfo->tableInputs != msg.info.tableInputs || m_modelInfo->modelInputs != msg.info.modelInputs)
        {
            m_modelInfo = msg.info;
            resetModelAndTech(par.wsp);
        }
        m_modelInfo = msg.info;
        m_modelSchema = m_modelInfo ? par.wsp.findModelSchema(m_modelInfo->type) : nullptr;
        updateTech(par.wsp.ctx());
        par.model->ItemChanged(wxDataViewItem(this));
    }
    //m_layer->processModelUpdated(par, msg);
}

void gceLayerItem::processSceneUpdatedMsg(gceWorkspaceItemActionParams par, const wspSceneUpdatedMsg &msg)
{
    if (this->m_info.id_scene == msg.info.id_scene)
    {
        updateTech(par.wsp.ctx());
        par.model->ItemChanged(wxDataViewItem(this));
    }
}

bool gceLayerItem::update(gceWorkspace &wsp, bool checked, const std::string & /*name*/)
{
    if (m_info.visible != checked)
    {
        m_info.visible = checked;
        wsp.updateLayer(m_info);

        updateTech(wsp.ctx());
        return true;
    }
    return false;
}

void gceLayerItem::removeTech(gceContext &ctx)
{
    ctx.postRenderQueue(urenderRemoveTechMsg{m_info.id_layer});
}

void gceLayerItem::updateTech(gceContext &ctx)
{
    urenderUpdateTechMsg msg;
    msg.id_layer = m_info.id_layer;
    msg.visible = m_info.visible;
    msg.order = m_info.order;
    ctx.postRenderQueue(msg);
}

void gceLayerItem::resetModelAndTech(gceWorkspace &wsp)
{
    m_modelInfo = wsp.getModel(m_info.id_model);
    m_modelSchema = m_modelInfo ? wsp.findModelSchema(m_modelInfo->type) : nullptr;
    resetTech(wsp);
}

void gceLayerItem::resetTech(gceWorkspace &wsp)
{
    gceTechType techFlat = gceTechType::NONE;
    gceTechType techGlobe = gceTechType::NONE;
    if (m_modelSchema)
    {
        techFlat = m_modelSchema->m_techFlat;
        techGlobe = m_modelSchema->m_techGlobe;
    }

    urenderReplaceTechMsg rendermsg;
    rendermsg.techFlat = techFlat;
    rendermsg.techGlobe = techGlobe;
    rendermsg.id_layer = m_info.id_layer;
    rendermsg.id_model = m_info.id_model;
    rendermsg.order = m_info.order;
    rendermsg.visible = m_info.visible;

    wsp.ctx().postRenderQueue(std::move(rendermsg));
}

