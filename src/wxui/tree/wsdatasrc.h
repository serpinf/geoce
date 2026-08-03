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
class gceTableGroup;

template <typename... Args>
inline auto to_wxstring(Args&&... args)
{
    return std::make_tuple(wxString::FromUTF8(args)...);
}
template <typename... Args>
inline auto to_string(const Args&... args)
{
    return std::make_tuple(to_string(args)...);
}

class gceDataSourceItem final : public gceWorkspaceItem
{
public:
    explicit gceDataSourceItem(gceWorkspaceItem *parent, const gce::datasrc_info &svc, gceContext &ctx);
    ~gceDataSourceItem() override;
    unsigned int getChildren(wxDataViewItemArray &array) const final;

    wxIcon icon() const final;

    bool isContainer() const final
    {
        return isConnectionOk();
    }
    void onMenu(gceWorkspaceItemActionParams par) final;
    void onActivated(gceWorkspaceItemActionParams par) final
    {
        doConnect(par.wsp);
    }
    const std::string name() const override
    {
        return m_cfg.name;
    }
    gce::datasrc_info &cfg()
    {
        return m_cfg;
    }

    void processMsg(gceWorkspaceItemActionParams par, const gce::MessageInfo &msg) final;

    void doConnect(gceWorkspace &wsp);
private:
    void processConnectReplyMsg(gceWorkspaceItemActionParams par, const udataConnectReplyMsg &msg);

    bool isConnectionOk() const
    {
        return m_status == gceEntityStatus::OK;
    }

    wxIcon m_icon0;
    wxIcon m_icon1;
    gce::datasrc_info m_cfg;
    gceEntityStatus m_status = gceEntityStatus::NONE;
    std::unique_ptr<gceTableGroup> m_tables;
};
