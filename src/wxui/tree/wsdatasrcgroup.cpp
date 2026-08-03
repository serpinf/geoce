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
#include "wsdatasrcgroup.h"
#include "wsdatasrc.h"
#include "menucmd.h"
#include "dlg/IconSingleChoiceDialog.h"
#include "datasrc/datasrcshema.h"

gceDatasourceGroup::gceDatasourceGroup(gceWorkspace &wsp) : gceWorkspaceItem(nullptr)
{
    if (wsp.isOk())
    {
        auto srcs = wsp.getDataSources();
        for (auto &src : srcs)
        {
            m_dataSrcs.emplace_back(std::make_unique<gceDataSourceItem>(this, src, wsp.ctx()));
        }

        for (auto &src : m_dataSrcs)
        {
            if (src->cfg().autoConnect)
            {
                src->doConnect(wsp);
            }
        }
    }
}
gceDatasourceGroup::~gceDatasourceGroup()
{

}
unsigned int gceDatasourceGroup::getChildren(wxDataViewItemArray &array) const
{
    array.reserve(m_dataSrcs.size());
    for (const auto &ds : m_dataSrcs)
    {
        array.Add(wxDataViewItem(ds.get()));
    }
    return static_cast<unsigned>(m_dataSrcs.size());
}
void gceDatasourceGroup::onMenu(gceWorkspaceItemActionParams par)
{
    enum
    {
        ADD_DS = gceMenuCmds::IDM_ContextLow,
    };
    gceMenuCmds menu;
    menu.add(ADD_DS, "Add datasource...");
    auto selection = menu.GetPopupMenuSelectionFromUser(par.parent, ADD_DS);
    if (selection == ADD_DS)
    {
        std::vector<wxDataViewIconText> datasource_items;
        const auto &schemas = par.wsp.ctx().cfg.getDatasourceSchemas();
        datasource_items.reserve(schemas.size());
        for (auto &schema : schemas)
        {
            wxIcon icon = schema ? iconFromSVG(schema->m_svgIcon) : wxIcon();
            datasource_items.emplace_back(to_wxstring(schema->m_name), icon);
        }

        if (int choice = gceGetSingleChoiceIndex("Add datasource", "Available datasources", datasource_items, 0, par.parent); choice > -1)
        {
            gce::datasrc_info svc;
            svc.id_datasrc = makeUUID();
            svc.type = schemas[choice]->m_name;
            svc.name = "New " + schemas[choice]->m_name;
            svc.params = schemas[choice]->m_defaultParams;
            svc.autoConnect = 0;
            par.wsp.addDataSource(svc);
            auto &ds = m_dataSrcs.emplace_back(std::make_unique<gceDataSourceItem>(this, svc, par.wsp.ctx()));
            par.model->ItemAdded(wxDataViewItem(this), wxDataViewItem(ds.get()));
        };

    }
}

const std::string gceDatasourceGroup::name() const
{
    return "Data sources";
}

wxIcon gceDatasourceGroup::icon() const
{
    return wxArtProvider::GetIcon(wxART_FLOPPY, wxART_MENU);
}

template <class T, class UnaryPredicate>
std::unique_ptr<T> extract1(std::vector<std::unique_ptr<T>> &c, UnaryPredicate pred)
{

    std::unique_ptr<T> res;
    auto first = std::find_if(c.begin(), c.end(), pred);
    if (first != c.end())
    {
        res = std::move(*first);
        c.erase(first);
    }
    return res;
}
void gceDatasourceGroup::processMsg(gceWorkspaceItemActionParams par, const gce::MessageInfo &msg)
{
    if (std::holds_alternative<udataSvcRemoveMsg>(msg))
    {
        auto &svc_remove = std::get<udataSvcRemoveMsg>(msg);
        auto ptr = extract1(m_dataSrcs, [&svc_remove](auto &x){ return x->cfg().id_datasrc == svc_remove.id_datasrc; });
        if (ptr)
        {
            par.model->ItemDeleted(wxDataViewItem(this), wxDataViewItem(ptr.get()));
        }
    }
    else
    {
        for (auto &s : m_dataSrcs)
        {
            s->processMsg(par, msg);
        }
    }
}
