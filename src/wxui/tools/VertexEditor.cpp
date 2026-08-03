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
#include "VertexEditor.h"
#include "Canvas.h"
#include "tools/ToolOptions.h"
#include "editorfrm.h"
#include "geom/Point.h"
#include "geom/LineString.h"
#include "geom/Polygon.h"
#include "geom/CoordSeq.h"
#include "type/entity.h"
#include "alg/alg_geos.h"

geom::Coordinate gceVertexEditorModel::DoRecalc(const geom::CoordinateSeq &seq, const wxPoint &mousePosition, std::vector<geom::Coordinate> &hint_seq)
{
    geom::Coordinate pt_coord; 	// Cursor coordinates

    m_canvas->CoordFromPoint(pt_coord.pos, mousePosition);

    std::unique_ptr<PlotConstraint> pc = CreateConstraint(seq, pt_coord);
    return pc->Project(pt_coord);
}

/* XPM */
static const char *vert_cur_xpm[] = {
/* columns rows colors chars-per-pixel */
    "16 16 5 1",
    "o c Black",
    "X c #FFFFFF",
    "O c #808080",
    "  c None",
    ". c #C0C0C0",
    /* pixels */
    "                ",
    ".Xoo            ",
    ".oXXooo         ",
    ".oXXXXXoo       ",
    " .oXXXXXXooo    ",
    " .oXXXXXXXXXoo  ",
    " .oXXXXXXXXXXXo ",
    "  .oXXXXXXXXXXo ",
    "  .oXXXXXXXXOO  ",
    "   .oXXXXXXO    ",
    "   .oXXXXXO     ",
    "   .oXXXXO      ",
    "    .oXXO       ",
    "    .oXXO       ",
    "     .oo        ",
    "      ..        "
};

gceVertexEditor::gceVertexEditor(gceEditorFrame &owner) : gceToolBase(owner), m_VEModel(getCanvas())
{
    this->Bind(wxEVT_LEFT_DOWN, &gceVertexEditor::OnLeftDown, this);
    this->Bind(wxEVT_RIGHT_UP, &gceVertexEditor::OnRightUp, this);
    this->Bind(wxEVT_MENU, &gceVertexEditor::OnCmdFinish, this, cmdFINISH);

    uic(cmdFINISH, "Finish\t`").IconSVG(finish_svg);

    // create cursor
    SetCursor(0, vert_cur_xpm, 1, 1);
}

gceToolInfo gceVertexEditor::GetInfo() const
{
    return gceToolInfo{"Vertex editor", nullptr, "use mouse Left to add or drag vertex, Right to remove"};
}

void gceVertexEditor::BeginUse_Custom()
{
    postSelectionModelProps(true, glm::dmat4{1.0});
}

bool gceVertexEditor::EndUse_Custom()
{
    resetEditor();
    postSelectionModelProps(false, glm::dmat4{1.0});
    Display();
    return true;
}

void gceVertexEditor::OnLeftDown(wxMouseEvent &event)
{
    if (this->cVertex)
    {
        // apply coordinates and reset editor
        gceEntityVar en(this->cVertex->ref.entity);
        if (auto *g = en.getGeometry(); g)
        {
            try
            {
                geom::Coordinate coo = this->currentCoo;
                //proj->toInternal(coo.pos, this->currentCoo.pos);
                if (cVertex->factor == 0.0)
                {
                    g->updateVertex(this->cVertex->index, coo);
                }
                else
                {
                    g->insertVertex(this->cVertex->index, coo);
                }

                this->updateGeometry(this->cVertex->ref, en, fmt::format("edit vertex{}", this->cVertex->index));

                this->resetEditor();
            }
            catch (std::exception &e)
            {
                wxLogMessage(e.what());
            }
        }

    }
    else
    {
        auto *proj = this->getCanvas()->getProj();
        // select existing vertex or create new on a segment
        auto aoi = getCanvas()->calculateCursorAOI(10);

        auto &selection = this->m_owner.getSelection();
        auto res = selection.search_vertex2D(glm::dvec2(aoi), aoi.z, proj);
        if (!res)
        {
            res = selection.search_segment2D(glm::dvec2(aoi), aoi.z, proj);
        }
        if (res)
        {
            cVertex = std::move(res);
            updateNeighborCoords();
        }
    }

    event.Skip();
}
namespace
{

inline size_t get_next_id(size_t idx, size_t count)
{
    return idx < count - 1 ? idx + 1 : 0;
}

inline size_t get_next_id(size_t *idx, size_t count)
{
    return *idx = get_next_id(*idx, count);
}

inline size_t get_prev_id(size_t idx, size_t count)
{
    return idx > 0 ? idx - 1 : count - 1;
}

}

