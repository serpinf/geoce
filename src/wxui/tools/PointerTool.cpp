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

#include "PointerTool.h"
#include "Canvas.h"
#include "editorfrm.h"
#include "type/entity.h"
#include "alg/alg_geos.h"
#include "geom/MGeometry.h"

#include "MoveTool.h"
#include "RotateTool.h"
#include "DifferenceTool.h"
#include "IntersectionTool.h"
#include "VertexEditor.h"

static constexpr char deselect_svg[] = R"rawsvg(<symbol viewBox="0 0 24 24" id="deselect"><title>deselect</title><path fill="currentColor" d="m19.833 21.254l-4.946-4.946H7.692V9.114L2.746 4.167l.708-.713l17.092 17.092zm-11.14-5.946h5.194l-5.195-5.194zm7.615-1.825l-1-1v-3.79h-3.79l-1-1h5.79zM5.23 18.769V20q-.508 0-.87-.362T4 18.77zM4 16.308v-1.231h1.23v1.23zm0-3.693v-1.23h1.23v1.23zm0-3.692v-1.23h1.23v1.23zM7.692 20v-1.23h1.231V20zm0-14.77V4h1.231v1.23zM11.385 20v-1.23h1.23V20zm0-14.77V4h1.23v1.23zM15.077 20v-1.23h1.23V20zm0-14.77V4h1.23v1.23zm3.692 11.078v-1.231H20v1.23zm0-3.693v-1.23H20v1.23zm0-3.692v-1.23H20v1.23zm0-3.692V4q.508 0 .87.362T20 5.23z"></path></symbol>)rawsvg";
static constexpr char shapeunion_svg[] = R"rawsvg(<symbol viewBox="0 0 24 24" id="shape-union-24-regular"><title>shape-union-24-regular</title><path fill="currentColor" d="M5.25 2A3.25 3.25 0 0 0 2 5.25v8q0 .15.013.297a3.2 3.2 0 0 0 .542 1.52c.258.382.596.707.989.95a3.2 3.2 0 0 0 1.409.47h.006a3 3 0 0 0 .291.013H7.5v2.25a3.24 3.24 0 0 0 2.226 3.085A3.2 3.2 0 0 0 10.75 22h8A3.25 3.25 0 0 0 22 18.75v-8a3.246 3.246 0 0 0-3.25-3.25H16.5V5.25q0-.15-.013-.297a3.2 3.2 0 0 0-.542-1.52a3.3 3.3 0 0 0-.989-.95a3.2 3.2 0 0 0-1.409-.47h-.006A3 3 0 0 0 13.25 2zm.69 1.5L3.5 5.94v-.69c0-.966.784-1.75 1.75-1.75zM3.5 8.06L8.06 3.5h1.88L3.5 9.94zm8.56-4.56h1.19q.31.002.588.101L3.601 13.838a1.8 1.8 0 0 1-.101-.588v-1.19zm2.84 1.162q.1.277.101.588v1.19L6.44 15H5.25q-.31-.002-.588-.101zm.104 3.895A.5.5 0 0 0 15.5 9h.94L9 16.44v-.94a.5.5 0 0 0-.443-.497zM18.561 9h.189c.468 0 .893.184 1.207.483L9.483 19.957A1.74 1.74 0 0 1 9 18.75v-.19zm1.939 2.06v1.88l-7.56 7.56h-1.88zm0 4v1.88l-3.56 3.56h-1.88zm-.034 4.035a1.75 1.75 0 0 1-1.371 1.371z"></path></symbol>)rawsvg";
static constexpr char shapeintersection_svg[] = R"rawsvg(<symbol viewBox="0 0 16 16" id="shape-intersect-16-regular"><title>shape-intersect-16-regular</title><path fill="currentColor" d="M2 4a2 2 0 0 1 2-2h5a2 2 0 0 1 2 2v1h1a2 2 0 0 1 2 2v5a2 2 0 0 1-2 2H7a2 2 0 0 1-2-2v-1H4a2 2 0 0 1-2-2zm8 0a1 1 0 0 0-1-1H4a1 1 0 0 0-1 1v5a1 1 0 0 0 1 1h1V7a2 2 0 0 1 2-2h3zM6 9.707V10h1.293L10 7.293V6h-.293zM8.293 6H7a1 1 0 0 0-1 1v1.293zM11 9a2 2 0 0 1-.04.403A2 2 0 0 1 9 11H6v1a1 1 0 0 0 1 1h5a1 1 0 0 0 1-1V7a1 1 0 0 0-1-1h-1zm-2.293 1H9a1 1 0 0 0 1-1v-.293z"></path></symbol>)rawsvg";
static constexpr char shapesplit_svg[] = R"rawsvg(<symbol viewBox="0 0 24 24" id="horizontal-split"><title>horizontal-split</title><path fill="none" stroke="currentColor" stroke-linecap="round" stroke-linejoin="round" stroke-width="1.5" d="M10 12H2m0 0l3.5-3.5M2 12l3.5 3.5M14 12h8m0 0l-3.5-3.5M22 12l-3.5 3.5M10 21V3m4 18V3"></path></symbol>)rawsvg";
static constexpr char shapesubtruct_svg[] = R"rawsvg(<symbol viewBox="0 0 24 24" id="shape-subtract"><title>shape-subtract</title><g fill="none"><path fill="currentColor" d="M12.748 2.75h-7a3 3 0 0 0-3 3v7a3 3 0 0 0 3 3h2.504v-4.5a3 3 0 0 1 3-3h4.496v-2.5a3 3 0 0 0-3-3"></path><path stroke="currentColor" stroke-linecap="round" stroke-linejoin="round" stroke-width="1.5" d="M8.252 15.75v2.5a3 3 0 0 0 3 3h7a3 3 0 0 0 3-3v-7a3 3 0 0 0-3-3h-2.504m-7.496 7.5v-4.5a3 3 0 0 1 3-3h4.496m-7.496 7.5H5.748a3 3 0 0 1-3-3v-7a3 3 0 0 1 3-3h7a3 3 0 0 1 3 3v2.5"></path></g></symbol>)rawsvg";

