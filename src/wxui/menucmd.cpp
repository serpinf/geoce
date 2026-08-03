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

void gceMenuCmds::MenuItem(wxMenu *menu, int id) const
{
    if (auto it = elements.find(id); it != elements.end())
    {
        it->second.MenuItem(menu, id);
    }
    else
    {
        // try to add as a wx stock item
        menu->Append(id);
    }
}

void gceMenuCmds::ToolItem(wxToolBar *toolbar, int id) const
{
    if (auto it = elements.find(id); it != elements.end())
    {
        it->second.ToolItem(toolbar, id);
    }
}

void gceMenuCommand::MenuItem(wxMenu *menu, int id) const
{
    auto *item = menu->Append(id, m_text, m_help, m_kind);
    item->SetBitmap(getIcon(wxART_MENU));
    item->Enable(m_enabled);
}

void gceMenuCommand::ToolItem(wxToolBar *toolbar, int id) const
{
    toolbar->AddTool(id, m_text, getIcon(wxART_TOOLBAR), wxNullBitmap, m_kind, m_text, m_help);
}

wxBitmap gceMenuCommand::getIcon(const wxArtClient &client) const
{
    wxBitmap icon;
    if (!m_artIcon.empty())
    {
        icon = wxArtProvider::GetBitmap(m_artIcon, client);
    }
    else if (!m_svgIcon.empty())
    {
        auto size = wxArtProvider::GetDIPSizeHint(client);
        auto size2 = wxArtProvider::GetSizeHint(client);
        icon = wxBitmapBundle::FromSVG(m_svgIcon.c_str(), size).GetBitmap(size2);
    }
    else
    {
        icon = m_icon;
    }
    return icon;
}
