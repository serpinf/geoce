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

class UIPropertySizer : public wxFlexGridSizer
{
public:
    UIPropertySizer(const wxSizerFlags &labelFlags, const wxSizerFlags &dataFlags) : wxFlexGridSizer(2, 0, 0), m_labelFlags(labelFlags), m_dataFlags(dataFlags)
    {
        AddGrowableCol(1);
    }
    void Add(const wxString &label, wxWindow *win)
    {
        auto parent = win->GetParent();
        wxFlexGridSizer::Add(new wxStaticText(parent, wxID_ANY, label), m_labelFlags);
        wxFlexGridSizer::Add(win, m_dataFlags);
    }
private:
    const wxSizerFlags m_labelFlags;
    const wxSizerFlags m_dataFlags;
};

template <class Sizer>
class GVESizerWithAddLabel : public Sizer
{
public:
    template <typename ...Args>
    GVESizerWithAddLabel(Args ...args) : Sizer(args...) {}

    auto AddLabel(wxWindow *parent, const wxString &label, const wxSizerFlags &flags)
    {
        return this->AddLabel(parent, label, flags.GetProportion(), flags.GetFlags(), flags.GetBorderInPixels());
    }

    auto AddLabel(wxWindow *parent, const wxString &label, int proportion = 0, int flag = 0, int border = 0)
    {
        auto stext = new wxStaticText(parent, wxID_STATIC, label);
        return this->Add(stext, proportion, flag, border);
    }
};

using wxBoxSizerGVE = GVESizerWithAddLabel<wxBoxSizer>;
using wxFlexGridSizerGVE = GVESizerWithAddLabel<wxFlexGridSizer>;
