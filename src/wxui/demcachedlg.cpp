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

#include "demcachedlg.h"
#include "gvesizer.h"
#include <wx/filepicker.h>
#include <wx/spinctrl.h>
#include "tileid.h"
#include "engine.hpp"
#include "alg/wgsop.h"

gceDEMCacheDialog::gceDEMCacheDialog(wxWindow *parent, wxString &pathCache, wxString &pathSrc, gcePyramidAOI &aoi) :
    wxDialog(parent, wxID_ANY, "DEM cache update"),
    m_pathCache(pathCache), m_pathSrc(pathSrc), m_aoi(aoi)
{
    const double latMax = glm::degrees(wgsop::flat2wgs_Y(M_PI));
    wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(topSizer);

    auto *gridSizer = new UIPropertySizer(wxSizerFlags().Align(wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL).Border(wxALL),
                                          wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL).Border(wxALL).Expand());
    topSizer->Add(gridSizer, 0, wxGROW | wxALL, 0);
    gridSizer->SetMinSize(wxSize(800, -1));

    m_tcCachePath = new wxDirPickerCtrl(this, wxID_ANY, pathCache);
    gridSizer->Add("Cache path:", m_tcCachePath);

    m_tcDEMPath = new wxDirPickerCtrl(this, wxID_ANY, pathSrc);
    gridSizer->Add("DEM path:", m_tcDEMPath);

    m_spLevMin = new wxSpinCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, gce::tileid::max_level(), aoi.levMin);
    gridSizer->Add("Min zoom:", m_spLevMin);
    m_spLevMax = new wxSpinCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, gce::tileid::max_level(), aoi.levMax);
    gridSizer->Add("Max zoom:", m_spLevMax);
    m_spXMin = new wxSpinCtrlDouble(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, -180, 180, aoi.bbox.cmin.x, 0.0001);
    gridSizer->Add("Min longitude:", m_spXMin);
    m_spYMin = new wxSpinCtrlDouble(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, -latMax, latMax, aoi.bbox.cmin.y, 0.0001);
    gridSizer->Add("Min latitude:", m_spYMin);
    m_spXMax = new wxSpinCtrlDouble(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, -180, 180, aoi.bbox.cmax.x, 0.0001);
    gridSizer->Add("Max longitude:", m_spXMax);
    m_spYMax = new wxSpinCtrlDouble(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, -latMax, latMax, aoi.bbox.cmax.y, 0.0001);
    gridSizer->Add("Max latitude:", m_spYMax);

    m_stCacheParams = new wxStaticText(this, wxID_ANY, "");
    gridSizer->Add("Cache parameters:", m_stCacheParams);

    auto *butDefaults = new wxButton(this, wxID_ANY, "Default area");
    butDefaults->Bind(wxEVT_BUTTON, [this, latMax](wxCommandEvent &){
        m_spLevMin->SetValue(1);
        m_spLevMax->SetValue(10);
        m_spXMin->SetValue(-180);
        m_spYMin->SetValue(-latMax);
        m_spXMax->SetValue(180);
        m_spYMax->SetValue(latMax);
    });
    gridSizer->Add(wxString(), butDefaults);

    if (auto *butsizer = CreateSeparatedButtonSizer(wxOK | wxCANCEL); butsizer)
    {
        topSizer->Add(butsizer, 0, wxEXPAND | wxALL, 2);
    }
    topSizer->SetSizeHints(this);
    Centre();
    m_stCacheParams->Bind(wxEVT_UPDATE_UI, &gceDEMCacheDialog::OnUpdateParams, this);
}

bool gceDEMCacheDialog::TransferDataFromWindow()
{
    if (wxString pathCache = m_tcCachePath->GetPath(); !pathCache.IsEmpty())
    {
        m_pathCache = pathCache;
    }
    else
    {
        return false;
    }

    if (wxString pathSrc = m_tcDEMPath->GetPath(); !pathSrc.IsEmpty())
    {
        m_pathSrc = pathSrc;
    }
    else
    {
        return false;
    }

    gcePyramidAOI aoi;
    GetAOI(aoi);
    if (aoi.bbox.empty() || aoi.levMin > aoi.levMax)
    {
        return false;
    }

    m_aoi = aoi;
    return true;
}

void gceDEMCacheDialog::GetAOI(gcePyramidAOI &aoi)
{
    aoi.levMin = m_spLevMin->GetValue();
    aoi.levMax = m_spLevMax->GetValue();
    aoi.bbox.cmin.x = m_spXMin->GetValue();
    aoi.bbox.cmin.y = m_spYMin->GetValue();
    aoi.bbox.cmax.x = m_spXMax->GetValue();
    aoi.bbox.cmax.y = m_spYMax->GetValue();
}

inline glm::ivec2 tileIndex(const geom::Box2D &bbox, const glm::dvec2 &pos, int lev)
{
    const int count = 1u << lev;
    const glm::dvec2 t = (pos - bbox.cmin) / bbox.size();
    return glm::clamp(glm::ivec2(t * double(count)), 0, count - 1);
}

static void countTiles(const gcePyramidAOI &aoi, uint64_t &tiles, uint64_t &size)
{
    tiles = 0;
    const geom::Box2D bbox{glm::dvec2{-M_PI}, glm::dvec2{M_PI}};

    for (int lev = aoi.levMin; lev <= aoi.levMax; ++lev)
    {
        glm::ivec2 imin = tileIndex(bbox, wgsop::wgs_degrees2flat(aoi.bbox.cmin), lev);
        glm::ivec2 imax = tileIndex(bbox, wgsop::wgs_degrees2flat(aoi.bbox.cmax), lev);
        tiles += (imax.x - imin.x + 1) * (imax.y - imin.y + 1);
    }
    size = tiles * gce::sizep * gce::sizep * sizeof(float) / (1024 * 1024);
}

void gceDEMCacheDialog::OnUpdateParams(wxUpdateUIEvent &event)
{
    gcePyramidAOI aoi;
    GetAOI(aoi);
    uint64_t tiles = 0, size = 0;
    countTiles(aoi, tiles, size);
    auto text = fmt::format("zoom levels {}-{}, sizelon {:.4f}, sizelat {:.4f}, tiles {}, size {}MB",
        m_spLevMin->GetValue(), m_spLevMax->GetValue(),
        std::max(m_spXMax->GetValue() - m_spXMin->GetValue(), 0.0),
        std::max(m_spYMax->GetValue() - m_spYMin->GetValue(), 0.0),
        tiles, size);
    event.SetText(to_wxstring(text));
}

