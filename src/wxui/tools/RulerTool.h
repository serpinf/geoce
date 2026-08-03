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

#include "tools/tool.h"
#include "tools/PlotModel.h"
#include "geom/CoordSeq.h"

class RulerTool final : public gceToolBase
{
public:
    explicit RulerTool(gceEditorFrame &owner);

    gceToolInfo GetInfo() const override;

    wxString GetString() const override;
private:
    gceToolOptions *create_to(wxWindow *parent) override;
    bool DoRecalc(const wxPoint &mousePosition) override;
    void Display() override;
    void BeginUse_Custom() override;
    bool EndUse_Custom() override;
    bool DoEnter();

    void setBaseLine(double azimuth, bool enable) override
    {
        m_PlotModel.m_base = azimuth;
        m_PlotModel.m_fBase = enable;
        updateOtionsPanel();
    }
    //
    enum POPUP_CMD
    {
        cmdFIRST,
        cmdENTER,
        cmdCANCEL,
        cmdSTEP_BACK,
        cmdBASE_LINE,
        cmdLAST,
    };
    void OnLeftDown(wxMouseEvent &event);

    // popup menu handlers
    void OnCmdEnter(wxCommandEvent &event);
    void OnCmdCancel(wxCommandEvent &event);
    void OnCmdStepBack(wxCommandEvent &event);
    void OnCmdBaseLine(wxCommandEvent &event);
    void OnUpdateUIEvent(wxUpdateUIEvent &event);

    geom::CoordinateSeq m_ring{geom::CoordinateType::XYZM};
    geom::Coordinate m_NextCoord;

    gcePlotModel m_PlotModel;
    bool m_EnterFlag = false;
    bool m_AreaFlag = false;
};

