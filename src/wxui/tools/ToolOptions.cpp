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

#include "ToolOptions.h"
#include <wx/spinctrl.h>

wxDEFINE_EVENT(gveEVT_TO_UPDATE, wxCommandEvent);

//
// Options Dialog Implementation
//
gceToolOptions::gceToolOptions(wxWindow *parent)
    : wxToolBar(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTB_FLAT | wxTB_NOALIGN | wxTB_NODIVIDER)
{
    Show(false);
    //SetExtraStyle(wxWS_EX_BLOCK_EVENTS | GetExtraStyle());
    SetToolBitmapSize(wxSize(22, 22)); // must be get from environment
}

void gceToolOptions::AddLabel(const wxString &label)
{
    this->AddControl(new wxStaticText(this, wxID_STATIC, label));
}

void gceToolOptions::AddNamedFlag(bool &option, const wxString &name)
{
    auto ctrl = new wxCheckBox(this, wxID_ANY, name);
    this->AddControl(ctrl);

    auto id = ctrl->GetId();
    auto set = [&option](auto &event){ option = event.IsChecked(); };
    auto get = [&option, ctrl](auto &){ ctrl->SetValue(option); };

    Bind(wxEVT_CHECKBOX, set, id);
    Bind(gveEVT_TO_UPDATE, get, id);
}
void gceToolOptions::AddEditableFloatList(double &option, const std::vector<double> &values, const wxSize &size)
{
    wxArrayString itemsStrings;
    for (auto value : values)
    {
        itemsStrings.Add(wxVariant(value));
    }
    auto ctrl = new wxComboBox(this, wxID_ANY, wxString(), wxDefaultPosition, size, itemsStrings, wxCB_DROPDOWN, wxTextValidator(wxFILTER_NUMERIC));
    this->AddControl(ctrl);

    auto id = ctrl->GetId();
    auto set = [&option](auto &event){ option = wxVariant(event.GetString()); };
    auto get = [&option, ctrl](auto &){ ctrl->SetValue(wxVariant(option)); };

    Bind(wxEVT_TEXT, set, id);
    Bind(wxEVT_COMBOBOX, set, id);
    Bind(gveEVT_TO_UPDATE, get, id);
}

void gceToolOptions::AddEnum(int &option, const std::vector<std::pair<wxString, int> > &values, const wxSize &size)
{
    auto ctrl = new wxComboBox(this, wxID_ANY, wxString(), wxDefaultPosition, size, wxArrayString(), wxCB_READONLY | wxCB_DROPDOWN);
    for (auto value : values)
    {
        ctrl->Append(value.first, (void *)(intptr_t)value.second);
    }
    this->AddControl(ctrl);

    auto id = ctrl->GetId();
    auto set = [&option, ctrl](auto &){
        int n = ctrl->GetSelection();
        if (n != wxNOT_FOUND)
        {
            option = (int)(intptr_t)ctrl->GetClientData(n);
        }
    };
    auto get = [&option, ctrl](auto &){
        for (unsigned n = 0; n < ctrl->GetCount(); ++n)
        {
            if ((intptr_t)ctrl->GetClientData(n) == option)
            {
                ctrl->SetSelection(n);
            }
        }
    };

    Bind(wxEVT_COMBOBOX, set, id);
    Bind(wxEVT_TEXT, set, id);
    Bind(gveEVT_TO_UPDATE, get, id);
}

void gceToolOptions::AddSpin(int &option, int xmin, int xmax, const wxSize &size)
{
    auto ctrl = new wxSpinCtrl(this, wxID_ANY, wxString(), wxDefaultPosition, size);
    ctrl->SetRange(xmin, xmax);
    this->AddControl(ctrl);

    auto id = ctrl->GetId();
    auto set = [&option, ctrl](auto &){ option = ctrl->GetValue(); };
    auto get = [&option, ctrl](auto &){ ctrl->SetValue(option); };

    Bind(wxEVT_SPINCTRL, set, id);
    Bind(gveEVT_TO_UPDATE, get, id);
}

void gceToolOptions::Update()
{
    for (auto child : GetChildren())
    {
        wxCommandEvent event(gveEVT_TO_UPDATE, child->GetId());
        event.SetEventObject(child);

        // Do send it
        child->ProcessWindowEvent(event);
    }
}