static constexpr char move_svg[] = R"rawsvg(<symbol viewBox="0 0 24 24" id="arrow-move-24-filled"><title>arrow-move-24-filled</title><path fill="currentColor" d="M15.457 6.457a1 1 0 0 1-1.414 0L13 5.414V8.25a1 1 0 1 1-2 0V5.414L9.957 6.457a1 1 0 0 1-1.414-1.414l2.75-2.75a1 1 0 0 1 1.414 0l2.75 2.75a1 1 0 0 1 0 1.414m-9 7.586a1 1 0 1 1-1.414 1.414l-2.75-2.75a1 1 0 0 1 0-1.414l2.75-2.75a1 1 0 0 1 1.414 1.414L5.414 11H8.25a1 1 0 1 1 0 2H5.414zm12.5 1.414a1 1 0 0 1-1.414-1.414L18.586 13H15.75a1 1 0 1 1 0-2h2.836l-1.043-1.043a1 1 0 0 1 1.414-1.414l2.75 2.75a1 1 0 0 1 0 1.414zM11 18.586V15.75a1 1 0 1 1 2 0v2.836l1.043-1.043a1 1 0 0 1 1.414 1.414l-2.75 2.75a1 1 0 0 1-1.414 0l-2.75-2.75a1 1 0 1 1 1.414-1.414z"></path></symbol>)rawsvg";
static constexpr char rotateshape_svg[] = R"rawsvg(<symbol viewBox="0 0 24 24" id="shape-rotate-ccw"><title>shape-rotate-ccw</title><path fill="currentColor" d="M12 2C9.04 2 6.33 3.29 4.46 5.46L2 3v6h6L5.88 6.88C7.38 5.08 9.59 4 12 4c4.41 0 8 3.59 8 8h2c0-5.51-4.49-10-10-10" class="b"></path><path fill="currentColor" d="M12.71 6.93c-.38-.38-1.04-.38-1.41 0l-6.36 6.36a.996.996 0 0 0 0 1.41l6.36 6.36c.19.19.44.29.71.29s.52-.11.71-.29l6.36-6.36a.996.996 0 0 0 0-1.41l-6.36-6.36ZM12 18.95L7.05 14L12 9.05L16.95 14z" class="b"></path></symbol>)rawsvg";

