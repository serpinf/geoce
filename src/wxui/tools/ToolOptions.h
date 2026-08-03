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

class gceToolOptions final : public wxToolBar
{
public:
    explicit gceToolOptions(wxWindow *parent);

    void AddLabel(const wxString &label);
    void AddNamedFlag(bool &option, const wxString &name);
    void AddEditableFloatList(double &option, const std::vector<double> &values, const wxSize &size = wxDefaultSize);
    void AddEnum(int &option, const std::vector<std::pair<wxString, int> > &values, const wxSize &size = wxDefaultSize);
    void AddSpin(int &option, int xmin, int xmax, const wxSize &size = wxDefaultSize);

    void Update();
};


