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
#include "wsmodel.h"
#include "models/modelschema.h"
#include "menucmd.h"
#include "wsmodeltableinputgroup.h"

gceModelItem::gceModelItem(gceWorkspaceItem *parent, gceWorkspace &wsp, const gce::model_info &info) :
    gceWorkspaceItem(parent), m_info(info)
{
    auto *schema = wsp.findModelSchema(info.type);
    if (schema == nullptr) throw std::runtime_error("Model schema not found: " + info.type);
    m_schema = schema;
    m_tableInputs = std::make_unique<gceModelTableInputGroup>(this, wsp, info);

    wsp.ctx().postModelQueue(umodelReplaceMsg{m_info});
}
gceModelItem::~gceModelItem() {}

unsigned int gceModelItem::getChildren(wxDataViewItemArray &array) const
{
    array.reserve(2);
    array.Add(wxDataViewItem(m_tableInputs.get()));
    return 1;
}

wxIcon gceModelItem::icon() const
{
    if (m_schema && !m_schema->m_svgIcon.empty())
    {
        return iconFromSVG(m_schema->m_svgIcon.c_str());
    }
    return wxArtProvider::GetIcon(wxART_ERROR, wxART_MENU);
}

void gceModelItem::onMenu(gceWorkspaceItemActionParams par)
{
    enum
    {
        Add = gceMenuCmds::IDM_ContextLow,
        Info,
    };
    gceMenuCmds menu("Model");
    menu.add(Info, "Info");
    auto selection = menu.GetPopupMenuSelectionFromUser(par.parent, Info, wxID_SEPARATOR, wxID_EDIT, wxID_DELETE);
    if (selection == Info)
    {
        // Show info dialog or perform info action
    }
    else if (selection == wxID_EDIT)
    {
    }
    else if (selection == wxID_DELETE)
    {
        par.wsp.removeModel(m_info);
        // actual delete is done later by message processor
    }
}
void gceModelItem::processMsg(gceWorkspaceItemActionParams par, const gce::MessageInfo &msg)
{
    if (std::holds_alternative<wspModelUpdatedMsg>(msg))
    {
        processMsg(par, std::get<wspModelUpdatedMsg>(msg));
    }
    m_tableInputs->processMsg(par, msg);
}

void gceModelItem::processMsg(gceWorkspaceItemActionParams par, const wspModelUpdatedMsg &msg)
{
    if (msg.info.id_model == m_info.id_model)
    {
        m_info = msg.info;
        par.model->ItemChanged(wxDataViewItem(this));
    }
}
