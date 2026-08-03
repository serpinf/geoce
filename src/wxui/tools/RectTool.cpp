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

#include "RectTool.h"
#include "tools/ToolOptions.h"
#include "editorfrm.h"
#include "BaseLineTool.h"

/* XPM */
static const char *Input_rect_xpm[] = {
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
    "     +++++++++++                ",
    "                 ............   ",
    "                 .++++++++++.   ",
    "                 .+@@@@@@@@+.   ",
    "                 .+@@@@@@@@+.   ",
    "                 .+@@@@@@@@+.   ",
    "                 .+@@@@@@@@+.   ",
    "                 .+@@@@@@@@+.   ",
    "                 .++++++++++.   ",
    "                 ............   ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                "};

static constexpr char rectangle_svg[] = R"rawsvg(<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24"><path fill="currentColor" d="M2 4h20v16H2z"/></svg>)rawsvg";

gceRectangleTool::gceRectangleTool(gceEditorFrame &owner) : gceInputToolBase(owner, 2), m_PlotModel(getCanvas())
{
    this->Bind(wxEVT_LEFT_DOWN, &gceRectangleTool::OnLeftDown, this);

    // popup handling
    this->Bind(wxEVT_MENU, &gceRectangleTool::OnCmdCancel, this, cmdCANCEL);
    this->Bind(wxEVT_MENU, &gceRectangleTool::OnCmdBaseLine, this, cmdBASE_LINE);
    this->Bind(wxEVT_MENU, &gceRectangleTool::OnCmdSetLength, this, cmdLENGTH);
    this->Bind(wxEVT_MENU, &gceRectangleTool::OnCmdStepBack, this, cmdSTEP_BACK);
    this->Bind(wxEVT_UPDATE_UI, &gceRectangleTool::OnUpdateUIEvent, this, cmdFIRST, cmdLAST);

    m_PlotModel.m_fAngleStep = true;
    m_PlotModel.m_angle = 90.0;

    uic(cmdSTEP_BACK, "Step back\tZ").IconSVG(step_back_svg);
    uic(cmdLENGTH, "Length\tL");
    uic(cmdCANCEL, "Cancel").IconSVG(cancel_svg);
    uic.separtor();
    uic(cmdBASE_LINE, "Base line\tCtrl+G").IconSVG(base_angle_svg);

    // create cursor
    SetCursor(0, Input_rect_xpm, 11, 0);
}

gceToolInfo gceRectangleTool::GetInfo() const
{
    return gceToolInfo{"Rectangle", rectangle_svg, "3-point rectangle"};
}

void gceRectangleTool::BeginUse_Custom()
{
    ConnectLayer();
}

std::unique_ptr<geom::Geometry> gceRectangleTool::createInputGeometry(geom::CoordinateType cooType) const
{
    if (m_ring.size() > 2)
    {
        geom::CoordinateXYZ c0, c1;
        m_ring.get(c0, 0);
        m_ring.get(c1, 1);

        geom::CoordinateSeq dest(geom::CoordinateType::XY);
        dest.reserve(5);
        dest.push_back(c0);
        dest.push_back(c1);

        dest.push_back(m_NextCoord);

        dest.push_back(geom::CoordinateXY(m_NextCoord.pos + c0.pos - c1.pos));

        dest.push_back(c0);

        return geom::Geometry::Create(2, cooType, dest);
    }
    return {};
}

void gceRectangleTool::OnLeftDown(wxMouseEvent &event)
{
    TryEnterOrAddPoint();

    event.Skip();
}

void gceRectangleTool::Display()
{
    if (m_ring.size() < 2)
    {
        m_PlotModel.postRenderTempLine1(m_ring, m_NextCoord, this->HaveMouse());
    }
    else if (this->HaveMouse())
    {
        const auto c0 = m_ring.get_xy(0);
        const auto c1 = m_ring.get_xy(1);
        const auto c2 = geom::CoordinateXY(m_NextCoord);
        const auto c3 = geom::CoordinateXY(glm::dvec2(m_NextCoord.pos) + c0.pos - c1.pos);

        m_PlotModel.postRenderLine(TEMP_LINE_ID1, {c0, c1, c2, c3, c0}, 1.0, false);
    }
}

void gceRectangleTool::OnCmdCancel(wxCommandEvent &)
{
    m_ring.clear();
    saveGeometry();
}

void gceRectangleTool::OnUpdateUIEvent(wxUpdateUIEvent &event)
{
    switch (event.GetId())
    {
    case cmdCANCEL:
        event.Enable(!m_ring.empty());
        break;
    case cmdBASE_LINE:
        event.Enable(m_ring.size() < 2);
        break;
    case cmdSTEP_BACK:
        event.Enable(!m_ring.empty());
        break;
    }
}

void gceRectangleTool::OnCmdSetLength(wxCommandEvent &)
{
    if (m_PlotModel.setLength_User(m_ring, &m_owner, m_NextCoord))
    {
        TryEnterOrAddPoint();
    }
}

void gceRectangleTool::OnCmdStepBack(wxCommandEvent &)
{
    if (!m_ring.empty())
    {
        m_ring.pop_back();
        saveGeometry();
    }
}

void gceRectangleTool::OnCmdBaseLine(wxCommandEvent &)
{
    startChild<gceBaseLineTool>();
}

bool gceRectangleTool::DoRecalc(const wxPoint &mousePosition)
{
    std::vector<geom::Coordinate> hint_seq;
    m_NextCoord = m_PlotModel.DoRecalc(m_ring, mousePosition, hint_seq);
    saveGeometry();
    return true;
}

wxString gceRectangleTool::GetString() const
{
    size_t nPts = m_ring.size();
    if (nPts == 0) return wxString();

    wxString w, h;
    geom::Coordinate c0, c1;
    m_ring.get(c0, 0);
    if (nPts == 1)
    {
        w = m_PlotModel.distanceHR(c0, m_NextCoord);
        h = wxT("N/A");
    }
    else /*if (nPts==2)*/
    {
        m_ring.get(c1, 1);
        w = m_PlotModel.distanceHR(c0, c1);
        h = m_PlotModel.distanceHR(c1, m_NextCoord);
    }
    return wxString::Format("length = %s; width = %s", w, h);
}
gceToolOptions *gceRectangleTool::create_to(wxWindow *parent)
{
    auto to = new gceToolOptions(parent);
    to->AddNamedFlag(m_PlotModel.m_fBase, "Base line:");
    to->AddEditableFloatList(m_PlotModel.m_base, {0.0});

    to->AddSeparator();

    to->AddNamedFlag(m_PlotModel.m_fLength, "Length step:");
    to->AddEditableFloatList(m_PlotModel.m_lstep, {0.001, 0.01, 0.1, 1.0});

    to->Realize();
    return to;
}

void gceRectangleTool::TryEnterOrAddPoint()
{
    m_ring.push_back(m_NextCoord);

    if (m_ring.size() < 2)
    {
        saveGeometry();
    }
    else // nPts == 2
    {
        DoEnter();
    }
}
