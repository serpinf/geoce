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

#include "CircleTool.h"
#include <geom/LineSegment.h>
#include "editorfrm.h"
#include "tools/ToolOptions.h"

/* XPM */
static const char *Input_circ_xpm[] = {
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
    "     +++++++++++    ......      ",
    "                  ..+++++..     ",
    "                  .+@@@@@+.     ",
    "                 .+@@@@@@@+.    ",
    "                 .+@@@@@@@+.    ",
    "                 .+@@@@@@@+.    ",
    "                 .+@@@@@@@+.    ",
    "                 .+@@@@@@@+.    ",
    "                  .+@@@@@+.     ",
    "                  ..+++++..     ",
    "                    .....       ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                "};

/* XPM */
static const char *Input_round_xpm[] = {
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
    "     +.........+                ",
    "     +++++++++++    ......      ",
    "                  ..+++++..     ",
    "                  .+     +.     ",
    "                 .+       +.    ",
    "                 .+       +.    ",
    "                 .+       +.    ",
    "                 .+       +.    ",
    "                 .+       +.    ",
    "                  .+     +.     ",
    "                  ..+++++..     ",
    "                    .....       ",
    "                                ",
    "                                ",
    "                                ",
    "                                ",
    "                                "};

static constexpr char circle_contour_svg[] = R"rawsvg(<symbol viewBox="0 0 2048 2048" id="circle-shape"><title>circle-shape</title><path fill="currentColor" d="M1024 0q141 0 272 36t244 104t207 160t161 207t103 245t37 272q0 141-36 272t-104 244t-160 207t-207 161t-245 103t-272 37q-141 0-272-36t-244-104t-207-160t-161-207t-103-245t-37-272q0-141 36-272t104-244t160-207t207-161T752 37t272-37m0 1920q124 0 238-32t214-90t181-140t140-181t91-214t32-239t-32-238t-90-214t-140-181t-181-140t-214-91t-239-32t-238 32t-214 90t-181 140t-140 181t-91 214t-32 239t32 238t90 214t140 181t181 140t214 91t239 32"></path></symbol>)rawsvg";
static constexpr char circle_solid_svg[] = R"rawsvg(<symbol viewBox="0 0 2048 2048" id="circle-shape-solid"><title>circle-shape-solid</title><path fill="currentColor" d="M1024 0q141 0 272 36t244 104t207 160t161 207t103 245t37 272q0 141-36 272t-104 244t-160 207t-207 161t-245 103t-272 37q-141 0-272-36t-244-104t-207-160t-161-207t-103-245t-37-272q0-141 36-272t104-244t160-207t207-161T752 37t272-37"></path></symbol>)rawsvg";

CircleTool::CircleTool(gceEditorFrame &owner, int dimension) : gceInputToolBase(owner, dimension),
m_PlotModel(getCanvas())
{

    this->Bind(wxEVT_LEFT_DOWN, &CircleTool::OnLeftDown, this);

    // popup handling
    this->Bind(wxEVT_MENU, &CircleTool::OnCmdEnter, this, cmdENTER);
    this->Bind(wxEVT_MENU, &CircleTool::OnCmdCancel, this, cmdCANCEL);
    this->Bind(wxEVT_MENU, &CircleTool::OnCmdBaseLine, this, cmdBASE_LINE);
    this->Bind(wxEVT_MENU, &CircleTool::OnCmdStepBack, this, cmdSTEP_BACK);
    this->Bind(wxEVT_MENU, &CircleTool::OnCmdSetLength, this, cmdLENGTH);
    this->Bind(wxEVT_UPDATE_UI, &CircleTool::OnUpdateUIEvent, this, cmdFIRST, cmdLAST);

    // init Plot Model
    m_ToolType = 0;
    m_Precision = 32;

    uic(cmdENTER, "Enter\tEnter").IconSVG(enter_svg);
    uic(cmdSTEP_BACK, "Step back\tZ").IconSVG(step_back_svg);
    uic(cmdCANCEL, "Cancel").IconSVG(cancel_svg);
    uic.separtor();
    //uic(cmdBASE_LINE, "Base line\tCtrl+G").Icon(base_angle_svg);
}

void CircleTool::BeginUse_Custom()
{
    ConnectLayer();
}


void CircleTool::OnLeftDown(wxMouseEvent &event)
{
    TryEnterOrAddPoint();

    event.Skip();
}

void CircleTool::OnCmdSetLength(wxCommandEvent &)
{
    if (m_PlotModel.setLength_User(m_ring, &m_owner, m_NextCoord))
    {
        saveGeometry();
        TryEnterOrAddPoint();
    }
}

