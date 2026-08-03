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

#include "wstable.hpp"
#include "wstablegroup.h"
#include "menucmd.h"
#include "wscolumngroup.h"

gceTableItem::gceTableItem(gceTableGroup *parent, gce::table_info &table, const gceTypeSchema *schema) : gceWorkspaceItem(parent),
m_info(table),
m_schema(schema)
{
    m_columnsGroup = std::make_unique<gceColumnGroup>(this, schema);
}
wxIcon gceTableItem::icon() const
{
    if (m_schema)
    {
        if (m_status)
        {
            return iconFromSVG(m_schema->getIcon().c_str());
        }
        if (m_info.name.empty())
        {
            return wxArtProvider::GetIcon(wxART_QUESTION, wxART_MENU);
        }
        return wxArtProvider::GetIcon(wxART_MISSING_IMAGE, wxART_MENU);
    }
    return wxArtProvider::GetIcon(wxART_QUESTION, wxART_MENU);
}

unsigned int gceTableItem::getChildren(wxDataViewItemArray &array) const
{
    array.push_back(wxDataViewItem(m_columnsGroup.get()));
    return 1;
}

void gceTableItem::onMenu(gceWorkspaceItemActionParams par)
{
    enum
    {
        Add = gceMenuCmds::IDM_ContextLow,
        Info,
        Rename,
        ViewData,
        Delete,
        DeleteAndDrop,
        Unlink,
        Link
    };
    gceMenuCmds menu;

    menu.add(Info, "Information").Enable(true);
    menu.add(Rename, "Rename...").Enable(true);
    menu.add(ViewData, "View data...").Enable(isOk());
    menu.add(Delete, "Delete").Enable(true);
    menu.add(DeleteAndDrop, "Delete and drop table").Enable(isOk());
    menu.add(Unlink, "Unlink table").Enable(!m_info.name.empty());
    menu.add(Link, "Link table...").Enable(true);
    auto selection = menu.GetPopupMenuSelectionFromUser(par.parent, Info, Rename, ViewData, wxID_SEPARATOR, Unlink, Link, Delete, DeleteAndDrop);
    if (selection == Info)
    {
        wxString info = fmt::format("Label: {}\nTable name: {}.{}\nSchema: {}", m_info.label, m_info.dbSchema, m_info.name, par.wsp.getSchemaName(m_info.id_schema));
        wxMessageDialog dlg1(par.parent, info, _("Table info"));
        dlg1.ShowModal();
    }
    else if (selection == Rename)
    {
        wxString label = wxGetTextFromUser(_("Enter new table name"), _("Table"), m_info.label, par.parent);
        if (!label.empty() && label != m_info.label)
        {
            auto info = m_info;
            info.label = to_string(label);
            par.wsp.updateTable(info);
            //par.model->ItemChanged(wxDataViewItem(this));
        }
    }
    else if (selection == ViewData)
    {
        wxMessageBox("not implemented");
    }
    else if (selection == Delete)
    {
        par.wsp.removeTable(m_info, false);
    }
    else if (selection == DeleteAndDrop)
    {
        par.wsp.removeTable(m_info, true);
    }
    else if (selection == Unlink)
    {
        m_info.name.clear();
        par.wsp.updateTable(m_info);
        par.model->ItemChanged(wxDataViewItem(this));
        m_status = false;
    }
    else if (selection == Link)
    {
        udataListTablesMsg msg;
        msg.id_table = m_info.id_table;
        msg.id_datasrc = m_info.id_datasrc;
        msg.sender = gce::queueId::WORKSPACE;
        par.wsp.ctx().postDataQueue(msg);
    }

}
void gceTableItem::processMsg(gceWorkspaceItemActionParams par, const gce::MessageInfo &msg)
{
    if (std::holds_alternative<udataTableStatusMsg>(msg))
    {
        processCreateTableReplyMsg(par, std::get<udataTableStatusMsg>(msg));
    }
    else if (std::holds_alternative<udataListTablesReplyMsg>(msg))
    {
        processListTablesReplyMsg(par, std::get<udataListTablesReplyMsg>(msg));
    }
    else if (std::holds_alternative<wspTableUpdatedMsg>(msg))
    {
        processTableUpdated(par, std::get<wspTableUpdatedMsg>(msg));
    }
}

void gceTableItem::processCreateTableReplyMsg(gceWorkspaceItemActionParams par, const udataTableStatusMsg &msg)
{
    if (msg.id_table == m_info.id_table)
    {
        m_status = msg.result;
        par.model->ItemChanged(wxDataViewItem(this));
    }
}

void gceTableItem::processListTablesReplyMsg(gceWorkspaceItemActionParams par, const udataListTablesReplyMsg &msg)
{
    if (msg.id_table == m_info.id_table)
    {
        if (msg.result.tableNames.empty())
        {
            wxMessageBox("No table found in the datasorce");
        }
        else
        {
            wxArrayString table_names;
            table_names.reserve(msg.result.tableNames.size());
            for (const auto &tableName : msg.result.tableNames)
            {
                if (!tableName.first.empty())
                {
                    table_names.Add(tableName.first + '.' + tableName.second);
                }
                else
                {
                    table_names.Add(tableName.second);
                }
            }

            if (int choice = wxGetSingleChoiceIndex("Select table", "Table", table_names, 0, par.parent); choice >= 0)
            {
                auto &tn = msg.result.tableNames[choice];
                m_info.name = tn.second;
                m_info.dbSchema = tn.first;
                par.wsp.updateTable(m_info);

                udataRegisterTableMsg reply;
                reply.id_datasrc = m_info.id_datasrc;
                reply.m_tableInfo = gceDSTable{m_info.id_table, m_schema, m_info.name, m_info.dbSchema};
                reply.sender = gce::queueId::WORKSPACE;
                par.wsp.ctx().postDataQueue(std::move(reply));
            }
        }
    }
}

void gceTableItem::processTableUpdated(gceWorkspaceItemActionParams par, const wspTableUpdatedMsg &msg)
{
    if (m_info.id_table == msg.info.id_table)
    {
        if (m_info.label != msg.info.label)
        {
            par.model->ItemChanged(wxDataViewItem(this));
        }
        m_info = msg.info;
    }
}
