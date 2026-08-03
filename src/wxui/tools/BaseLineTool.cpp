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

#include "BaseLineTool.h"

static const char *base_line_xpm[] = {
    "32 32 7 1",
    " 	c None",
    ".	c #000000",
    "+	c #FFFFFF",
    "@	c #838383",
    "#	c #FFFF00",
    "$	c #FF7600",
    "%	c #FF0000",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "               .                ",
    "              .+.               ",
    "              .+.               ",
    "              .+.               ",
    "              .+.               ",
    "              .+.               ",
    "              .+.               ",
    "              .+.               ",
    "              .+.               ",
    "      ........ . ........       ",
    "     .++++++++. .++++++++.     @",
    "      ........ . ........     @ ",
    "              .+.@        .  @  ",
    "              .+. @      .#.@   ",
    "              .+.  @     .#$.   ",
    "              .+.   @   .#$%%.  ",
    "              .+.    @ .#$%..   ",
    "              .+.     .#$%.     ",
    "              .+.    .#$%.      ",
    "              .+.   .#$%.       ",
    "               .  ..#$%. @      ",
    "                 .##$%.   @     ",
    "                  .$%.     @    ",
    "                  @.%.      @   ",
    "                 @  .        @  ",
    "                @               ",
    "               @                "};


gceBaseLineTool::gceBaseLineTool(gceEditorFrame &owner) : gceToolBase(owner), m_PlotModel(getCanvas())
{
    this->Bind(wxEVT_LEFT_DOWN, &gceBaseLineTool::OnLeftDown, this);
    this->Bind(wxEVT_MENU, &gceBaseLineTool::OnCmdCancel, this, cmdCANCEL);

    // create popup
    uic(cmdCANCEL, "Cancel\t`");

    // create cursor
    SetCursor(0, base_line_xpm, 15, 15);
}

void gceBaseLineTool::BeginUse_Custom()
{}

bool gceBaseLineTool::EndUse_Custom()
{
    m_CoordSeq.clear();
    return true;
}

void gceBaseLineTool::OnLeftDown(wxMouseEvent &)
{
    if (m_CoordSeq.empty())
    {
        m_PlotModel.AddPoint(m_CoordSeq, m_NextCoord);
    }
    else if (m_CoordSeq.size() == 1)
    {
        setBaseForParent(m_CoordSeq.front<true, true>(), m_NextCoord);
        EndUse();
    }
}

void gceBaseLineTool::setBaseForParent(const geom::Coordinate &c0, const geom::Coordinate &c1)
{
    if (auto tool = getParent())
    {
        auto vec = c0.compareTo(c1) < 0 ? c1.pos - c0.pos : c0.pos - c1.pos;
        tool->setBaseLine(gce::azimuth(vec), true);
    }
}

void gceBaseLineTool::Display()
{
    if (m_CoordSeq.size() == 1)
    {
        m_PlotModel.postRenderTempLine1(m_CoordSeq, m_NextCoord, this->HaveMouse());
    }

    //using namespace geom;
    //auto* ctx = getCanvas();
    //if( m_CoordSeq.size() == 2 )
    //{
        //glPushAttrib(GL_LINE_BIT);
            // draw base segment if any
            //glColor3f(0.5f,0.0f,0.0f);
            //glLineWidth(3);
            //ctx.DrawLine(m_CoordSeq[0].pos, m_CoordSeq[1].pos);
            //glColor3f(1.0f,1.0f,0.0f);
            //glLineWidth(1);
            //ctx.DrawLine(m_CoordSeq[0].pos, m_CoordSeq[1].pos);
        //glPopAttrib();
    //}
}

bool gceBaseLineTool::DoRecalc(const wxPoint &mousePosition)
{
    std::vector<geom::Coordinate> hint_seq;
    m_NextCoord = m_PlotModel.DoRecalc(m_CoordSeq, mousePosition, hint_seq);
    return true;
}

wxString gceBaseLineTool::GetString() const
{
    return ("Enter base line");
}

void gceBaseLineTool::OnCmdCancel(wxCommandEvent &)
{
    EndUse();
}
