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

#include "IntersectionTool.h"
#include <alg/alg_geos.h>
#include "editorfrm.h"
#include "Canvas.h"
#include "type/entity.h"


/* XPM */
static const char *add_cross_xpm[] = {
    "32 32 5 1",
    " 	c None",
    ".	c #000000",
    "+	c #FFFFFF",
    "@	c #C9C9C9",
    "#	c #FF0000",
    "..                              ",
    ".+.                             ",
    ".++.                            ",
    ".+++.                           ",
    ".++++.                          ",
    ".+++++.                         ",
    ".++@+++.                        ",
    ".++@@+++.                       ",
    ".++@@@+++.                      ",
    ".++@@@@+++.                     ",
    ".++@@@@@+++.                    ",
    ".++@@@@@@++.                    ",
    ".++@@@@@@@.                     ",
    ".++@@@@@@.                      ",
    ".++@@@@@.                       ",
    ".++@@@..                        ",
    " .....                          ",
    "            ...                 ",
    "          ..   ..               ",
    "         .       .              ",
    "    .....#.#.#   .              ",
    "    .   #####.    .             ",
    "    .   .#####    .             ",
    "    .   #####.    .             ",
    "    .   ######   .              ",
    "    .    .###.   .              ",
    "    .     #.## ..               ",
    "    .        ..                 ",
    "    ..........                  ",
    "                                ",
    "                                ",
    "                                "};

gceIntersectionTool::gceIntersectionTool(gceEditorFrame &owner) : gceToolBase(owner)
{
    this->Bind(wxEVT_LEFT_DOWN, &gceIntersectionTool::OnLeftDown, this);
    this->Bind(wxEVT_MENU, &gceIntersectionTool::OnCmdFinish, this, cmdFINISH);

    uic(cmdFINISH, "Finish\t`").IconSVG(finish_svg);

    // create cursor
    SetCursor(0, add_cross_xpm, 0, 0);
}

gceIntersectionTool::~gceIntersectionTool()
{}

void gceIntersectionTool::BeginUse_Custom()
{}

bool gceIntersectionTool::EndUse_Custom()
{
    return true;
}

void gceIntersectionTool::OnLeftDown(wxMouseEvent &event)
{
    //if (!m_querySent)
    {
        auto &al = m_owner.getActiveLayer();

        umodelSelect2DMsg msg;
        msg.id_modelActive = al.id_model;
        msg.model_ids = this->m_owner.getModelsSelectable();
        msg.aoi = getCanvas()->calculateCursorAOI(10);
        msg.sender = gce::queueId::WORKSPACE;
        msg.limit = 7;
        getCanvas()->ctx().postModelQueue(std::move(msg));
        //m_querySent = true;
    }

    event.Skip();
}

void gceIntersectionTool::processSelectResult(const umodelSelectXDResultMsg &msg)
{
    auto &sel = m_owner.getSelection();
    for (auto &sd : msg.data)
    {
        //wxLogMessage(to_wxstring(fmt::format("{} {} {} {}", msg.data.size(), sd.id_table, sd.id_entity, sd.radius)));
        if (!sel.ContainsRec(sd.key))
        {
            // first element not in selection has smallest radius
            udataSelectIDMsg dmsg;
            dmsg.key = sd.key;
            dmsg.sender = gce::queueId::WORKSPACE;
            getCanvas()->ctx().postDataQueue(std::move(dmsg));
            //wxLogMessage(to_wxstring(fmt::format("{} {} {}", sd.key.id_table, sd.key.id_entity, sd.radius)));
            break;
        }
    }
}

void gceIntersectionTool::processSelectDataResult(const udataSelectReplyMsg &msg)
{
    if (!msg.rows.empty())
    {
        if (auto sub = msg.rows.front().get_geometry_GEOS(); sub)
        {
            gceCommandGroup cmds("geometry intersection");
            for (auto &ref : m_owner.getSelection())
            {
                if (auto geomResult = geom::alg_geos::GeomIntersection(ref.entity.get_geometry_GEOS(), sub); geomResult)
                {
                    if (GEOSisEmpty(geomResult.get()))
                    {
                        cmds.Delete(ref.id_table, ref.entity);
                    }
                    else
                    {
                        gceEntityVar enResult{ref.entity, geomResult};
                        cmds.Update(ref.id_table, gceEntityPacked{enResult}, ref.entity);
                    }
                }
            }
            m_owner.getActionProcessor().postCommandGroup(cmds);
        }
        else
        {
            wxLogMessage("get_geometry_GEOS error");
        }
    }
}

void gceIntersectionTool::OnCmdFinish(wxCommandEvent &)
{
    this->EndUse();
}

wxString gceIntersectionTool::GetString() const
{
    return{};
}
