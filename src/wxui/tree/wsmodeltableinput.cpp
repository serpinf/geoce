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
#include "menucmd.h"
#include "wsmodeltableinput.h"
#include "wsmodeltableinputcolumn.h"
#include "models/modelschema.h"
#include "dlg/IconSingleChoiceDialog.h"

gceModelTableInputItem::gceModelTableInputItem(gceWorkspaceItem *parent, const gceModelTableInputSchema &inputSchema, const gceWorkspace &wsp, const gce::model_info &info)
    : gceWorkspaceItem(parent), m_inputSchema(inputSchema), m_modelInfo(info)
{
    m_typeSchema = wsp.findSchema(inputSchema.typeSchema);
    if (!m_typeSchema) throw std::runtime_error("invalid model table input schema: type schema not found");

    const auto &columns = m_typeSchema->getColumns();
    m_columns.reserve(columns.size());
    for (auto &col : columns)
    {
        m_columns.emplace_back(std::make_unique<gceModelTableInputColumnItem>(this, col, info, gce::model_table_input_info{}));
    }

    model_info_changed(wsp);
}
gceModelTableInputItem::~gceModelTableInputItem() {}

wxIcon gceModelTableInputItem::icon() const
{
    if (!m_icon.IsOk())
    {
        m_icon = m_typeSchema ? iconFromSVG(m_typeSchema->getIcon().c_str()) : wxArtProvider::GetIcon(wxART_QUESTION, wxART_MENU);
    }
    return m_icon;
}

void gceModelTableInputItem::onMenu(gceWorkspaceItemActionParams par)
{
    enum
    {
        Set = gceMenuCmds::IDM_ContextLow,
        Clear
    };
    gceMenuCmds menu;
    menu.add(Set, "Set...");
    menu.add(Clear, "Clear").Enable(m_tableInfo.has_value());
    auto selection = menu.GetPopupMenuSelectionFromUser(par.parent, Set, Clear);
    if (selection == Set)
    {
        const auto &tables = par.wsp.getTables();
        std::vector<gce::uuid> tableIds;
        std::vector<wxDataViewIconText> tableItems;
        for (auto &table : tables)
        {
            //if (table.id_schema == m_inputSchema.typeSchema)

            auto *typeSchema = par.wsp.findSchema(table.id_schema);
            if (typeSchema)
            {
                wxDataViewIconText item(to_wxstring(table.label), iconFromSVG(typeSchema->getIcon().c_str()));
                tableItems.push_back(item);
                tableIds.push_back(table.id_table);
            }
        }
        int sel = gceGetSingleChoiceIndex("Select table", "Tables", tableItems, 0, par.parent);
        if (sel >= 0)
        {
            gceContext::log_message("Selected table: {}", tables[sel].label);
            gce::model_info updatedModelInfo = m_modelInfo;
            updatedModelInfo.tableInputs[m_inputSchema.name].id_table = tableIds[sel];
            par.wsp.updateModel(updatedModelInfo);
            //par.model->ItemChanged(wxDataViewItem(this));
        }
        else
        {
            gceContext::log_message("No table selected");
        }
    }
    else if (selection == Clear)
    {
        gce::model_info updatedModelInfo = m_modelInfo;
        updatedModelInfo.tableInputs.erase(m_inputSchema.name);
        par.wsp.updateModel(updatedModelInfo);
    }
}
unsigned int gceModelTableInputItem::getChildren(wxDataViewItemArray &array) const
{
    array.reserve(m_columns.size());
    for (const auto &col : m_columns)
    {
        array.Add(wxDataViewItem(col.get()));
    }
    return static_cast<unsigned>(m_columns.size());
}
void gceModelTableInputItem::processMsg(gceWorkspaceItemActionParams par, const gce::MessageInfo &msg)
{
    if (std::holds_alternative<wspModelUpdatedMsg>(msg))
    {
        processMsg(par, std::get<wspModelUpdatedMsg>(msg));
    }
    // TODO: update column items
}

void gceModelTableInputItem::processMsg(gceWorkspaceItemActionParams par, const wspModelUpdatedMsg &msg)
{
    if (msg.info.id_model == m_modelInfo.id_model && m_modelInfo.tableInputs != msg.info.tableInputs)
    {
        m_modelInfo = msg.info;
        model_info_changed(par.wsp);
    }
    par.model->ItemChanged(wxDataViewItem(this));
}
void gceModelTableInputItem::model_info_changed(const gceWorkspace &wsp)
{
    if (auto it = m_modelInfo.tableInputs.find(m_inputSchema.name); it != m_modelInfo.tableInputs.end())
    {
        m_inputInfo = it->second;
        m_tableInfo = wsp.getTable(m_inputInfo->id_table);
    }
    else
    {
        m_inputInfo = std::nullopt;
        m_tableInfo = std::nullopt;
    }
}

const std::string gceModelTableInputItem::name() const
{
    return fmt::format("{} ({})", m_inputSchema.name, m_tableInfo ? m_tableInfo->label : "not set");
}