static constexpr char layeralt_svg[] = R"rawsvg(<symbol viewBox="0 0 24 24" id="layers-alt"><title>layers-alt</title><path fill="currentColor" d="M21 2H9a1 1 0 0 0-1 1v4H6a1 1 0 0 0-1 1v4H3a1 1 0 0 0-1 1v8a1 1 0 0 0 1 1h8a1 1 0 0 0 1-1v-2h4a1 1 0 0 0 1-1v-2h4a1 1 0 0 0 1-1V3a1 1 0 0 0-1-1M10 20H4v-6h6Zm5-3h-3v-4a1 1 0 0 0-1-1H7V9h8Zm5-3h-3V8a1 1 0 0 0-1-1h-6V4h10Z"></path></symbol>)rawsvg";

static constexpr char pointer_svg[] = R"rawsvg(<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24"><path fill="none" stroke="currentColor" stroke-linecap="round" stroke-linejoin="round" stroke-width="1.4" d="m6.244 3.114l12.298 8.66A.693.693 0 0 1 18.346 13l-4.62.877a.565.565 0 0 0-.334.82l2.31 4.377a.693.693 0 0 1-.22.981l-1.663.866a.693.693 0 0 1-.935-.289l-2.31-4.387a.577.577 0 0 0-.866-.232L6.325 19.27a.692.692 0 0 1-1.155-.554V3.703a.693.693 0 0 1 1.074-.589"/></svg>)rawsvg";

gcePointerTool::gcePointerTool(gceEditorFrame &owner) : gceToolBase(owner)
{
    this->Bind(wxEVT_LEFT_DOWN, &gcePointerTool::OnLeftDown, this);

    this->Bind(wxEVT_MENU, &gcePointerTool::OnCmdDelete, this, cmdDELETE);
    this->Bind(wxEVT_MENU, &gcePointerTool::OnCmdEditPoints, this, cmdEDITP);
    this->Bind(wxEVT_MENU, &gcePointerTool::OnCmdEditMove, this, cmdMOVE);
    this->Bind(wxEVT_MENU, &gcePointerTool::OnCmdEditRotate, this, cmdROTATE);
    this->Bind(wxEVT_MENU, &gcePointerTool::OnCmdDifference, this, cmdDIFFERENCE);
    this->Bind(wxEVT_MENU, &gcePointerTool::OnCmdIntersection, this, cmdINTERSECTION);
    this->Bind(wxEVT_MENU, &gcePointerTool::OnCmdUnion, this, cmdUNION);
    this->Bind(wxEVT_MENU, &gcePointerTool::OnCmdSplit, this, cmdSPLIT);

    this->Bind(wxEVT_MENU, &gcePointerTool::OnCmdChangeLayer, this, cmdCHANGELAYER);

    this->Bind(wxEVT_MENU, &gcePointerTool::OnCmdEditDeselect, this, cmdDESEL);

    this->Bind(wxEVT_UPDATE_UI, &gcePointerTool::OnUpdateUIEvent, this, cmdFIRST, cmdLAST);

    uic(cmdDESEL, "Reset selection\tEsc").IconSVG(deselect_svg);
    uic(cmdEDITP, "Vertex editor\t`").IconSVG(vertexedit_svg);
    uic(cmdUNION, "Union").IconSVG(shapeunion_svg);
    uic(cmdSPLIT, "Split").Help("Split multi geometry").IconSVG(shapesplit_svg);
    uic(cmdINTERSECTION, "Intersection...").IconSVG(shapeintersection_svg);
    uic(cmdDIFFERENCE, "Difference...").IconSVG(shapesubtruct_svg);
    uic.separtor();
    uic(cmdMOVE, "Move/copy...").IconSVG(move_svg);
    uic(cmdROTATE, "Rotate/copy...").IconSVG(rotateshape_svg);
    uic(cmdCHANGELAYER, "Move to layer...").IconSVG(layeralt_svg);
    uic.separtor();
    uic(cmdDELETE, "Delete\tDel").Icon(wxART_DELETE);

    SetCursor({0, wxCursor(wxCURSOR_ARROW)});
}

gcePointerTool::~gcePointerTool() {}

gceToolInfo gcePointerTool::GetInfo() const
{
    return gceToolInfo{"Select", pointer_svg, "select feature"};
}

void gcePointerTool::BeginUse_Custom()
{
    m_owner.getSelection().selectionChanged.connect(this, &gcePointerTool::onSelectionChanged);
}

