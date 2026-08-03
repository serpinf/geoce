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

#include "CConnect.h"
#include <wx/valgen.h>
#include "gvesizer.h"

CConnect::CConnect(const wxString &svctype, wxString &svcname, wxString &svcparams, bool &autoConnect, wxWindow *parent) :
    wxDialog(parent, wxID_ANY, wxString::Format("Setup %s datasource", svctype))
{
    wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(topSizer);

    auto *gridSizer = new UIPropertySizer(wxSizerFlags().Align(wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL).Border(wxALL),
                                          wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL).Border(wxALL).Expand());
    topSizer->Add(gridSizer, 0, wxGROW | wxALL, 0);

    auto *tc_name = new wxTextCtrl(this, wxID_ANY);
    tc_name->SetValidator(wxTextValidator(wxFILTER_EMPTY, &svcname));
    gridSizer->Add("Name:", tc_name);

    auto *tc_params = new wxTextCtrl(this, wxID_ANY);
    tc_params->SetValidator(wxTextValidator(wxFILTER_EMPTY, &svcparams));
    tc_params->SetMinSize(wxSize(800, -1));
    gridSizer->Add("Params:", tc_params);

    auto *ch_autoConnect = new wxCheckBox(this, wxID_ANY, "Connect on load");
    ch_autoConnect->SetValidator(wxGenericValidator(&autoConnect));
    gridSizer->Add(wxString(), ch_autoConnect);

    if (auto butsizer = CreateSeparatedButtonSizer(wxOK | wxCANCEL); butsizer)
    {
        topSizer->Add(butsizer, 0, wxEXPAND | wxALL, 2);
    }
    topSizer->SetSizeHints(this);
    Centre();
}

