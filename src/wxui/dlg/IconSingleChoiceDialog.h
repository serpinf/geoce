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
#include <wx/wx.h>
#include <wx/dataview.h>
#include <vector>

class IconSingleChoiceDialogDVC : public wxDialog
{
public:
    // items: pair<text, optional wxIcon*> (nullptr => no icon)
    IconSingleChoiceDialogDVC(wxWindow *parent,
                              const wxString &message,
                              const wxString &caption,
                              long style = wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);
    ~IconSingleChoiceDialogDVC();

    void SetItems(const std::vector<wxDataViewIconText> &items);

    void SelectRow(int sel)
    {
        if (m_dvc)
            m_dvc->SelectRow(sel);
        m_selection = sel;
    }
    int GetSelection() const { return m_selection; } // -1 if none or cancelled

private:
    void InitControls();
    void OnOK(wxCommandEvent &evt);
    void OnCancel(wxCommandEvent &evt);
    void OnActivated(wxDataViewEvent &evt);
    void OnSelectionChanged(wxDataViewEvent &evt);
    void UpdateButtons(wxUpdateUIEvent &evt);

    wxString m_message;
    wxDataViewListCtrl *m_dvc{nullptr};
    int m_selection{-1};
};

int gceGetSingleChoiceIndex(const wxString &message,
                            const wxString &caption,
                            const std::vector<wxDataViewIconText> &choices,
                            int initialSelection,
                            wxWindow *parent = nullptr);
