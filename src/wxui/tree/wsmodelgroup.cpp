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
#include "wsmodelgroup.h"
#include "wsmodel.h"
#include "menucmd.h"
#include "models/modelschema.h"
#include <boost/uuid/uuid_io.hpp>
#include "dlg/IconSingleChoiceDialog.h"

gceModelGroup::gceModelGroup(gceWorkspace &wsp) : gceWorkspaceItem(nullptr)
{
    if (wsp.isOk())
    {
        auto models = wsp.getModels();
        for (auto &src : models)
        {
            m_models.emplace_back(std::make_unique<gceModelItem>(this, wsp, src));
        }
    }
}
gceModelGroup::~gceModelGroup()
{

}
unsigned int gceModelGroup::getChildren(wxDataViewItemArray &array) const
{
    array.reserve(m_models.size());
    for (const auto &ds : m_models)
    {
        array.Add(wxDataViewItem(ds.get()));
    }
    return static_cast<unsigned>(m_models.size());
}
void gceModelGroup::onMenu(gceWorkspaceItemActionParams par)
{
    enum
    {
        ADD_MODEL = gceMenuCmds::IDM_ContextLow,
    };
    gceMenuCmds menu;
    menu.add(ADD_MODEL, "Add model...");
    auto selection = menu.GetPopupMenuSelectionFromUser(par.parent, ADD_MODEL);
    if (selection == ADD_MODEL)
    {
        const auto &schemas_all = par.wsp.ctx().cfg.getModelSchemas();
        std::vector<const gceModelSchema *> schemas;
        schemas.reserve(schemas_all.size());
        for (const auto &s : schemas_all)
        {
            if (!s->internal)
            {
                schemas.push_back(s.get());
            }
        }

        std::vector<wxDataViewIconText> model_items;
        model_items.reserve(schemas.size());
        for (const gceModelSchema *schema : schemas)
        {
            wxIcon icon = iconFromSVG(schema->m_svgIcon);
            model_items.emplace_back(to_wxstring(schema->m_name), icon);
        }

        if (int choice = gceGetSingleChoiceIndex("Add model", "Available models", model_items, 0, par.parent); choice > -1)
        {
            auto *modelSchema = schemas[choice];
            gce::model_info mi;
            mi.id_model = makeUUID();
            mi.type = modelSchema->m_name;
            mi.label = modelSchema->m_name + to_string(mi.id_model);
            par.wsp.addModel(mi);
            const auto &modelItem = m_models.emplace_back(std::make_unique<gceModelItem>(this, par.wsp, mi));
            par.model->ItemAdded(wxDataViewItem(this), wxDataViewItem(modelItem.get()));
        };
    }
}

const std::string gceModelGroup::name() const
{
    return "Models";
}

wxIcon gceModelGroup::icon() const
{
    return wxArtProvider::GetIcon(wxART_FLOPPY, wxART_MENU);
}

void gceModelGroup::processMsg(gceWorkspaceItemActionParams par, const gce::MessageInfo &msg)
{
    if (std::holds_alternative<wspModelDeletedMsg>(msg))
    {
        this->processMsg(par, std::get<wspModelDeletedMsg>(msg));
    }
    else
    {
        for (auto &s : m_models)
        {
            s->processMsg(par, msg);
        }
    }
}

void gceModelGroup::processMsg(gceWorkspaceItemActionParams par, const wspModelDeletedMsg &msg)
{
    wxDataViewItem item;
    size_t count = gce::erase_if(m_models, [&msg, &item](auto &x){
        if (x->getId() == msg.id_model)
        {
            item = wxDataViewItem(x.get());
            return true;
        }
        return false;
    });
    assert(count == 1);

    if (item.IsOk())
        par.model->ItemDeleted(wxDataViewItem(this), item);
}
