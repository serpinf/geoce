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
class wxDirPickerCtrl;
class wxSpinCtrl;
class wxSpinCtrlDouble;
struct gcePyramidAOI;

class gceDEMCacheDialog : public wxDialog
{
public:
    gceDEMCacheDialog(wxWindow *parent, wxString &pathCache, wxString &pathSrc, gcePyramidAOI &aoi);

    bool TransferDataFromWindow() override;

    void GetAOI(gcePyramidAOI &aoi);

private:
    void OnUpdateParams(wxUpdateUIEvent &event);
    wxString &m_pathCache;
    wxString &m_pathSrc;
    gcePyramidAOI &m_aoi;

    wxDirPickerCtrl *m_tcCachePath;
    wxDirPickerCtrl *m_tcDEMPath;
    wxSpinCtrl *m_spLevMin;
    wxSpinCtrl *m_spLevMax;
    wxSpinCtrlDouble *m_spXMin;
    wxSpinCtrlDouble *m_spYMin;
    wxSpinCtrlDouble *m_spXMax;
    wxSpinCtrlDouble *m_spYMax;
    wxStaticText *m_stCacheParams;
};
