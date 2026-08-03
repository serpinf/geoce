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
#include "tools/PlotModel.h"

class CircleTool : public gceInputToolBase
{
public:
    CircleTool(gceEditorFrame &owner, int dimension);

    wxString GetString() const override;

    void BeginUse_Custom() override;

private:

    gceToolOptions *create_to(wxWindow *parent) override;

    enum POPUP_CMD
    {
        cmdFIRST,
        cmdENTER,
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

    bool makeCircle(geom::CoordinateSeq &dest) const;


    void Display() override;

    void OnLeftDown(wxMouseEvent &event);
    void TryEnterOrAddPoint();
    void OnCmdEnter(wxCommandEvent &event);
    void OnCmdCancel(wxCommandEvent &event);
    void OnCmdBaseLine(wxCommandEvent &event);
    void OnCmdStepBack(wxCommandEvent &event);
    void OnCmdSetLength(wxCommandEvent &event);
    void OnUpdateUIEvent(wxUpdateUIEvent &event);

    virtual void saveGeometry() override;
    std::unique_ptr<geom::Geometry> createInputGeometry(geom::CoordinateType cooType) const override;
    bool canInsertGeometry() const override;

    //! vertexes per quadrant
    int m_Precision;
    enum ToolMode
    {
        circle_3points,
        circle_edge_to_center,
        circle_center_to_edge,
        circle_2poins_diameter
    };
    int m_ToolType = circle_3points;

    gcePlotModel 	m_PlotModel;
};

class CircleLineTool final : public CircleTool
{
public:
    explicit CircleLineTool(gceEditorFrame &owner);
    gceToolInfo GetInfo() const override;
};

class CirclePolyTool final : public CircleTool
{
public:
    explicit CirclePolyTool(gceEditorFrame &owner);
    gceToolInfo GetInfo() const override;
};


