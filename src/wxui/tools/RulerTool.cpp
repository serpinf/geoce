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

#include "RulerTool.h"
#include "BaseLineTool.h"
#include "editorfrm.h"
#include "tools/ToolOptions.h"

/* XPM */
static const char *ruler_cur_xpm[] = {
    "32 32 3 1",
    " 	c None",
    ".	c #000000",
    "+	c #FFFFFF",
    "       ...                      ",
    "       .+.                      ",
    "       .+.                      ",
    "       .+.                      ",
    "       .+.                      ",
    "       .+.                      ",
    "                                ",
    "......  .  ......               ",
    ".+++++ .+. +++++.               ",
    "......  .  ......               ",
    "                                ",
    "       .+.                      ",
    "       .+.                      ",
    "       .+.                      ",
    "       .+.                      ",
    "       .+.                      ",
    "       ...                      ",
    "                                ",
    "                                ",
    "                                ",
    "   ......................       ",
    "   .++++++++++++++++++++.       ",
    "   .+++++.+++++.+++++.++.       ",
    "   .+++++.+++++.+++++.++.       ",
    "   .++.++.++.++.++.++.++.       ",
    "   ......................       ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                "};

static constexpr char ruler_svg[] = R"rawsvg(<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24"><g fill="none"><path d="m15.857 1.63l6.397 6.397L7.861 22.42l-6.397-6.397z"/><path stroke="currentColor" stroke-linecap="square" stroke-width="2" d="m5.062 12.425l1.602 1.602m5.595-8.798L13.86 6.83m-5.2 1.995l2.401 2.402m4.795-9.598l6.397 6.397L7.861 22.42l-6.397-6.397z"/></g></svg>)rawsvg";

RulerTool::RulerTool(gceEditorFrame &owner) : gceToolBase(owner),
m_PlotModel(getCanvas())
{
    this->Bind(wxEVT_LEFT_DOWN, &RulerTool::OnLeftDown, this);

    // popup handling
    this->Bind(wxEVT_MENU, &RulerTool::OnCmdEnter, this, cmdENTER);
    this->Bind(wxEVT_MENU, &RulerTool::OnCmdCancel, this, cmdCANCEL);
    this->Bind(wxEVT_MENU, &RulerTool::OnCmdStepBack, this, cmdSTEP_BACK);
    this->Bind(wxEVT_MENU, &RulerTool::OnCmdBaseLine, this, cmdBASE_LINE);
    this->Bind(wxEVT_UPDATE_UI, &RulerTool::OnUpdateUIEvent, this, cmdFIRST, cmdLAST);


    uic(cmdENTER, "Enter\tEnter").IconSVG(enter_svg);
    uic(cmdSTEP_BACK, "Back\tZ").IconSVG(step_back_svg);
    uic(cmdCANCEL, "Cancel").IconSVG(cancel_svg);
    uic.separtor();
    uic(cmdBASE_LINE, "Base line\tCtrl+G").IconSVG(base_angle_svg);

    // create cursor
    SetCursor(0, ruler_cur_xpm, 8, 7);
}

gceToolInfo RulerTool::GetInfo() const
{
    return gceToolInfo{"Map ruler", ruler_svg, "mesure length and area"};
}

void RulerTool::BeginUse_Custom()
{}

bool RulerTool::EndUse_Custom()
{
    if (EndUseChild() && DoEnter())
    {
        m_EnterFlag = false;
        m_ring.clear();
        m_PlotModel.postModelMainLine(m_ring);
        return true;
    }
    return false;
}

void RulerTool::OnLeftDown(wxMouseEvent &event)
{
    if (m_EnterFlag)
    {
        m_ring.clear();
        m_EnterFlag = false;
    }
    m_PlotModel.AddPoint(m_ring, m_NextCoord);
    m_PlotModel.postModelMainLine(m_ring);
    wxLogMessage("AddPoint %f, %f", m_NextCoord.pos.x, m_NextCoord.pos.y);

    event.Skip();
}

void RulerTool::Display()
{
    m_PlotModel.postRenderTempLine1(m_ring, m_NextCoord, this->HaveMouse() && !this->m_EnterFlag);
    if (m_AreaFlag)
    {
        m_PlotModel.postRenderTempLine2(m_ring, m_NextCoord, this->HaveMouse() && !this->m_EnterFlag);
    }
}

void RulerTool::OnCmdEnter(wxCommandEvent &)
{
    if (m_ring.empty()) return;

    if (DoEnter())
    {
        // TODO: check if onUpdate UI works for this
        // force status update here as we may press enter without moving mouse
        m_owner.setToolString(GetString());
    }
}

void RulerTool::OnCmdCancel(wxCommandEvent &)
{
    m_EnterFlag = false;
    m_ring.clear();
    m_PlotModel.postModelMainLine(m_ring);
}

void RulerTool::OnCmdBaseLine(wxCommandEvent &)
{
    startChild<gceBaseLineTool>();
}

void RulerTool::OnUpdateUIEvent(wxUpdateUIEvent &event)
{
    switch (event.GetId())
    {
    case cmdENTER:
    case cmdCANCEL:
    case cmdSTEP_BACK:
        event.Enable(!m_ring.empty());
        break;
    case cmdBASE_LINE:
        event.Enable(true);
        break;
    }
}

void RulerTool::OnCmdStepBack(wxCommandEvent &)
{
    if (!m_ring.empty())
    {
        m_ring.pop_back();
        m_PlotModel.postModelMainLine(m_ring);
    }
}

bool RulerTool::DoRecalc(const wxPoint &mousePosition)
{
    std::vector<geom::Coordinate> hint_seq;
    m_NextCoord = m_PlotModel.DoRecalc(m_ring, mousePosition, hint_seq);
    return true;
}


bool RulerTool::DoEnter()
{
    if (m_AreaFlag && m_ring.size() > 2)
    {
        m_ring.EnsureRing();
        m_PlotModel.postModelMainLine(m_ring);
    }
    m_EnterFlag = true;
    return true;
}

wxString RulerTool::GetString() const
{
    if (!m_ring.empty())
    {
        geom::CoordinateSeq seq(geom::CoordinateType::XY);
        if (m_EnterFlag)
        {
            seq.append(m_ring);
        }
        else
        {
            seq.reserve(m_ring.size() + 1);
            seq.append(m_ring);
            seq.push_back(m_NextCoord);
        }
        auto pgeom = geom::Geometry::Create(m_AreaFlag ? 2 : 1, geom::CoordinateType::XY, seq);
        if (pgeom)
        {
            return gceToolBase::describeGeometry(*pgeom);
        }
    }
    return {};
}

gceToolOptions *RulerTool::create_to(wxWindow *parent)
{
    // set options
    auto to = new gceToolOptions(parent);

    to->AddNamedFlag(this->m_AreaFlag, "Area");

    to->AddSeparator();

    to->AddNamedFlag(m_PlotModel.m_fAngleStep, "Angle step:");
    to->AddEditableFloatList(m_PlotModel.m_angle, {15.0, 30.0, 45.0, 90.0});

    to->AddSeparator();

    to->AddNamedFlag(m_PlotModel.m_fBase, "Base:");
    to->AddEditableFloatList(m_PlotModel.m_base, {0.0}, wxSize(50, -1));

    to->AddSeparator();

    to->AddNamedFlag(m_PlotModel.m_fLength, "Length step:");
    to->AddEditableFloatList(m_PlotModel.m_lstep, {0.001, 0.01, 0.1, 1.0});

    to->Realize();
    return to;
}