bool gcePointerTool::EndUse_Custom()
{
    if (EndUseChild())
    {
        m_owner.getSelection().selectionChanged.connect(this, &gcePointerTool::onSelectionChanged);
        return true;
    }
    return false;
}

void gcePointerTool::processSelectResult(const umodelSelectXDResultMsg &msg)
{
    //for (auto& sd : msg.data) {
    //	wxLogMessage(to_wxstring(fmt::format("{} {}", sd.id_entity, sd.radius)));
    //}
    //if (msg.data.empty()) {
    //	wxLogMessage("empty selection");
    //}

    this->m_querySent = false;

    if (msg.data.empty())
    {
        if (!this->m_shiftDown && !this->m_controlDown)
        {
            m_owner.getSelection().DeselectAll();
            m_owner.getSelection().signalSelection();
        }
    }
    else
    {
        auto &sel = m_owner.getSelection();
        if (auto it = std::find_if_not(msg.data.begin(), msg.data.end(), [&sel](const SelectedInfo &si){return sel.ContainsRec(si.key); }); it != msg.data.end())
        {
            udataSelectIDMsg dmsg;
            dmsg.key = it->key;
            dmsg.sender = gce::queueId::WORKSPACE;
            getCanvas()->ctx().postDataQueue(dmsg);
            m_querySent = true;
        }
        else
        {
            // do not requet entity data to deselect it
            if (m_controlDown)
            {
                if (sel.Deselect(msg.data.front().key, true) > 0)
                {
                    sel.signalSelection();
                }
            }
            else if (!m_shiftDown)
            {
                if (sel.DeselectExcept(msg.data.front().key))
                {
                    sel.signalSelection();
                }
            }
        }
    }
}

void gcePointerTool::processSelectDataResult(const udataSelectReplyMsg &msg)
{
    m_querySent = false;

    for (auto &row : msg.rows)
    {
        wxLogMessage(to_wxstring(row.to_string()));
    }
    if (msg.rows.empty())
    {
        wxLogMessage("msg.rows.empty()");
    }
    auto &selection = m_owner.getSelection();

    if (this->m_shiftDown)
    {
        for (auto &row : msg.rows)
        {
            selection.Select({msg.id_table, row});
        }
    }
    else if (this->m_controlDown)
    {
        for (auto &row : msg.rows)
        {
            selection.SelectXOR({msg.id_table, row});
        }
    }
    else
    {
        selection.DeselectAll();
        for (auto &row : msg.rows)
        {
            selection.Select({msg.id_table, row});
        }
    }
    selection.signalSelection();
}

void gcePointerTool::onSelectionChanged(const int)
{
    updateToolString();
}

void gcePointerTool::OnLeftDown(wxMouseEvent &event)
{
    if (!m_querySent)
    {
        auto &al = m_owner.getActiveLayer();

        umodelSelect2DMsg msg;
        msg.id_modelActive = al.id_model;
        msg.model_ids = this->m_owner.getModelsSelectable();
        msg.aoi = getCanvas()->calculateCursorAOI(10);
        msg.sender = gce::queueId::WORKSPACE;
        msg.limit = 5;
        getCanvas()->ctx().postModelQueue(std::move(msg));
        m_querySent = true;
        m_shiftDown = event.ShiftDown();
        m_controlDown = event.ControlDown();
    }

    event.Skip();
}

void gcePointerTool::OnCmdDelete(wxCommandEvent &WXUNUSED(event))
{
    auto &selection = this->m_owner.getSelection();
    if (selection.empty()) return;

    gceCommandGroup cmds("Delete");
    for (auto &ref : selection)
    {
        cmds.Delete(ref.id_table, ref.entity);
    }

    (void)this->m_owner.getActionProcessor().postCommandGroup(cmds);
}

void gcePointerTool::OnCmdEditPoints(wxCommandEvent &WXUNUSED(event))
{
    if (!this->m_owner.getSelection().empty())
    {
        startChild<gceVertexEditor>();
    }
}

void gcePointerTool::OnCmdEditMove(wxCommandEvent &WXUNUSED(event))
{
    if (!this->m_owner.getSelection().empty())
    {
        startChild<gceMoveTool>();
    }
}

void gcePointerTool::OnCmdDifference(wxCommandEvent &WXUNUSED(event))
{
    if (!this->m_owner.getSelection().empty())
    {
        startChild<DifferenceTool>();
    }
}