void gceVertexEditor::updateNeighborCoords()
{
    if (this->cVertex)
    {
        auto [idx, xgeom] = cVertex->model->getGeometryForVertex(cVertex->index);
        if (xgeom != nullptr)
        {
            if (auto *poly = xgeom->isPolygon(); poly != nullptr)
            {
                if (auto [idx_seq, cSeq] = poly->getRingForVertex(idx); cSeq != nullptr)
                {
                    size_t nPts = cSeq->size();
                    geom::Coordinate c;
                    if (cVertex->factor == 0.0)
                    {
                        bool closed = cSeq->isClosed() && nPts > 2;
                        if (closed)
                        {
                            nPts--;
                        }

                        cSeq->get(c, get_prev_id(idx_seq, nPts));
                        this->prevCoo = c;

                        cSeq->get(c, get_next_id(idx_seq, nPts));
                        this->nextCoo = c;

                        cSeq->get(c, idx_seq);
                        this->currentCoo = c;
                    }
                    else // factor!=0.0
                    {
                        cSeq->get(c, idx_seq - 1);
                        this->prevCoo = c;

                        cSeq->get(c, idx_seq);
                        this->nextCoo = c;

                        this->currentCoo = geom::mix(*prevCoo, *nextCoo, cVertex->factor);
                    }
                }
            }
            else if (auto *line = xgeom->isLineString(); line != nullptr)
            {
                auto &cSeq = line->getCoordSeq();
                size_t nPts = cSeq.size();
                geom::Coordinate c;
                if (cVertex->factor == 0.0)
                {
                    if (idx > 0)
                    {
                        cSeq.get(c, idx - 1);
                        this->prevCoo = c;
                    }

                    if (idx < nPts - 1)
                    {
                        cSeq.get(c, idx + 1);
                        this->nextCoo = c;
                    }
                    cSeq.get(c, idx);
                    this->currentCoo = c;
                }
                else
                {
                    cSeq.get(c, idx - 1);
                    this->prevCoo = c;

                    cSeq.get(c, idx);
                    this->nextCoo = c;

                    this->currentCoo = geom::mix(*prevCoo, *nextCoo, cVertex->factor);
                }
            }
            else if (auto *point = xgeom->isPoint(); point != nullptr)
            {
                this->prevCoo = point->getCoordinate();
                this->currentCoo = point->getCoordinate();
            }
            if (m_fReverse)
            {
                std::swap(prevCoo, nextCoo);
            }
        }
    }
    else
    {
        this->prevCoo = {};
        this->nextCoo = {};
    }
}

void gceVertexEditor::updateGeometry(const gceEntityPackedRef &ref, gceEntityVar &newRec, const std::string &actionName)
{
    if (ref.isTableRow())
    {
        newRec.setGeometry(geom::alg_geos::MakeValid(*newRec.getGeometry()));

        gceCommandGroup cmds(actionName);
        cmds.Update(ref.id_table, gceEntityPacked{newRec}, ref.entity);
        (void)this->m_owner.getActionProcessor().postCommandGroup(cmds);
    }
    else
    {
        auto &selection = this->m_owner.getSelection();
        selection.Replace({ref.id_table, gceEntityPacked{newRec}}, true);
        //selection.signalSelection();
    }
}

void gceVertexEditor::OnRightUp(wxMouseEvent &event)
{
    if (cVertex)
    {
        resetEditor();
    }
    else
    {
        auto *proj = this->getCanvas()->getProj();

        glm::dvec3 aoi = getCanvas()->calculateCursorAOI(10);

        auto &selection = this->m_owner.getSelection();
        if (auto res = selection.search_vertex2D(glm::dvec2(aoi), aoi.z, proj); res)
        {
            //gceContext::log_message("remove point {}", res->index);
            gceEntityVar en{res->ref.entity};
            if (auto *g = en.getGeometry(); g)
            {
                try
                {
                    g->removeVertex(res->index);
                    this->updateGeometry(res->ref, en, fmt::format("remove point{}", res->index));
                }
                catch (std::exception &e)
                {
                    wxLogMessage(e.what());
                }
            }
        }
        else
        {
            event.Skip();
        }
    }
}

