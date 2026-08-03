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

#include "tools/inputtool.h"
#include "RectPM.h"

class gceRectangleTool final : public gceInputToolBase
{
public:
    explicit gceRectangleTool(gceEditorFrame &owner);

    gceToolInfo GetInfo() const override;

    wxString GetString() const override;

    void BeginUse_Custom() override;
protected:
    std::unique_ptr<geom::Geometry> createInputGeometry(geom::CoordinateType cooType) const override;

    gceToolOptions *create_to(wxWindow *parent) override;
    enum POPUP_CMD
    {
        cmdFIRST,
        cmdCANCEL,
        cmdSTEP_BACK,
        cmdBASE_LINE,
        cmdLENGTH,
        cmdLAST,
    };

    void setBaseLine(double azimuth, bool enable) override
    {
        m_PlotModel.m_base = azimuth;
        m_PlotModel.m_fBase = enable;
        updateOtionsPanel();
    }

    bool DoRecalc(const wxPoint &mousePosition) override;

    void Display() override;

    void OnLeftDown(wxMouseEvent &event);

    void TryEnterOrAddPoint();

    void OnCmdCancel(wxCommandEvent &event);

    void OnCmdBaseLine(wxCommandEvent &event);

    void OnCmdStepBack(wxCommandEvent &event);

    void OnUpdateUIEvent(wxUpdateUIEvent &event);

    void OnCmdSetLength(wxCommandEvent &event);

    gceRectPM m_PlotModel;
};

