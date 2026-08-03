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
#include "wstablegroup.h"
#include "wstable.hpp"
#include "menucmd.h"
#include <boost/uuid/uuid_io.hpp>
#include "datasrc/datasrcshema.h"

gceTableGroup::gceTableGroup(gceWorkspaceItem *parent, const gce::datasrc_info &info) : gceWorkspaceItem(parent), m_info(info)
{}
gceTableGroup::~gceTableGroup()
{}
unsigned int gceTableGroup::getChildren(wxDataViewItemArray &array) const
{
    for (const auto &it : m_tables)
    {
        array.Add(wxDataViewItem(it.get()));
    }
    return static_cast<unsigned>(m_tables.size());
}
void gceTableGroup::onMenu(gceWorkspaceItemActionParams par)
{
    enum
    {
        Add = gceMenuCmds::IDM_ContextLow,
        AddNoData
    };
    auto *dsSchema = par.wsp.ctx().cfg.findDatasourceSchema(m_info.type);
    if (!dsSchema)
    {
        return;
    }
    gceMenuCmds menu;
    menu.add(Add, "Add table and create source...").Enable(dsSchema->canCreateTables);
    menu.add(AddNoData, "Add without source...");
    auto selection = menu.GetPopupMenuSelectionFromUser(par.parent, Add, AddNoData);
    if (selection == Add || selection == AddNoData)
    {
        wxArrayString schema_names;
        for (const auto &typeSchema : par.wsp.schemas())
        {
            schema_names.Add(to_wxstring(typeSchema->getName()));
        }
        auto choice = wxGetSingleChoice("Select schema for table", "Schemas", schema_names, 0, par.parent);
        if (!choice.empty())
        {
            if (auto *schema = par.wsp.findSchema(to_string(choice)))
            {
                gce::table_info ti;
                ti.id_table = makeUUID();
                ti.id_datasrc = m_info.id_datasrc;
                ti.flags = 0;
                ti.id_schema = schema->getId();
                ti.label = schema->getName() + to_string(ti.id_table);
                if (selection == Add)
                {
                    ti.name = ti.label;
                }
                par.wsp.addTable(ti);

                par.model->ItemChanged(wxDataViewItem(this));
                m_tables.push_back(std::make_unique<gceTableItem>(this, ti, schema));
                par.model->ItemAdded(wxDataViewItem(this), wxDataViewItem(m_tables.back().get()));

                if (!ti.name.empty())
                {
                    udataCreateTableMsg msg;
                    msg.id_datasrc = m_info.id_datasrc;
                    msg.m_table = gceDSTable{ti.id_table, schema, ti.name, ti.dbSchema};
                    msg.sender = gce::queueId::WORKSPACE;
                    par.wsp.ctx().postDataQueue(std::move(msg));
                }
            }
        }
    }
}

wxIcon gceTableGroup::icon() const
{
    return wxArtProvider::GetIcon(wxART_HARDDISK, wxART_MENU);
}
void gceTableGroup::processInit(gceWorkspaceItemActionParams par)
{
    auto tables = par.wsp.getTables(m_info);
    for (auto &tableInfo : tables)
    {
        if (auto *schema = par.wsp.findSchema(tableInfo.id_schema))
        {
            auto &item = m_tables.emplace_back(std::make_unique<gceTableItem>(this, tableInfo, schema));
            par.model->ItemAdded(wxDataViewItem(this), wxDataViewItem(item.get()));

            if (!tableInfo.name.empty())
            {
                udataRegisterTableMsg msg;
                msg.id_datasrc = m_info.id_datasrc;
                msg.m_tableInfo = gceDSTable{tableInfo.id_table, schema, tableInfo.name, tableInfo.dbSchema};
                msg.sender = gce::queueId::WORKSPACE | gce::queueId::MODEL;
                par.wsp.ctx().postDataQueue(std::move(msg));
            }
        }
    }
}

void gceTableGroup::processMsg(gceWorkspaceItemActionParams par, const gce::MessageInfo &msg)
{
    if (std::holds_alternative<wspTableDeletedMsg>(msg))
    {
        processTableDeletedMsg(par, std::get<wspTableDeletedMsg>(msg));
    }
    else
    {
        for (auto &pItem : m_tables)
        {
            pItem->processMsg(par, msg);
        }
    }
}

void gceTableGroup::processTableDeletedMsg(gceWorkspaceItemActionParams par, const wspTableDeletedMsg &msg)
{
    wxDataViewItem item;
    size_t count = gce::erase_if(m_tables, [&msg, &item](auto &x){
        if (x->getId() == msg.id_table)
        {
            item = wxDataViewItem(x.get());
            return true;
        }
        return false;
    });
    assert(count == 1);
    par.model->ItemDeleted(wxDataViewItem(this), item);
}
