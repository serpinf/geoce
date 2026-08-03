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

#include "DataTree.h" // class's header file
#include "tree/wsdatasrcgroup.h"
#include "tree/wsmodelgroup.h"
#include "tree/wsscenegroup.h"

/* SceneTreeModel */
class WSTreeModel : public wxDataViewModel
{
public:
    explicit WSTreeModel(gceWorkspace &wsp) : m_wsp(wsp)
    {}

    virtual unsigned int GetColumnCount() const override
    {
        return 1;
    }

    bool SetValue(const wxVariant &, const wxDataViewItem &, unsigned int) override
    {
        return false;
    }

    wxDataViewItem GetParent(const wxDataViewItem &item) const final
    {
        // the invisible root node has no parent
        if (!item.IsOk())
        {
            return wxDataViewItem(nullptr);
        }
        return wxDataViewItem(to_wsitem(item)->getParent());
    }
    bool IsContainer(const wxDataViewItem &item) const final
    {
        if (!item.IsOk())
        {
            return true;
        }
        return to_wsitem(item)->isContainer();
    }
    unsigned int GetChildren(const wxDataViewItem &parent, wxDataViewItemArray &array) const final
    {
        if (!parent.IsOk())
        {
            array.Add(wxDataViewItem(m_root.get()));
            return 1;
        }
        return to_wsitem(parent)->getChildren(array);
    }

    //void SetRootNode(const QueryViewRoot::handle_type& rootNode);
    //void ToggleItem( const wxDataViewItem & item );

    void onItemMenu(wxWindow *parent, const wxDataViewItem &item)
    {
        if (item.IsOk())
        {
            to_wsitem(item)->onMenu({this, parent, m_wsp});
        }
    }
    void onItemActivated(wxWindow *parent, const wxDataViewItem &item)
    {
        if (item.IsOk())
        {
            to_wsitem(item)->onActivated({this, parent, m_wsp});
        }
    }
    void processMsg(const gce::MessageInfo &msg)
    {
        m_root->processMsg({this, nullptr, m_wsp}, msg);
    }
    gceWorkspaceItem *getRoot()
    {
        return m_root.get();
    }

    // Get text attribute, return false of default attributes should be used
    bool GetAttr(const wxDataViewItem &item, unsigned int col, wxDataViewItemAttr &attr) const override
    {
        assert(col == 0);
        if (auto *wsi = to_wsitem(item); wsi)
        {
            return wsi->GetAttr(this->m_wsp, attr);
        }
        return false;
    }

    virtual void onLoadFile() = 0;
protected:
    gceWorkspace &m_wsp;
    std::unique_ptr<gceWorkspaceItem> m_root;
};

class SceneTreeModel final : public WSTreeModel
{
public:
    explicit SceneTreeModel(gceWorkspace &wsp) : WSTreeModel(wsp)
    {}
    virtual void onLoadFile() final
    {
        m_root = std::make_unique<gceSceneGroup>(m_wsp);
        Cleared();
    }
    wxString GetColumnType(unsigned int col) const final
    {
        assert(col == 0);
        return wxDataViewCheckIconTextRenderer::GetDefaultType();
    }

    void GetValue(wxVariant &variant, const wxDataViewItem &item, unsigned int col) const final
    {
        assert(col == 0);

        if (auto *wsi = to_wsitem(item); wsi != nullptr)
        {
            variant << wxDataViewCheckIconText(wsi->name(), wsi->icon(), wsi->isChecked());
        }
    }
    bool SetValue(const wxVariant &variant, const wxDataViewItem &item, unsigned int col) final
    {
        assert(col == 0);

        if (auto *wsi = to_wsitem(item); wsi)
        {
            wxDataViewCheckIconText checkIconText;
            checkIconText << variant;
            return wsi->update(this->m_wsp, checkIconText.GetCheckedState() == wxCHK_CHECKED, to_string(checkIconText.GetText()));
        }
        return false;
    }
};

class ModelTreeModel final : public WSTreeModel
{
public:
    explicit ModelTreeModel(gceWorkspace &wsp) : WSTreeModel(wsp)
    {}
    virtual void onLoadFile() final
    {
        m_root = std::make_unique<gceModelGroup>(m_wsp);
        Cleared();
    }
    wxString GetColumnType(unsigned int col) const final
    {
        assert(col == 0);
        return wxDataViewIconTextRenderer::GetDefaultType();
    }

    void GetValue(wxVariant &variant, const wxDataViewItem &item, unsigned int col) const final
    {
        assert(col == 0);

        if (auto *wsi = to_wsitem(item); wsi)
        {
            variant << wxDataViewIconText(wsi->name(), wsi->icon());
        }
    }
    //std::vector <
};

class DataTreeModel final : public WSTreeModel
{
public:
    explicit DataTreeModel(gceWorkspace &wsp) : WSTreeModel(wsp)
    {}
    virtual void onLoadFile() final
    {
        m_root = std::make_unique<gceDatasourceGroup>(m_wsp);
        Cleared();
    }
    wxString GetColumnType(unsigned int col) const final
    {
        assert(col == 0);
        return wxDataViewIconTextRenderer::GetDefaultType();
    }

    void GetValue(wxVariant &variant, const wxDataViewItem &item, unsigned int col) const final
    {
        assert(col == 0);

        if (auto *wsi = to_wsitem(item); wsi)
        {
            variant << wxDataViewIconText(wsi->name(), wsi->icon());
        }
    }
};



gceDataTreeCtrl::gceDataTreeCtrl(wxWindow *parent, gceWorkspace &wsp, NodeType nodeType)
    : wxDataViewCtrl(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxDV_NO_HEADER)
{

    wxDataViewRenderer *renderer = nullptr;
    if (nodeType == SCENE)
    {
        renderer = new wxDataViewCheckIconTextRenderer;
        m_dataModel = new SceneTreeModel(wsp);
    }
    else if (nodeType == MODELS)
    { //
        renderer = new wxDataViewIconTextRenderer;
        m_dataModel = new ModelTreeModel(wsp);
    }
    else if (nodeType == DATASRC)
    { //
        renderer = new wxDataViewIconTextRenderer;
        m_dataModel = new DataTreeModel(wsp);
    }
    AssociateModel(m_dataModel.get());
    wxDataViewColumn *column = new wxDataViewColumn("", renderer, 0, wxDVC_DEFAULT_WIDTH, wxALIGN_LEFT);
    if (!AppendColumn(column))
    {
        wxDELETE(column);
    }
    m_dataModel->onLoadFile();
    Expand(wxDataViewItem(m_dataModel->getRoot()));

    this->Bind(wxEVT_DATAVIEW_ITEM_CONTEXT_MENU, &gceDataTreeCtrl::OnContextMenu, this);
    this->Bind(wxEVT_DATAVIEW_ITEM_ACTIVATED, &gceDataTreeCtrl::OnItemActivated, this);
}
void gceDataTreeCtrl::OnContextMenu(wxDataViewEvent &event)
{
    if (event.GetItem() != nullptr)
    {
        m_dataModel->onItemMenu(this, event.GetItem());
    }
}
void gceDataTreeCtrl::OnItemActivated(wxDataViewEvent &event)
{
    if (event.GetItem() != nullptr)
    {
        m_dataModel->onItemActivated(this, event.GetItem());
    }
}

void gceDataTreeCtrl::onLoadFile()
{
    m_dataModel->onLoadFile();
    Expand(wxDataViewItem(m_dataModel->getRoot()));
}

void gceDataTreeCtrl::processMsg(const gce::MessageInfo &msg)
{
    m_dataModel->processMsg(msg);
}
