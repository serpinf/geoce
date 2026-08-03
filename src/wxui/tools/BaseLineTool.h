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
#include "geom/CoordSeq.h"
#include "tools/PlotModel.h"

inline constexpr char base_angle_svg[] = R"rawsvg(<svg xmlns="http://www.w3.org/2000/svg" width="32" height="32" viewBox="0 0 32 32"><path d="M9 24a3.51 3.51 0 0 0-.88-1.86l9.53-16.58l-1.73-1l-9.57 16.56A3.06 3.06 0 0 0 5.5 21a3.5 3.5 0 1 0 3.15 5H28v-2zm-3.5 2A1.5 1.5 0 1 1 7 24.5A1.5 1.5 0 0 1 5.5 26z" fill="currentColor"/><path d="M22 21h2a13 13 0 0 0-5.42-10.56l-1.16 1.62A11 11 0 0 1 22 21z" fill="currentColor"/></svg>)rawsvg";

class gceBaseLineTool final : public gceToolBase
{
public:
    explicit gceBaseLineTool(gceEditorFrame &owner);

    virtual wxString GetString() const override;

    virtual void Display() override;

private:
    virtual void BeginUse_Custom() override;
    virtual bool EndUse_Custom() override;

    virtual bool DoRecalc(const wxPoint &mousePosition) override;

    enum POPUP_CMD
    {
        cmdFIRST,
        cmdCANCEL,
        cmdLAST,
    };
    void OnLeftDown(wxMouseEvent &event);
    void setBaseForParent(const geom::Coordinate &c0, const geom::Coordinate &c1);
    void OnCmdCancel(wxCommandEvent &event);

    geom::CoordinateSeq m_CoordSeq{geom::CoordinateType::XYZM};
    geom::Coordinate m_NextCoord;
    gcePlotModel		m_PlotModel;
};