void CircleTool::Display()
{
    geom::CoordinateSeq dest{geom::CoordinateType::XY};
    dest.reserve(m_ring.size() + 1);
    dest.assign(m_ring);
    dest.push_back(m_NextCoord);

    m_PlotModel.postRenderLine(TEMP_LINE_ID1, dest, 1.0, true);

    geom::CoordinateSeq ring{geom::CoordinateType::XYZ};
    makeCircle(ring);
    m_PlotModel.postRenderLine(TEMP_LINE_ID2, ring, 1.0, false);
}


void CircleTool::OnCmdEnter(wxCommandEvent &)
{
    DoEnter();
}


void CircleTool::OnCmdCancel(wxCommandEvent &)
{
    m_ring.clear();
    saveGeometry();
}


void CircleTool::OnUpdateUIEvent(wxUpdateUIEvent &event)
{
    switch (event.GetId())
    {
    case cmdENTER:
        event.Enable(CanInsert());
        break;
    case cmdCANCEL:
        event.Enable(!m_ring.empty());
        break;
    case cmdBASE_LINE:
        event.Enable(m_ring.size() <= 1);
        break;
    case cmdSTEP_BACK:
        event.Enable(!m_ring.empty());
        break;
    }
}


void CircleTool::OnCmdStepBack(wxCommandEvent &)
{
    if (!m_ring.empty())
    {
        m_ring.pop_back();
        saveGeometry();
    }
}


void CircleTool::OnCmdBaseLine(wxCommandEvent &)
{
    //callTool("BaseLineTool");
}


bool CircleTool::DoRecalc(const wxPoint &mousePosition)
{
    std::vector<geom::Coordinate> hint_seq;
    m_NextCoord = m_PlotModel.DoRecalc(m_ring, mousePosition, hint_seq);
    saveGeometry();
    return true;
}


bool CircleTool::makeCircle(geom::CoordinateSeq &dest) const
{
    // TODO: refactor and check: use vector of coordinates, sorted by angle
    typedef std::map<double, int> angles_map;

    if (m_ring.empty())
    {
        return true;
    }
    // generate geometry
    dest.clear();

    long precision = m_Precision;

    double r;
    geom::CoordinateXY c, center;
    geom::CoordinateXY crd0, crd1, crd2;
    double ang;
    switch (m_ToolType)
    {
    case circle_edge_to_center:
        m_ring.get(crd1, 0);
        crd0 = m_NextCoord;
        ang = gce::angleX(crd1.pos - crd0.pos);
        r = geom::distance2d(crd0, crd1);
        for (int i = 0; i <= precision; i++)
        {
            c.pos[0] = crd0.pos[0] + r * cos(i * M_PI * 2.0 / precision + ang);
            c.pos[1] = crd0.pos[1] + r * sin(i * M_PI * 2.0 / precision + ang);
            dest.push_back(c);
        };

        break;
    case circle_center_to_edge:
        m_ring.get(crd0, 0);
        crd1 = m_NextCoord;
        ang = gce::angleX(crd1.pos - crd0.pos);
        r = geom::distance2d(crd0, crd1);
        for (int i = 0; i <= precision; i++)
        {
            c.pos[0] = crd0.pos[0] + r * cos(i * M_PI * 2.0 / precision + ang);
            c.pos[1] = crd0.pos[1] + r * sin(i * M_PI * 2.0 / precision + ang);
            dest.push_back(c);
        };

        break;
    case circle_2poins_diameter:
        m_ring.get(crd0, 0);
        crd1 = m_NextCoord;
        ang = gce::angleX(crd1.pos - crd0.pos);
        center.pos[0] = (crd1.pos[0] + crd0.pos[0]) / 2;
        center.pos[1] = (crd1.pos[1] + crd0.pos[1]) / 2;
        r = geom::distance2d(crd0, crd1) / 2;
        for (int i = 0; i <= precision; i++)
        {
            if ((i < (precision / 2)) && (i > (1 + precision / 2)))
            {
                // add opposite point
                dest.push_back(c);
            }
            c.pos[0] = center.pos[0] + r * cos(i * M_PI * 2.0 / precision + ang);
            c.pos[1] = center.pos[1] + r * sin(i * M_PI * 2.0 / precision + ang);
            dest.push_back(c);
        };

        break;
    default: // circle_3points
        if (m_ring.size() >= 2)
        {
            m_ring.get(crd0, 0);
            m_ring.get(crd1, 1);
            crd2 = m_NextCoord;
            if (!gce::pointsOnLine(crd0.pos, crd1.pos, crd2.pos))
            {
                geom::LineSegmentXY ls_0(crd1, crd0);
                geom::LineSegmentXY ls_1(crd0, crd2);
                geom::LineSegmentXY ls_perp_0 = ls_0.perpSegment();
                ls_perp_0 = ls_perp_0.translateSegment(ls_0.lerp(0.5));
                geom::LineSegmentXY ls_perp_1 = ls_1.perpSegment();
                ls_perp_1 = ls_perp_1.translateSegment(ls_1.lerp(0.5));

                if (ls_perp_0.intersection2d(center.pos, ls_perp_1))
                {
                    angles_map Amap;
                    // prepare poin angles
                    for (int i = 0; i <= precision; i++)
                    {
                        Amap.emplace(M_PI * 2 * i / precision - M_PI, 0);
                    }
                    Amap.emplace(gce::angleX(crd0.pos - center.pos), 0);		// angle of 1s predefined point
                    Amap.emplace(gce::angleX(crd1.pos - center.pos), 0);		// angle of 2nd predefined point
                    Amap.emplace(gce::angleX(crd2.pos - center.pos), 0);		// angle of 3rd predefined point

                    // generate circle
                    r = geom::distance2d(crd0, center);
                    for (auto &_val : Amap)
                    {
                        double Ang = _val.first;
                        c.pos[0] = center.pos[0] + r * cos(Ang);
                        c.pos[1] = center.pos[1] + r * sin(Ang);
                        dest.push_back(c);
                    }
                }
            }
        }
    }
    dest.EnsureRing();

    return true;
}


