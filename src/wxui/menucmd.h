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
#include <map>
#include <wx/artprov.h>

struct gceMenuCommand
{
    wxString m_text;
    wxString m_help;
    wxBitmap m_icon;
    wxArtID m_artIcon;
    std::string m_svgIcon;
    wxItemKind m_kind = wxITEM_NORMAL;
    bool m_enabled = true;

    explicit gceMenuCommand(const wxString text = wxString()) : m_text(text) {}
    gceMenuCommand &Help(const wxString help)
    {
        m_help = help;
        return *this;
    }
    gceMenuCommand &Icon(const wxBitmap icon)
    {
        m_icon = icon;
        return *this;
    }
    gceMenuCommand &Icon(const wxArtID &id)
    {
        m_artIcon = id;
        return *this;
    }

    gceMenuCommand &IconSVG(const char *svgData)
    {
        m_svgIcon = svgData;
        return *this;
    }

    gceMenuCommand &Check()
    {
        m_kind = wxITEM_CHECK;
        return *this;
    }
    gceMenuCommand &Radio()
    {
        m_kind = wxITEM_RADIO;
        return *this;
    }
    gceMenuCommand &Enable(bool enable)
    {
        m_enabled = enable;
        return *this;
    }

    void MenuItem(wxMenu *menu, int id) const;
    void ToolItem(wxToolBar *toolbar, int id) const;
    wxBitmap getIcon(const wxArtClient &client) const;
};

class gceMenuCmds
{
public:
    enum
    {
        IDM_ContextLow = wxID_HIGHEST + 2000
    };

    gceMenuCmds(const wxString &title = {}) : title(title)
    {}

    void add(int id, const gceMenuCommand &cmddata)
    {
        elements.emplace(id, cmddata);
    }

    gceMenuCommand &add(int id, const wxString text = wxString())
    {
        auto res = elements.emplace(id, text);
        return res.first->second;
    }

    void MenuItem(wxMenu *menu, int id) const;

    template <typename... Args>
    void MenuItems(wxMenu *menu, Args... id) const
    {
        if (menu->GetMenuItemCount() > 0)
        {
            menu->AppendSeparator();
        }
        (MenuItem(menu, id), ...);
    }

    template <typename... Args>
    int GetPopupMenuSelectionFromUser(wxWindow *parent, Args... id) const
    {
        wxMenu menu(title);
        MenuItems(&menu, id...);
        return parent->GetPopupMenuSelectionFromUser(menu);
    }

    void ToolItem(wxToolBar *toolbar, int id) const;

    template <typename... Args>
    void ToolItems(wxToolBar *toolbar, Args... id) const
    {
        if (toolbar->GetToolsCount() > 0)
        {
            toolbar->AddSeparator();
        }
        (ToolItem(toolbar, id), ...);
    }
private:
    const wxString title;
    std::map<int, gceMenuCommand> elements;
};
