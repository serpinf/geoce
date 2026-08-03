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

#include "DotTool.h"
#include "editorfrm.h"
#include "ToolOptions.h"
#include "VertexEditor.h"
#include "BaseLineTool.h"


/* XPM */
static const char *Input_dot_xpm[] = {
    "32 32 3 1",
    " 	c None",
    ".	c #FFFFFF",
    "+	c #000000",
    "                                ",
    "         .+.                    ",
    "         +.+                    ",
    "         +.+                    ",
    "        +...+                   ",
    "        +...+                   ",
    "        +...+                   ",
    "       +.....+                  ",
    "       +.....+                  ",
    "       +.....+                  ",
    "      +.......+                 ",
    "      +.......+                 ",
    "      +.......+                 ",
    "     +.........+                ",
    "     +.........+                ",
    "     +.........+   .         .  ",
    "     +++++++++++  .+.       .+. ",
    "                   .+.     .+.  ",
    "                    .+.   .+.   ",
    "                     . ... .    ",
    "                      .+++.     ",
    "                      .+.+.     ",
    "                      .+++.     ",
    "                     . ... .    ",
    "                    .+.   .+.   ",
    "                   .+.     .+.  ",
    "                  .+.       .+. ",
    "                   .         .  ",
    "                                ",
    "                                ",
    "                                ",
    "                                "};

static constexpr char vertex_svg[] = R"rawsvg(<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24"><path fill="currentColor" d="M10 10h4v4h-4zm6 0h4v4h-4zM4 10h4v4H4z"/></svg>)rawsvg";

DotTool::DotTool(gceEditorFrame &owner) : gceInputToolBase(owner, 0),
m_PlotModel(getCanvas())
{
    this->Bind(wxEVT_LEFT_DOWN, &DotTool::OnLeftDown, this);

    // popup handling
    this->Bind(wxEVT_MENU, &DotTool::OnCmdEnter, this, cmdENTER);
    this->Bind(wxEVT_MENU, &DotTool::OnCmdCancel, this, cmdCANCEL);
    this->Bind(wxEVT_MENU, &DotTool::OnCmdEditP, this, cmdEDITP);
    this->Bind(wxEVT_MENU, &DotTool::OnCmdStepBack, this, cmdSTEP_BACK);
    this->Bind(wxEVT_MENU, &DotTool::OnCmdSetLength, this, cmdLENGTH);
    this->Bind(wxEVT_MENU, &DotTool::OnCmdBaseLine, this, cmdBASE_LINE);
    this->Bind(wxEVT_UPDATE_UI, &DotTool::OnUpdateUIEvent, this, cmdFIRST, cmdLAST);

    // init Plot Model
    uic(cmdENTER, "Enter\tEnter").IconSVG(enter_svg);
    uic(cmdSTEP_BACK, "Step back\tZ").IconSVG(step_back_svg);
    uic(cmdEDITP, "Edit...\t`").IconSVG(vertexedit_svg);
    uic(cmdLENGTH, "Length\tL");
    uic(cmdCANCEL, "Cancel").Icon(cancel_svg);
    uic.separtor();
    uic(cmdBASE_LINE, "Base line\tCtrl+G").Icon(base_angle_svg);

    SetCursor(0, Input_dot_xpm, 11, 0);
}

gceToolInfo DotTool::GetInfo() const
{
    return gceToolInfo{"Points", vertex_svg, "use 'Enter' to finish"};
}

void DotTool::BeginUse_Custom()
{
    ConnectLayer();
}

void DotTool::OnLeftDown(wxMouseEvent &event)
{
    m_PlotModel.AddPoint(m_ring, m_NextCoord);
    saveGeometry();

    event.Skip();
}

void DotTool::Display()
{
    m_PlotModel.postRenderTempLine1(m_ring, m_NextCoord, this->HaveMouse());
}

void DotTool::OnCmdEnter(wxCommandEvent &)
{
    DoEnter();
}

void DotTool::OnCmdCancel(wxCommandEvent &)
{
    m_ring.clear();
    saveGeometry();
}

void DotTool::OnCmdEditP(wxCommandEvent &)
{
    if (!m_ring.empty())
    {
        startChild<gceVertexEditor>();
    }
}

void DotTool::OnCmdBaseLine(wxCommandEvent &)
{
    startChild<gceBaseLineTool>();
}

void DotTool::OnUpdateUIEvent(wxUpdateUIEvent &event)
{
    switch (event.GetId())
    {
    case cmdENTER:
        event.Enable(CanInsert());
        break;
    case cmdCANCEL:
    case cmdSTEP_BACK:
    case cmdEDITP:
        event.Enable(!m_ring.empty());
        break;
    }
}

void DotTool::OnCmdStepBack(wxCommandEvent &)
{
    if (!m_ring.empty())
    {
        m_ring.pop_back();
        saveGeometry();
    }
}

void DotTool::OnCmdSetLength(wxCommandEvent &)
{
    if (m_PlotModel.setLength_User(m_ring, &m_owner, m_NextCoord))
    {
        m_ring.push_back(m_NextCoord);
        saveGeometry();
    }
}

bool DotTool::DoRecalc(const wxPoint &mousePosition)
{
    std::vector<geom::Coordinate> hint_seq;
    m_NextCoord = m_PlotModel.DoRecalc(m_ring, mousePosition, hint_seq);
    return true;
}

wxString DotTool::GetString() const
{
    return describeGeometry() + m_PlotModel.GetString(m_ring, m_NextCoord);
}

gceToolOptions *DotTool::create_to(wxWindow *parent)
{
    // set options
    auto to = new gceToolOptions(parent);

    to->AddNamedFlag(m_PlotModel.m_fAngleStep, "Angle step");
    to->AddEditableFloatList(m_PlotModel.m_angle, {15.0, 30.0, 45.0, 90.0});

    to->AddSeparator();

    to->AddNamedFlag(m_PlotModel.m_fBase, "Base line");
    to->AddEditableFloatList(m_PlotModel.m_base, {0.0});

    to->AddSeparator();

    to->AddNamedFlag(m_PlotModel.m_fLength, "Length step");
    to->AddEditableFloatList(m_PlotModel.m_lstep, {0.001, 0.01, 0.1, 1.0});

    to->Realize();
    return to;
}

