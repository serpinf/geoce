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

// IconSingleChoiceDialogDVC.cpp
#include "IconSingleChoiceDialog.h"

IconSingleChoiceDialogDVC::IconSingleChoiceDialogDVC(wxWindow *parent,
                                                     const wxString &message,
                                                     const wxString &caption,
                                                     long style)
    : wxDialog(parent, wxID_ANY, caption, wxDefaultPosition, wxDefaultSize, style),
    m_message(message)
{
    InitControls();
}

IconSingleChoiceDialogDVC::~IconSingleChoiceDialogDVC() = default;

void IconSingleChoiceDialogDVC::InitControls()
{
    wxBoxSizer *top = new wxBoxSizer(wxVERTICAL);

    if (!m_message.IsEmpty())
    {
        wxStaticText *msg = new wxStaticText(this, wxID_ANY, m_message);
        msg->Wrap(450);
        top->Add(msg, 0, wxALL | wxEXPAND, 8);
    }

    m_dvc = new wxDataViewListCtrl(this, wxID_ANY, wxDefaultPosition, wxSize(480, 300), wxDV_ROW_LINES | wxDV_SINGLE | wxDV_NO_HEADER);
    m_dvc->AppendIconTextColumn("");
    top->Add(m_dvc, 1, wxLEFT | wxRIGHT | wxEXPAND, 8);

    auto *btns1 = this->CreateButtonSizer(wxOK | wxCANCEL);
    top->Add(btns1, 0, wxALL | wxALIGN_RIGHT, 8);

    SetSizerAndFit(top);

    m_dvc->Bind(wxEVT_DATAVIEW_ITEM_ACTIVATED, &IconSingleChoiceDialogDVC::OnActivated, this);
    m_dvc->Bind(wxEVT_DATAVIEW_SELECTION_CHANGED, &IconSingleChoiceDialogDVC::OnSelectionChanged, this);

    this->Bind(wxEVT_UPDATE_UI, &IconSingleChoiceDialogDVC::UpdateButtons, this, wxID_OK);
    this->Bind(wxEVT_BUTTON, &IconSingleChoiceDialogDVC::OnOK, this, wxID_OK);
    this->Bind(wxEVT_BUTTON, &IconSingleChoiceDialogDVC::OnCancel, this, wxID_CANCEL);
}

void IconSingleChoiceDialogDVC::SetItems(const std::vector<wxDataViewIconText> &items)
{
    // Create a model row type: single variant of type wxString calling SetValue with wxDataViewIconText
    // Use AppendItem with wxVariant containing wxDataViewIconText via wxVariantData? Simpler: use AppendItem(wxVector<wxVariant>)
    for (const auto &it : items)
    {
        wxVariant v(it);
        m_dvc->AppendItem({v});
    }

    m_selection = -1;
    //UpdateButtons();
}

void IconSingleChoiceDialogDVC::OnOK(wxCommandEvent &WXUNUSED(evt))
{
    m_selection = m_dvc->GetSelectedRow();
    EndModal(wxID_OK);
}

void IconSingleChoiceDialogDVC::OnCancel(wxCommandEvent &WXUNUSED(evt))
{
    m_selection = -1;
    EndModal(wxID_CANCEL);
}

void IconSingleChoiceDialogDVC::OnActivated(wxDataViewEvent &evt)
{
    wxDataViewItem item = evt.GetItem();
    if (item.IsOk())
    {
        m_selection = m_dvc->ItemToRow(item);
        EndModal(wxID_OK);
    }
}

void IconSingleChoiceDialogDVC::OnSelectionChanged(wxDataViewEvent &WXUNUSED(evt))
{
    m_selection = m_dvc->GetSelectedRow();
   // UpdateButtons();
}

void IconSingleChoiceDialogDVC::UpdateButtons(wxUpdateUIEvent &evt)
{
    evt.Enable(m_selection != -1);
}


int gceGetSingleChoiceIndex(const wxString &message, const wxString &caption, const std::vector<wxDataViewIconText> &choices, int initialSelection, wxWindow *parent)
{
    IconSingleChoiceDialogDVC dlg(parent, message, caption);
    dlg.SetItems(choices);
    dlg.SelectRow(initialSelection);
    dlg.ShowModal();
    return dlg.GetSelection();
}
