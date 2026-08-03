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
#include <glm/gtc/matrix_transform.hpp>

#include "RotateTool.h"
#include "Canvas.h"
#include "editorfrm.h"
#include "geom/Geometry.h"
#include "tools/ToolOptions.h"
#include "type/entity.h"


/* XPM */
static  const char *Cur_rot_xpm[] = {
    "32 32 4 1",
    " 	c None",
    ".	c #000000",
    "+	c #FFFFFF",
    "@	c #FEFEFE",
    "..                              ",
    ".+...                           ",
    " .+++...                        ",
    " .++++++...                     ",
    " .+++++++++...                  ",
    "  .+++++++++++.                 ",
    "  .+++++++++..                  ",
    "  .++++++...                    ",
    "   .++++.                       ",
    "   .+++.      ...               ",
    "   .+++.    ..+++.              ",
    "    .++.   .++++++.             ",
    "    .+.    .+...+++.            ",
    "    .+.   .+.   .++.            ",
    "     .    ..    .++.            ",
    "              . .++.            ",
    "             .+.+++.            ",
    "            .+++@++.            ",
    "            .++@++.             ",
    "           .++++..              ",
    "            ..++.               ",
    "              ..                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                "};

RotateTool::RotateTool(gceEditorFrame &owner) : gceToolBase(owner),
m_PlotModel(getCanvas())
{
    this->Bind(wxEVT_LEFT_DOWN, &RotateTool::OnLeftDown, this);
    this->Bind(wxEVT_KEY_DOWN, &RotateTool::OnKeyDown, this);
    this->Bind(wxEVT_KEY_UP, &RotateTool::OnKeyUp, this);
    this->Bind(wxEVT_MENU, &RotateTool::OnCmdCancel, this, cmdCANCEL);
    this->Bind(wxEVT_UPDATE_UI, &RotateTool::OnUpdateUIEvent, this, cmdFIRST, cmdLAST);

    uic(cmdCANCEL, "Cancel\t`").IconSVG(cancel_svg);

    // create cursor
    SetCursor(0, Cur_rot_xpm, 0, 0);
}


gceToolInfo RotateTool::GetInfo() const
{
    return gceToolInfo{"Rotate/copy", nullptr, "use 2-lines for rotation angle (Ctrl+click - for center)"};
}

void RotateTool::BeginUse_Custom()
{
    if (auto &sel = m_owner.getSelection(); !sel.empty())
    {
        auto g = sel.begin()->entity.get_geometry();
        m_ring.push_back(geom::Coordinate(g->GetBaseCoord()));
    }
}


bool RotateTool::EndUse_Custom()
{
    m_ring.clear();
    this->postSelectionModelProps(false, glm::dmat4{1.0});
    m_PlotModel.postRenderClearLine(TEMP_LINE_ID1);
    m_PlotModel.postRenderClearLine(MAIN_LINE_ID);
    return true;
}


void RotateTool::OnLeftDown(wxMouseEvent &event)
{
    if (m_owner.getSelection().empty()) return;

    const size_t nPoints = m_ring.size();

    if (event.ControlDown())
    {
        // set rotation center
        m_ring.set(m_NextCoord, nPoints > 1 ? 1 : 0);
    }
    else
    {
        if (nPoints == 1)
        {
            m_ring.Insert(0, m_NextCoord);
        }
        else
        {
            // m_ring.size()==2, try enter
            if (DoEnter())
            {
                EndUse();
            }
        }
    }

    m_PlotModel.postModelMainLine(m_ring);
    event.Skip();
}

static glm::dmat4 rotateBaseMat(const glm::dvec2 &base, double angle)
{
    // construct rotate-around-base matrix
    glm::dmat4 m(1.0);
    m = glm::translate(m, glm::dvec3(base, 0.0));
    m = glm::rotate(m, angle, glm::dvec3(0.0, 0.0, 1.0));
    m = glm::translate(m, glm::dvec3(-base, 0.0));
    return m;
}

void RotateTool::Display()
{
    m_PlotModel.postRenderTempLine1(m_ring, m_NextCoord, this->HaveMouse());
    if (m_owner.getSelection().empty()) return;

    std::vector<glm::dmat4> poses;
    glm::dmat4 pose0{1.0};

    if (m_ring.size() == 2)
    {
        auto *proj = getCanvas()->getProj();
        geom::CoordinateXY cFront, cBack;
        m_ring.getFront(cFront);
        m_ring.getBack(cBack);
        glm::dvec2 pos1, pos2, posC;
        proj->fromInternal(posC, cBack.pos);
        proj->fromInternal(pos1, cFront.pos);
        proj->fromInternal(pos2, m_NextCoord.pos);

        double angle = gce::angle(pos2 - posC, pos1 - posC);
        if (m_Coping)
        {
            // display geometry copies
            for (int n = 1; n <= m_CopyNum; n++)
            {
                poses.emplace_back(rotateBaseMat(posC, angle * (double(n) / m_CopyNum)));
            }
        }
        else
        {
            pose0 = rotateBaseMat(posC, angle);
        }
    }
    this->postSelectionModelProps(false, pose0, poses);
}


