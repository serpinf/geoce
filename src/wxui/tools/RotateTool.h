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
#include "RotatePM.h"
#include "geom/CoordSeq.h"

class RotateTool : public gceToolBase
{
public:
    explicit RotateTool(gceEditorFrame &owner);


    virtual wxString GetString() const override;

    virtual void BeginUse_Custom() override;

    virtual bool EndUse_Custom() override;

    virtual void Display() override;
protected:

    virtual gceToolOptions *create_to(wxWindow *parent) override;

    bool DoEnter();

    virtual bool DoRecalc(const wxPoint &mousePosition) override;
    enum POPUP_PT
    {
        cmdFIRST,
        cmdCANCEL,
        cmdBACK,
        cmdLAST,
    };
    void OnLeftDown(wxMouseEvent &event);
    void OnKeyDown(wxKeyEvent &event);
    void OnKeyUp(wxKeyEvent &event);

    void OnCmdCancel(wxCommandEvent &event);

    void OnUpdateUIEvent(wxUpdateUIEvent &event);

private:
    gceToolInfo GetInfo() const override;
    mutable  geom::CoordinateSeq  m_ring{geom::CoordinateType::XYZM};
    geom::Coordinate m_NextCoord;

    gceRotatePM	m_PlotModel;
    bool m_Coping = false;
    int m_CopyNum = 1;
    bool m_basePointInput = false;
};

