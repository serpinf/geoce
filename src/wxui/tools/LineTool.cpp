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

#include "LineTool.h"

#include "ToolOptions.h"
#include "VertexEditor.h"
#include "BaseLineTool.h"
#include "editorfrm.h"


/* XPM */
static const char *Input_line_xpm[] = {
    "32 32 4 1",
    " 	c None",
    ".	c #FFFFFF",
    "+	c #000000",
    "@	c #838383",
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
    "     +.........+                ",
    "     +++++++++++         .+.    ",
    "                         .+.    ",
    "                     .   .+.    ",
    "                    .@.  .+.    ",
    "                   .@+@. .+.    ",
    "                  .@+.+@..+.    ",
    "                 .@+. .+@.+.    ",
    "                .@+.   .+.+.    ",
    "               .@+.     .++.    ",
    "               @+.       .+.    ",
    "                .         .     ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                "};

static constexpr char polyline_svg[] = R"rawsvg(<svg xmlns="http://www.w3.org/2000/svg" width="100" height="100" viewBox="0 0 100 100"><path fill="currentColor" d="M33.162 19.463a3.5 3.5 0 0 0-3.111 1.974L9.34 64.239a3.5 3.5 0 0 0 1.626 4.674a3.5 3.5 0 0 0 4.676-1.625l19.453-40.203l35.412 9.361L84.182 78.13a3.5 3.5 0 0 0 4.418 2.234a3.5 3.5 0 0 0 2.234-4.416l-14.268-43.49a3.5 3.5 0 0 0-2.431-2.293l-40.04-10.586a3.5 3.5 0 0 0-.933-.115" color="currentColor"/></svg>)rawsvg";

gceLineTool::gceLineTool(gceEditorFrame &owner) : gceInputToolBase(owner, 1),
m_PlotModel(getCanvas())
{
    this->Bind(wxEVT_LEFT_DOWN, &gceLineTool::OnLeftDown, this);

// popup handling
    this->Bind(wxEVT_MENU, &gceLineTool::OnCmdEnter, this, cmdENTER);
    this->Bind(wxEVT_MENU, &gceLineTool::OnCmdCancel, this, cmdCANCEL);
    this->Bind(wxEVT_MENU, &gceLineTool::OnCmdEditP, this, cmdEDITP);
    this->Bind(wxEVT_MENU, &gceLineTool::OnCmdStepBack, this, cmdSTEP_BACK);
    this->Bind(wxEVT_MENU, &gceLineTool::OnCmdSetLength, this, cmdLENGTH);
    this->Bind(wxEVT_MENU, &gceLineTool::OnCmdBaseLine, this, cmdBASE_LINE);
    this->Bind(wxEVT_UPDATE_UI, &gceLineTool::OnUpdateUIEvent, this, cmdFIRST, cmdLAST);

    uic(cmdENTER, "Enter\tENTER").IconSVG(enter_svg);
    uic(cmdSTEP_BACK, "Step back\tZ").IconSVG(step_back_svg);
    uic(cmdEDITP, "&Edit\t`").IconSVG(vertexedit_svg);
    uic(cmdLENGTH, "Length\tL");
    uic(cmdCANCEL, "Cancel").IconSVG(cancel_svg);
    uic.separtor();
    uic(cmdBASE_LINE, "Set base line\tCTRL+G").IconSVG(base_angle_svg);

    SetCursor(0, Input_line_xpm, 11, 0);
}

gceToolInfo gceLineTool::GetInfo() const
{
    return gceToolInfo{"Line", polyline_svg, "use 'Enter' to finish"};
}

void gceLineTool::BeginUse_Custom()
{
    ConnectLayer();
}

bool gceLineTool::EndUse_Custom()
{
    bool result = false;
    if (EndUseChild())
    {
        DisconnectLayer();
        m_ring.clear();
        result = true;

    }
    return result;
}

void gceLineTool::updateModel()
{}

void gceLineTool::OnLeftDown(wxMouseEvent &event)
{
    m_PlotModel.AddPoint(m_ring, m_NextCoord);
    saveGeometry();

    event.Skip();
}

void gceLineTool::Display()
{
    m_PlotModel.postRenderTempLine1(m_ring, m_NextCoord, this->HaveMouse());
}

void gceLineTool::OnCmdEnter(wxCommandEvent &)
{
    DoEnter();
}

void gceLineTool::OnCmdCancel(wxCommandEvent &)
{
    m_ring.clear();
    saveGeometry();
}

void gceLineTool::OnCmdEditP(wxCommandEvent &)
{
    if (!m_ring.empty())
    {
        startChild<gceVertexEditor>();
    }
}

void gceLineTool::OnCmdBaseLine(wxCommandEvent &)
{
    startChild<gceBaseLineTool>();
}

void gceLineTool::OnUpdateUIEvent(wxUpdateUIEvent &event)
{
    switch (event.GetId())
    {
    case cmdENTER:
        event.Enable(CanInsert());
        break;
    case cmdCANCEL:
    case cmdSTEP_BACK:
    case cmdEDITP:
    case cmdLENGTH:
        event.Enable(!m_ring.empty());
        break;
    }
}

void gceLineTool::OnCmdStepBack(wxCommandEvent &)
{
    if (!m_ring.empty())
    {
        m_ring.pop_back();
        saveGeometry();
    }
}

void gceLineTool::OnCmdSetLength(wxCommandEvent &)
{
    if (m_PlotModel.setLength_User(m_ring, &m_owner, m_NextCoord))
    {
        m_ring.push_back(m_NextCoord);
        saveGeometry();
    }
}

bool gceLineTool::DoRecalc(const wxPoint &mousePosition)
{
    std::vector<geom::Coordinate> hint_seq;
    m_NextCoord = m_PlotModel.DoRecalc(m_ring, mousePosition, hint_seq);
    return true;
}

wxString gceLineTool::GetString() const
{
    return describeGeometry() + m_PlotModel.GetString(m_ring, m_NextCoord);
}

gceToolOptions *gceLineTool::create_to(wxWindow *parent)
{
    // set options
    auto to = new gceToolOptions(parent);

    to->AddNamedFlag(m_PlotModel.m_fAngleStep, "Angle step");
    to->AddEditableFloatList(m_PlotModel.m_angle, {15.0, 30.0, 45.0, 90.0});

    to->AddSeparator();

    to->AddNamedFlag(m_PlotModel.m_fBase, "Base line");
    to->AddEditableFloatList(m_PlotModel.m_base, {0.0, 15.0, 30.0, 45.0});

    to->AddSeparator();

    to->AddNamedFlag(m_PlotModel.m_fLength, "Length step");
    to->AddEditableFloatList(m_PlotModel.m_lstep, {0.001, 0.01, 0.1, 1.0});

    to->Realize();
    return to;
}