void RotateTool::OnUpdateUIEvent(wxUpdateUIEvent &)
{}


void RotateTool::OnCmdCancel(wxCommandEvent &)
{
    EndUse();
}
void RotateTool::OnKeyDown(wxKeyEvent &event)
{
    if (event.GetKeyCode() == WXK_CONTROL)
    {
        m_basePointInput = true;
    }
    event.Skip();
}
void RotateTool::OnKeyUp(wxKeyEvent &event)
{
    if (event.GetKeyCode() == WXK_CONTROL)
    {
        m_basePointInput = false;
    }
    event.Skip();
}


bool RotateTool::DoRecalc(const wxPoint &mousePosition)
{
    std::vector<geom::Coordinate> hint_seq;
    m_NextCoord = m_PlotModel.DoRecalc(m_ring, mousePosition, hint_seq);
    return true;
}

bool RotateTool::DoEnter()
{
    if (m_ring.size() < 2) return false;

    auto &sel = m_owner.getSelection();

    if (sel.empty()) return true;

    auto *proj = getCanvas()->getProj();
    geom::CoordinateXY cFront, cBack;
    m_ring.getFront(cFront);
    m_ring.getBack(cBack);
    glm::dvec2 pos1, pos2, posC;
    proj->fromInternal(posC, cBack.pos);
    proj->fromInternal(pos1, cFront.pos);
    proj->fromInternal(pos2, m_NextCoord.pos);

    double angle = gce::angle(pos2 - posC, pos1 - posC);
    gceCommandGroup cmds(fmt::format("{}{} object(s)", m_Coping ? "Rotated copy" : "Rotate", sel.size()));

    for (auto &ref : sel)
    {
        if (m_Coping)
        {
            auto *schema = ref.entity.get_schema();
            gceEntityVar rotated{schema};
            for (auto &col : schema->getColumns())
            {
                if (!col.isPKEY() && !isGeometry(col.getType()))
                {
                    rotated.assign(ref.entity, col.getIndex());
                }
            }
            auto geoProjected = ref.entity.get_geometry();
            ProjectInplace(*geoProjected);
            for (int n = 1; n <= m_CopyNum; n++)
            {
                std::unique_ptr<geom::Geometry> geoMoved(geoProjected->clone());
                geoMoved->RotateAroundBase(posC, angle * (double(n) / m_CopyNum));
                UnProjectInplace(*geoMoved);
                rotated.setGeometry(std::move(geoMoved));
                cmds.Insert(ref.id_table, gceEntityPacked{rotated});
            }
        }
        else
        {
            gceEntityVar rotated{ref.entity};
            auto *geo = rotated.getGeometry();
            ProjectInplace(*geo);
            geo->RotateAroundBase(posC, angle);
            UnProjectInplace(*geo);
            cmds.Update(ref.id_table, gceEntityPacked{rotated}, ref.entity);
        }
    }
    if (m_owner.getActionProcessor().postCommandGroup(cmds))
    {
        m_ring.clear();
        return true;
    }
    return false;
}


wxString RotateTool::GetString() const
{
    if (m_ring.size() == 2)
    {
        geom::CoordinateXY cFront, c1; // rotation center
        m_ring.get(cFront, 1);
        m_ring.get(c1, 0);

        double ang = gce::angle(glm::dvec2(m_NextCoord.pos) - cFront.pos, c1.pos - cFront.pos);
    //	ang = fmod(ang, M_PI);
        return  wxString::Format(wxT("angle = %.3f"), glm::degrees(ang));
    }
    return wxString();
}


gceToolOptions *RotateTool::create_to(wxWindow *parent)
{
    auto to = new gceToolOptions(parent);

    to->AddNamedFlag(m_PlotModel.m_fAngleStep, "Rotation angle step");
    to->AddEditableFloatList(m_PlotModel.m_angle, {15.0, 30.0, 45.0, 90.0});

    to->AddSeparator();

    to->AddNamedFlag(m_Coping, "Copy, copies count");
    to->AddSpin(m_CopyNum, 1, 1000);

    to->Realize();
    return to;
}