void gceVertexEditor::Display()
{
    if (this->HaveMouse() && this->cVertex)
    {
        if (this->prevCoo)
        {
            m_VEModel.postRenderTempLine(TEMP_LINE_ID1, this->currentCoo, *this->prevCoo, false);
        }

        if (this->nextCoo)
        {
            m_VEModel.postRenderTempLine(TEMP_LINE_ID2, this->currentCoo, *this->nextCoo, true);
        }
    }
    else
    {
        m_VEModel.postRenderClearLine(TEMP_LINE_ID1);
        m_VEModel.postRenderClearLine(TEMP_LINE_ID2);
    }
}

void gceVertexEditor::processActionNotify(const udataMultiRowActionNotifyMsg &msg)
{
    if (cVertex)
    {
        for (auto &action : msg.m_actions)
        {
            if ((action.query == gceActionType::Delete || action.query == gceActionType::Update) &&
                action.id_table == cVertex->ref.id_table && action.oldEntity.get_pkey() == cVertex->ref.entity.get_pkey())
            {
                this->resetEditor();
            }
        }
    }
}

void gceVertexEditor::OnCmdFinish(wxCommandEvent &)
{
    EndUse();
}

bool gceVertexEditor::DoRecalc(const wxPoint &mousePosition)
{
    using namespace geom;
    if (this->cVertex)
    {
        auto [idx, xgeom] = cVertex->model->getGeometryForVertex(cVertex->index);
        if (xgeom != nullptr)
        {
            CoordinateSeq seq{CoordinateType::XYZM};
            if (auto *poly = xgeom->isPolygon(); poly != nullptr)
            {
                if (auto [idx_seq, cSeq] = poly->getRingForVertex(idx); cSeq != nullptr)
                {
                    size_t nPts = cSeq->size();
                    bool closed = cSeq->isClosed() && nPts > 2;
                    if (closed)
                    {
                        nPts--;
                    }
                    idx_seq %= nPts;

                    seq.reserve(nPts);
                    geom::Coordinate c;
                    size_t idx_it = cVertex->factor == 0.0 ? get_next_id(idx_seq, nPts) : idx_seq;
                    do
                    {
                        cSeq->get(c, idx_it);
                        seq.push_back(c);
                    }
                    while (get_next_id(&idx_it, nPts) != idx_seq);

                    if (m_fReverse)
                    {
                        seq.reverse();
                    }
                }
            }
            else if (auto *line = xgeom->isLineString(); line != nullptr)
            {
                auto &cSeq = line->getCoordSeq();
                size_t nPts = cSeq.size();
                geom::Coordinate c;
                seq.reserve(nPts);
                if (m_fReverse)
                {
                    for (size_t i = nPts - 1; i > idx; i--)
                    {
                        cSeq.get(c, i);
                        seq.push_back(c);
                    }
                    if (cVertex->factor != 0.0)
                    {
                        cSeq.get(c, idx);
                        seq.push_back(c);
                    }
                }
                else
                {
                    for (size_t i = 0; i < idx; i++)
                    {
                        cSeq.get(c, i);
                        seq.push_back(c);
                    }
                }
            }
            else if (auto *point = xgeom->isPoint(); point != nullptr)
            {
                seq.push_back(point->getCoordinate());
            }

            std::vector<geom::Coordinate> hint_seq;
            this->currentCoo = m_VEModel.DoRecalc(seq, mousePosition, hint_seq);
        }
    }
    return true;
}

gceToolOptions *gceVertexEditor::create_to(wxWindow *parent)
{
    // set options
    auto to = new gceToolOptions(parent);

    to->AddNamedFlag(m_VEModel.m_fAngleStep, "Angle step:");
    to->AddEditableFloatList(m_VEModel.m_angle, {15.0, 30.0, 45.0, 90.0});

    to->AddSeparator();

    to->AddNamedFlag(m_VEModel.m_fBase, "Base line:");
    to->AddEditableFloatList(m_VEModel.m_base, {0.0});

    to->AddSeparator();

    to->AddNamedFlag(m_VEModel.m_fLength, "Length step:");
    to->AddEditableFloatList(m_VEModel.m_lstep, {0.001, 0.01, 0.1, 1.0});

    to->AddNamedFlag(m_fReverse, "Invert");

    to->Realize();
    return to;
}

void gceVertexEditor::resetEditor()
{
    this->cVertex = {};
    this->prevCoo = {};
    this->nextCoo = {};
}
