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

#include "MoveTool.h"
#include "editorfrm.h"
#include "tools/ToolOptions.h"
#include "type/entity.h"
#include <glm/gtc/matrix_transform.hpp>
#include "Canvas.h"

static const char *cursor_move_xpm[] = {
    "32 32 3 1",
    " 	c None",
    ".	c #000000",
    "+	c #FFFFFF",
    "..                              ",
    ".+...                           ",
    " .+++...                        ",
    " .++++++...                     ",
    " .+++++++++...                  ",
    "  .+++++++++++.                 ",
    "  .+++++++++..                  ",
    "  .++++++...                    ",
    "   .++++.     ..                ",
    "   .+++.     .++.               ",
    "   .+++.    .++++.              ",
    "    .++.     .++.               ",
    "    .+.   .  .++.  .            ",
    "    .+.  .+...++...+.           ",
    "     .  .++++++++++++.          ",
    "        .++++++++++++.          ",
    "         .+...++...+.           ",
    "          .  .++.  .            ",
    "             .++.               ",
    "            .++++.              ",
    "             .++.               ",
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

gceMoveTool::gceMoveTool(gceEditorFrame &owner) : gceToolBase(owner), m_PlotModel(getCanvas())
{
    this->Bind(wxEVT_LEFT_DOWN, &gceMoveTool::OnLeftDown, this);

    // popup handling
    this->Bind(wxEVT_MENU, &gceMoveTool::OnCmdCancel, this, cmdCANCEL);
    this->Bind(wxEVT_MENU, &gceMoveTool::OnCmdSetLength, this, cmdLENGTH);
    this->Bind(wxEVT_UPDATE_UI, &gceMoveTool::OnUpdateUIEvent, this, cmdFIRST, cmdLAST);

        // init Plot Model
    m_PlotModel.m_fBase = true;
    m_PlotModel.m_base = 0.0;

    m_CopyNum = 1;
    m_Coping = false;

    // create popup
    uic(cmdLENGTH, "Offset length\tL");
    uic(cmdCANCEL, "Cancel\t`").IconSVG(cancel_svg);

    SetCursor(0, cursor_move_xpm, 0, 0);
}

gceToolInfo gceMoveTool::GetInfo() const
{
    return gceToolInfo{"Move/copy", nullptr, "enter offset line"};
}

void gceMoveTool::BeginUse_Custom()
{}

bool gceMoveTool::EndUse_Custom()
{
    EndUseChild(); // for baseline
    m_ring.clear();
    this->postSelectionModelProps(false, glm::dmat4{1.0});
    m_PlotModel.postRenderClearLine(TEMP_LINE_ID1);
    return true;
}

void gceMoveTool::OnLeftDown(wxMouseEvent &event)
{
    if (!m_ring.empty())
    {
        DoEnter();
        EndUse();
    }
    else
    {
        m_PlotModel.AddPoint(m_ring, m_NextCoord);
    }
    event.Skip();
}

void gceMoveTool::Display()
{
    m_PlotModel.postRenderTempLine1(m_ring, this->m_NextCoord, this->HaveMouse());
    std::vector<glm::dmat4> poses;
    glm::dmat4 pose0{1.0};

    if (!m_ring.empty())
    {
        auto *proj = getCanvas()->getProj();
        geom::CoordinateXYZ cFront;
        m_ring.getFront(cFront);
        glm::dvec3 pos1, pos2;
        proj->fromInternal(pos1, cFront.pos);
        proj->fromInternal(pos2, m_NextCoord.pos);

        const glm::dvec3 move_vec = pos2 - pos1;
        if (m_Coping)
        {
            // display geometry copies
            for (int n = 1; n <= m_CopyNum; n++)
            {
                poses.emplace_back(glm::translate(pose0, move_vec * (double(n) / m_CopyNum)));
            }
        }
        else
        {
            pose0 = glm::translate(pose0, move_vec);
        }
    }
    this->postSelectionModelProps(false, pose0, poses);
}

void gceMoveTool::OnCmdCancel(wxCommandEvent &)
{
    m_ring.clear();
    EndUse();
}

void gceMoveTool::OnUpdateUIEvent(wxUpdateUIEvent &event)
{
    switch (event.GetId())
    {
    case cmdCANCEL:
        event.Enable(true);
        break;
    case cmdLENGTH:
        event.Enable(m_ring.empty());
        break;
    }
}

void gceMoveTool::OnCmdSetLength(wxCommandEvent &)
{

    if (m_PlotModel.setLength_User(m_ring, &m_owner, m_NextCoord))
    {
        DoEnter();
        EndUse();
    }
}

bool gceMoveTool::DoRecalc(const wxPoint &mousePosition)
{
    std::vector<geom::Coordinate> hint_seq;
    m_NextCoord = m_PlotModel.DoRecalc(m_ring, mousePosition, hint_seq);
    return true;
}

bool gceMoveTool::DoEnter()
{
    using namespace geom;

    auto *proj = getCanvas()->getProj();
    geom::CoordinateXYZ cFront;
    m_ring.getFront(cFront);
    glm::dvec3 pos1, pos2;
    proj->fromInternal(pos1, cFront.pos);
    proj->fromInternal(pos2, m_NextCoord.pos);
    const glm::dvec3 move_vec = pos2 - pos1;

    if (glm::length(move_vec) < gce::big_epsilon<double>()) return true;

    auto &sel = m_owner.getSelection();
    gceCommandGroup cmds(fmt::format("{}{} object(s)", m_Coping ? "Copy" : "Move", sel.size()));

    for (auto &ref : sel)
    {
        if (m_Coping)
        {
            auto *schema = ref.entity.get_schema();
            gceEntityVar moved{schema};
            for (auto &col : schema->getColumns())
            {
                if (!col.isPKEY() && !isGeometry(col.getType()))
                {
                    moved.assign(ref.entity, col.getIndex());
                }
            }
            auto geoProjected = ref.entity.get_geometry();
            ProjectInplace(*geoProjected);
            for (int n = 1; n <= m_CopyNum; n++)
            {
                std::unique_ptr<geom::Geometry> geoMoved(geoProjected->clone());
                geoMoved->Move(move_vec * (double(n) / m_CopyNum));
                UnProjectInplace(*geoMoved);
                moved.setGeometry(std::move(geoMoved));
                cmds.Insert(ref.id_table, gceEntityPacked{moved});
            }
        }
        else
        {
            gceEntityVar moved{ref.entity};
            auto *geo = moved.getGeometry();
            ProjectInplace(*geo);
            geo->Move(move_vec);
            UnProjectInplace(*geo);
            cmds.Update(ref.id_table, gceEntityPacked{moved}, ref.entity);
        }
    }
    m_ring.clear();
    return m_owner.getActionProcessor().postCommandGroup(cmds);
}

wxString gceMoveTool::GetString() const
{
    if (!m_ring.empty())
    {
        return m_PlotModel.GetString(m_ring, m_NextCoord);
    }
    return wxString();
}

gceEntityPacked gceMoveTool::getMovedCopy(const gceEntityPacked &rec, const glm::dvec3 &move_vec)
{
    gceEntityVar moved{rec};
    auto *g = moved.getGeometry();
    ProjectInplace(*g);
    g->Move(move_vec);
    UnProjectInplace(*g);
    return gceEntityPacked{moved};
}

gceToolOptions *gceMoveTool::create_to(wxWindow *parent)
{
    // set options
    auto to = new gceToolOptions(parent);

    to->AddNamedFlag(m_PlotModel.m_fAngleStep, "Angle step");
    to->AddEditableFloatList(m_PlotModel.m_angle, {15.0, 30.0, 45.0, 90.0});

    to->AddSeparator();

    to->AddNamedFlag(m_PlotModel.m_fLength, "Length step");
    to->AddEditableFloatList(m_PlotModel.m_lstep, {0.001, 0.01, 0.1, 1.0});

    to->AddSeparator();

    // Coping
    to->AddNamedFlag(m_Coping, "Copy, copies count");
    to->AddSpin(m_CopyNum, 1, 1000);

    to->Realize();
    return to;
}
