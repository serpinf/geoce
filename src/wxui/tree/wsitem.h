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
#include <wx/dataview.h>
#include <wx/artprov.h>
#include "gisdom/engine.hpp"

class gceWorkspace;
struct gceWorkspaceItemActionParams
{
    wxDataViewModel *model;
    wxWindow *parent;
    gceWorkspace &wsp;
};
class gceWorkspaceItem
{
public:
    explicit gceWorkspaceItem(gceWorkspaceItem *parent) : m_parent(parent) {}
    virtual ~gceWorkspaceItem() = default;

    gceWorkspaceItem(const gceWorkspaceItem &) = delete;
    void operator=(const gceWorkspaceItem &) = delete;

    virtual const std::string name() const
    {
        return "workspace item";
    }
    virtual wxIcon icon() const
    {
        return wxArtProvider::GetIcon(wxART_QUESTION);
    }
    virtual wxCheckBoxState isChecked() const
    {
        return wxCHK_UNDETERMINED;
    }
    virtual bool update(gceWorkspace &wsp, bool checked, const std::string &name);
    virtual bool GetAttr(gceWorkspace &wsp, wxDataViewItemAttr &attr) const;

    virtual unsigned int getChildren(wxDataViewItemArray &) const
    {
        return 0;
    }
    gceWorkspaceItem *getParent() const
    {
        return m_parent;
    }
    virtual void onMenu(gceWorkspaceItemActionParams /*par*/) {}
    virtual void onActivated(gceWorkspaceItemActionParams /*par*/) {}
    virtual bool isContainer() const
    {
        return false;
    };
    //virtual void onLoadWorkspace(gceWorkspaceItemActionParams& ) {}
    virtual void processMsg(gceWorkspaceItemActionParams, const gce::MessageInfo &) {}
    static wxIcon iconFromSVG(const std::string &data)
    {
        return iconFromSVG(data.c_str());
    }
    static wxIcon iconFromSVG(const char *data);
    static gce::uuid makeUUID();
private:
    gceWorkspaceItem *m_parent = nullptr;
};

inline gceWorkspaceItem *to_wsitem(wxDataViewItem item)
{
    return static_cast<gceWorkspaceItem *>(item.GetID());
}
