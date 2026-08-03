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

class gceTableGroup;
class gceTableItem final : public gceWorkspaceItem
{
public:
    explicit gceTableItem(gceTableGroup *parent, gce::table_info &data, const gceTypeSchema *schema);

    unsigned int getChildren(wxDataViewItemArray &array) const final;

    wxIcon icon() const final;

    bool isContainer() const final
    {
        return true;
    }
    void onMenu(gceWorkspaceItemActionParams par) final;
    const std::string name() const override
    {
        return m_info.label;
    }
    void processMsg(gceWorkspaceItemActionParams par, const gce::MessageInfo &msg) override;
    bool isOk() const
    {
        return m_status;
    }
    gce::uuid getId() const
    {
        return m_info.id_table;
    }
private:
    void processCreateTableReplyMsg(gceWorkspaceItemActionParams par, const udataTableStatusMsg &msg);
    void processListTablesReplyMsg(gceWorkspaceItemActionParams par, const udataListTablesReplyMsg &msg);
    void processTableUpdated(gceWorkspaceItemActionParams par, const wspTableUpdatedMsg &msg);

    gce::table_info m_info;
    const gceTypeSchema *m_schema = nullptr;
    bool m_status = false;
    std::unique_ptr<gceWorkspaceItem> m_columnsGroup;
};