void gcePointerTool::OnCmdIntersection(wxCommandEvent &WXUNUSED(event))
{
    if (!this->m_owner.getSelection().empty())
    {
        startChild<gceIntersectionTool>();
    }
}

void gcePointerTool::OnCmdUnion(wxCommandEvent &WXUNUSED(event))
{
    auto &sel = m_owner.getSelection();
    auto &arr = sel.getCollection();
    if (arr.size() > 1 && sel.IsSameType())
    {
        auto first = arr.begin();
        auto last = arr.end();
        auto &ref = *first;

        auto united = ref.entity.get_geometry_GEOS();

        gceCommandGroup cmds("geometry union");
        while (++first != last && united)
        {
            auto B = first->entity.get_geometry();
            if (united = geom::alg_geos::GeomUnion(united, first->entity.get_geometry_GEOS()); united)
            {
                cmds.Delete(first->id_table, first->entity);
            }
        }

        if (first == last)
        {
            // union completed successfully
            gceEntityVar enUnion{ref.entity, united};
            cmds.Update(ref.id_table, gceEntityPacked{enUnion}, arr.begin()->entity);

            m_owner.getActionProcessor().postCommandGroup(cmds);
        }
    }
}

void gcePointerTool::OnCmdSplit(wxCommandEvent &WXUNUSED(event))
{
    gceCommandGroup cmds("Split multi geometries");
    for (auto &ref : m_owner.getSelection())
    {
        auto xgeom = ref.entity.get_geometry();
        if (auto *mgeom = xgeom->isMGeometry(); mgeom != nullptr)
        {
            auto *schema = ref.entity.get_schema();
            gceEntityVar entityTemplate{schema};
            for (auto &col : schema->getColumns())
            {
                if (!col.isPKEY() && !isGeometry(col.getType()))
                {
                    entityTemplate.assign(ref.entity, col.getIndex());
                }
            }

            while (mgeom->size() > 1)
            {
                entityTemplate.setGeometry(std::move(mgeom->collection.back()));
                mgeom->collection.pop_back();

                cmds.Insert(ref.id_table, gceEntityPacked{entityTemplate});
            }
            // update original entity with first geometry
            entityTemplate.assign(ref.entity, schema->getKeyIndex());
            entityTemplate.setGeometry(std::move(mgeom->collection.back()));
            mgeom->collection.pop_back();

            cmds.Update(ref.id_table, gceEntityPacked{entityTemplate}, ref.entity);
        }
    }
    if (!cmds.empty())
    {
        wxLogMessage("split to %d parts", (int)cmds.m_commands.size());
        m_owner.getActionProcessor().postCommandGroup(cmds);
    }
    else
    {
        wxLogMessage("nothing to split", (int)cmds.m_commands.size());
    }

}

void gcePointerTool::OnCmdEditRotate(wxCommandEvent &WXUNUSED(event))
{
    if (!m_owner.getSelection().empty())
    {
        startChild<RotateTool>();
    }
}

void gcePointerTool::OnCmdChangeLayer(wxCommandEvent &WXUNUSED(event))
{
    auto &sel = m_owner.getSelection();
    if (sel.empty())
    {
        wxLogError("No objects selected.");
        return;
    }

    if (!sel.IsSameType())
    {
        wxLogError("Objects of different types selected - unable to move to the same layer");
        return;
    }
    wxLogError("Not implemented");
}

void gcePointerTool::OnCmdEditDeselect(wxCommandEvent &)
{
    auto &selection = this->m_owner.getSelection();
    selection.DeselectAll();
    selection.signalSelection();
}

void gcePointerTool::OnUpdateUIEvent(wxUpdateUIEvent &event)
{
    auto &sel = this->m_owner.getSelection();
    switch (event.GetId())
    {
    case cmdDELETE:
    case cmdMOVE:
    case cmdROTATE:
    case cmdDIFFERENCE:
    case cmdDESEL:
    case cmdINTERSECTION:
    case cmdEDITP:
    case cmdCHANGELAYER:
    case cmdSPLIT:
        event.Enable(!sel.empty());
        break;
    case cmdUNION:
        event.Enable(sel.IsSameType() && sel.getCollection().size() > 1);
        break;
    }
}

bool gcePointerTool::DoRecalc(const wxPoint &)
{
    return false;
}

wxString gcePointerTool::GetString() const
{
    return getRecordsString();
}