wxString CircleTool::GetString() const
{
    wxString result;
    if (!m_ring.empty())
    {
        geom::Coordinate crd0, crd1;
        m_ring.get(crd0, 0);
        switch (m_ToolType)
        {
        case circle_edge_to_center:
        case circle_center_to_edge:
            result << "Radius = " << m_PlotModel.distanceHR(crd0, m_NextCoord);
            break;
        case circle_2poins_diameter:
            result << "Diameter = " << m_PlotModel.distanceHR(crd0, m_NextCoord);
            break;
        default:	// circle_3points
            if (m_ring.size() > 1)
            {
                m_ring.get(crd1, 1);
                result << "Chord-1 = " << m_PlotModel.distanceHR(crd0, crd1);
                result << "; Chord-2 = " << m_PlotModel.distanceHR(crd1, m_NextCoord);
            }
            else
            {
                result << " Chord-1 = " << m_PlotModel.distanceHR(crd0, m_NextCoord);
            }
        }
    }
    return result;
}

gceToolOptions *CircleTool::create_to(wxWindow *parent)
{
    // set options
    auto to = new gceToolOptions(parent);

    to->AddEnum(m_ToolType, {
        {"3 edge points", circle_3points},
                {"Edge to center", circle_edge_to_center},
                {"Center to edge", circle_center_to_edge},
                {"2 edge points", circle_2poins_diameter}}, wxSize(170, wxDefaultCoord));

    to->AddNamedFlag(m_PlotModel.m_fAngleStep, "Angle step");
    to->AddEditableFloatList(m_PlotModel.m_angle, {15.0, 30.0, 45.0, 90.0});

    to->AddSeparator();

    to->AddNamedFlag(m_PlotModel.m_fBase, "Base azimuth");
    to->AddEditableFloatList(m_PlotModel.m_base, {0.0});

    to->AddSeparator();

    to->AddNamedFlag(m_PlotModel.m_fLength, "Length step");
    to->AddEditableFloatList(m_PlotModel.m_lstep, {0.001, 0.01, 0.1, 1.0});

    to->AddLabel("Quadrant points");
    to->AddSpin(m_Precision, 2, 99);

    to->Realize();
    return to;
}

void CircleTool::saveGeometry() {}

std::unique_ptr<geom::Geometry> CircleTool::createInputGeometry(geom::CoordinateType cooType) const
{
    geom::CoordinateSeq dest{cooType};
    makeCircle(dest);
    return geom::Geometry::Create(m_dimension, cooType, dest);
}

bool CircleTool::canInsertGeometry() const
{
    if (m_ToolType == 0)
    {
        return m_ring.size() == 2;
    }
    return !m_ring.empty();
}

void CircleTool::TryEnterOrAddPoint()
{
    if (!DoEnter())
    {
        switch (m_ToolType)
        {
        case 0:
            if (m_ring.size() < 2)
            {
                m_ring.push_back(m_NextCoord);
            }
            break;
        case 1:
        case 2:
        case 3:
            if (m_ring.empty())
            {
                m_ring.push_back(m_NextCoord);
            }
        }
    }
}

CircleLineTool::CircleLineTool(gceEditorFrame &owner) : CircleTool(owner, 1)
{
    SetCursor(0, Input_round_xpm, 11, 0);
}

gceToolInfo CircleLineTool::GetInfo() const
{
    return gceToolInfo{"Circle tool", circle_contour_svg, ""};
}

CirclePolyTool::CirclePolyTool(gceEditorFrame &owner) : CircleTool(owner, 2)
{
    SetCursor(0, Input_circ_xpm, 11, 0);
}


gceToolInfo CirclePolyTool::GetInfo() const
{
    return gceToolInfo{"Circle poly tool", circle_solid_svg, ""};
}

