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

class gceEntityPacked;

class gceMoveTool final : public gceToolBase
{
public:
    explicit gceMoveTool(gceEditorFrame &owner);

    wxString GetString() const override;
private:
    gceEntityPacked getMovedCopy(const gceEntityPacked &rec, const glm::dvec3 &move_vec);

    gceToolOptions *create_to(wxWindow *parent) override;

    bool DoRecalc(const wxPoint &mousePosition) override;

    void Display() override;

    void BeginUse_Custom() override;

    bool EndUse_Custom() override;

    bool DoEnter();

    enum POPUP_PT
    {
        cmdFIRST,
        cmdCANCEL,
        cmdLENGTH,
        cmdLAST
    };

    void OnLeftDown(wxMouseEvent &event);

    void OnCmdCancel(wxCommandEvent &event);
    void OnUpdateUIEvent(wxUpdateUIEvent &event);
    void OnCmdSetLength(wxCommandEvent &event);

    gceToolInfo GetInfo() const override;
private:
    mutable geom::CoordinateSeq m_ring{geom::CoordinateType::XYZM};
    geom::Coordinate			m_NextCoord;

    gcePlotModel m_PlotModel;

    bool    m_Coping;
    int     m_CopyNum;
};
