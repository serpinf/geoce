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

#include <tools/PlotModelBase.h>
#include "Canvas.h"
#include "geom/LineSegment.h"
#include "geom/CoordSeq.h"
#include "teches/techbasicrte2.h"
#include <GeographicLib/GeodesicLine.hpp>

bool PlotConstraint_Empty::Intersection(geom::Coordinate &dest, geom::Coordinate const &cpt, const geom::LineSegment &ls) const
{
    dest = ls.closestPoint(cpt);
    return true;
}

bool PlotConstraint_Empty::Intersection(geom::Coordinate &dest, const geom::Coordinate &ls) const
{
    dest = ls;
    return true;
}

geom::Coordinate PlotConstraint_Empty::Project(geom::Coordinate &nextCoord) const
{
    return nextCoord;
}
bool PlotConstraint_Point::Intersection(geom::Coordinate &dest, geom::Coordinate const & /*cpt*/, const geom::LineSegment &ls) const
{
    const geom::Coordinate cpt0 = geom::Coordinate(m_pos);
    dest = ls.closestPoint(cpt0);
    if (gce::equalsEpsilon(cpt0.pos, dest.pos, 1.0e-10))
    {
        return true;
    }
    return false;
}

bool PlotConstraint_Point::Intersection(geom::Coordinate &dest, const geom::Coordinate &ls) const
{
    if (gce::equalsEpsilon(m_pos, ls.pos, 1.0e-10))
    {
        dest = ls; //for topology reasons we need exact fit to point
        return true;
    }
    return false;
}

geom::Coordinate PlotConstraint_Point::Project(geom::Coordinate &) const
{
    return geom::Coordinate(m_pos);
}
bool PlotConstraint_Ray::Intersection(geom::Coordinate &dest, geom::Coordinate const & /*cpt*/, const geom::LineSegment &ls) const
{
    //geom::CoordinateXYZ tmp;
    //if (m_ray.intersection2d(tmp, ls) == 1)
    //{
    //    dest = geom::Coordinate(tmp);
    //    return true;
    //}
    /*else
    {
    }
    dest = ls.closestPoint(cpt);*/
    return false;
}

bool PlotConstraint_Ray::Intersection(geom::Coordinate &dest, const geom::Coordinate &ls) const
{
    //const glm::dvec3 prj = m_ray.project(ls.pos);
    //if (gce::equalsEpsilon(prj, ls.pos, 1.0e-10))
    //{
    //    dest = ls; //for topology reasons we need exect equality to stick point
    //    return true;
    //}
    return false;
}

geom::Coordinate PlotConstraint_Ray::Project(geom::Coordinate &nextCoord) const
{
//return nextCoord;
    double len = gce::earth::distance(glm::dvec2(nextCoord.pos), glm::dvec2(m_pos));
    auto pos = gce::earth::position(glm::dvec2(m_pos), m_azi, len);
    return geom::Coordinate({pos.x, pos.y, 0.0}, 0.0);
}

bool PlotConstraint_Circle::Intersection(geom::Coordinate &dest, geom::Coordinate const &cpt, const geom::LineSegment &ls) const
{
    geom::Coordinate crd[2];
    const int N = ls.intersectCircle(crd, glm::dvec2(m_circle.pos), m_circle.m);
    double min_dist = std::numeric_limits<double>::infinity();
    for (int n = 0; n < N; ++n)
    {
        double dist = glm::distance(crd[n].pos, cpt.pos);
        if (dist < min_dist)
        {
            dest = crd[n];
            min_dist = dist;
        }
    }
    return N > 0;
}

bool PlotConstraint_Circle::Intersection(geom::Coordinate &dest, const geom::Coordinate &ls) const
{
    // TODO: use distance2 - r^2
    if (std::fabs(glm::distance(m_circle.pos, ls.pos) - m_circle.m) < 2.0e-10)
    {
        dest = ls; //for topology reasons we need exact equality to stick point
        return true;
    }
    return false;
}

geom::Coordinate PlotConstraint_Circle::Project(geom::Coordinate &nextCoord) const
{
    auto pos = gce::earth::position(glm::dvec2(m_circle.pos), glm::dvec2(nextCoord.pos), m_circle.m);
    return geom::Coordinate({pos.x, pos.y, 0.0}, 0.0);
}

///////////////////////////////////////////////////////////////////////////////
//
//
gcePlotModelBase::gcePlotModelBase(gceCanvas *ctx) : m_canvas(ctx)
{}

gcePlotModelBase::~gcePlotModelBase()
{}

std::unique_ptr<PlotConstraint> gcePlotModelBase::CreateConstraint(const geom::CoordinateSeq &seq, const geom::Coordinate &next) const
{
    if (const size_t nPoints = seq.size(); nPoints > 0)
    {
        geom::CoordinateXY cBack;
        seq.getBack(cBack);
        std::optional<double> azi_base;
        double next_s12, next_azi1, next_azi2;
        gce::earth::getGeodesic().Inverse(cBack.pos.y, cBack.pos.x, next.pos.y, next.pos.x, next_s12, next_azi1, next_azi2);
        if (m_fAngleStep)
        {
            // find base ray for fixed angle calculation
            if (m_fBase)
            {
                azi_base = gce::round2_base(next_azi1, m_angle, m_base);
            }
            else if (nPoints > 1)
            {
                // from direction of last input segment
                geom::CoordinateXY cBack1;
                seq.get_rev(cBack1, 1);
                double prev_s12, prev_azi1, prev_azi2;
                gce::earth::getGeodesic().Inverse(cBack1.pos.y, cBack1.pos.x, cBack.pos.y, cBack.pos.x, prev_s12, prev_azi1, prev_azi2);
                azi_base = gce::round2_base(next_azi1, m_angle, prev_azi2);
            }
        }
        if (m_fLength)
        {
            double rounded_len = gce::round2(next_s12, m_lstep);
            if (azi_base)
            {
                auto pos = gce::earth::position(cBack.pos, *azi_base, rounded_len);
                return std::make_unique<PlotConstraint_Point>(glm::dvec3(pos, 0.0));
            }
            return std::make_unique<PlotConstraint_Circle>(glm::dvec3(cBack.pos, 0.0), rounded_len);
        }
        if (azi_base)
        {
            return std::make_unique<PlotConstraint_Ray>(glm::dvec3(cBack.pos, 0.0), *azi_base);
        }
    }
    return std::make_unique<PlotConstraint_Empty>();
}

void gcePlotModelBase::set_stick_mode(std::vector<geom::Coordinate> &seq, double mode)
{
    for (auto &coo : seq)
    {
        coo.m = mode;
    }
}

size_t gcePlotModelBase::AddPoint(geom::CoordinateSeq &seq, const geom::Coordinate &next)
{
    seq.push_back(next, false);
    return 1;
}


void gcePlotModelBase::postRenderTempLine1(const geom::CoordinateSeq &seq, const geom::Coordinate &next, bool haveMouse)
{
    if (haveMouse && !seq.empty())
    {
        geom::Coordinate coo;
        seq.getBack(coo);
        postRenderTempLine(TEMP_LINE_ID1, coo, next, false);
    }
    else
    {
        postRenderClearLine(TEMP_LINE_ID1);
    }
}

void gcePlotModelBase::postRenderTempLine2(const geom::CoordinateSeq &seq, const geom::Coordinate &next, bool haveMouse)
{
    if (haveMouse && seq.size() > 1)
    {
        geom::Coordinate coo;
        seq.getFront(coo);
        postRenderTempLine(TEMP_LINE_ID2, coo, next, true);
    }
    else
    {
        postRenderClearLine(TEMP_LINE_ID2);
    }
}
void gcePlotModelBase::postRenderClearLine(const uint32_t lineId)
{
    auto msg = std::make_shared<tech_basicrte2::DataMsg>();
    msg->id_model = TOOL_LAYER_UUID;
    auto &inst = msg->instances.emplace_back();
    inst.id_instance = lineId;
    inst.id_mesh = 0;
    m_canvas->ctx().postRenderQueue(std::static_pointer_cast<urenderTechDataMsg>(msg));
}

void gcePlotModelBase::postRenderTempLine(const uint32_t TEMP_LINE_ID, geom::Coordinate &coo, const geom::Coordinate &cursorCoord, bool useStipple)
{
    postRenderLine(TEMP_LINE_ID, {coo, cursorCoord}, 1.0, useStipple);
}

void gcePlotModelBase::postRenderLine(uint32_t lineId, const geom::CoordinateSeq &seq, float width, bool useStipple)
{
    auto *proj = m_canvas->getProj();
    if (proj == nullptr)
    {
        throw std::logic_error("projection can't be null");
    }

    if (seq.empty())
    {
        this->postRenderClearLine(lineId);
    }
    else
    {
        auto msg = std::make_shared<tech_basicrte2::DataMsg>();
        msg->id_model = TOOL_LAYER_UUID;

        auto &array = msg->arrays.emplace_back();
        array.id_array = lineId;
        array.vertexes.reserve(seq.size());
        uint32_t realSize = seq.size();
        for (size_t n = 0; n < seq.size(); n++)
        {
            geom::CoordinateXY coo;
            glm::dvec2 pos;
            seq.get(coo, n);
            if (n > 0)
            {
                geom::CoordinateXY prev;
                seq.get(prev, n - 1);
                auto line = gce::earth::getGeodesic().InverseLine(prev.pos.y, prev.pos.x, coo.pos.y, coo.pos.x);
                int num = int(ceil(line.Arc()));
                double da = line.Arc() / num;
                for (int i = 0; i < num; ++i)
                {
                    double lat, lon;
                    line.ArcPosition(i * da, lat, lon);
                    if (proj->fromInternal(pos, glm::dvec3(lon, lat, 0.0)))
                    {
                        array.vertexes.emplace_back(pos);
                        realSize++;
                    }
                }
            }

            if (proj->fromInternal(pos, coo.pos))
            {
                array.vertexes.emplace_back(pos);
            }
        }

        auto &mesh = msg->meshes.emplace_back();
        mesh.id_mesh = lineId;

        auto &dc = mesh.drawCalls.emplace_back();
        dc.id_array = lineId;
        dc.mode = gcePimitiveType::LINE_STRIP;
        dc.first = 0;
        dc.count = realSize;
        dc.invertColor = true;
        dc.useStipple = useStipple;
        dc.lineWidth = width;

        auto &inst = msg->instances.emplace_back();
        inst.id_instance = lineId;
        inst.id_mesh = lineId;
        //inst.modelMat = glm::translate(glm::identity<glm::dmat4>(), cen);

        m_canvas->ctx().postRenderQueue(std::static_pointer_cast<urenderTechDataMsg>(msg));
    }
}

bool gcePlotModelBase::setLength_User(const geom::CoordinateSeq &seq, wxWindow *parent, geom::Coordinate &nextCoord)
{
    if (seq.empty()) return false;

    geom::CoordinateXY cBack, cNext(nextCoord);
    seq.getBack(cBack);
    const glm::dvec2 nextVec(cNext.pos - cBack.pos);
    const double def_len = 1.0;//TODO: use proper distance to current mouse coord
    wxString str = ::wxGetTextFromUser("Enter segment length in meters", "Segment length", wxString::Format("%.3f", def_len), parent);
    if (!str.empty())
    {
        double d_val = wxAtof(str);
        // TODO: proper distance
        nextCoord = geom::Coordinate({nextVec.y == 0.0 ? glm::dvec2(d_val, 0) : glm::mix(cBack.pos, cNext.pos, d_val / def_len), 0.0});
        return true;
    }
    return false;
}
inline glm::dvec2 direction2(double a)
{
    return {cos(a), sin(a)};
}

wxString gcePlotModelBase::GetString(const geom::CoordinateSeq &seq, const geom::Coordinate &nextCoo) const
{
    if (seq.empty()) return {};

    geom::Coordinate cBack;
    seq.getBack(cBack);
    auto next = glm::dvec2(nextCoo.pos - cBack.pos);

    // calculate angle
    double angle;
    const wxChar *angle_name;
    if (m_fBase)
    {
        angle = gce::angle(next, direction2(glm::radians(m_base)));
        angle_name = wxT("angle to base");
    }
    else if (seq.size() >= 2)
    {
        geom::Coordinate c_rev1;
        seq.get_rev(c_rev1, 1);
        angle = gce::angle(next, glm::dvec2(cBack.pos - c_rev1.pos));
        angle_name = wxT("angle to prev");
    }
    else
    {
        angle = gce::angleX(next);
        angle_name = wxT("angle");
    }

    wxString lenStr = distanceHR(cBack, nextCoo);
    return wxString::Format("Segment: length = %s; %s = %.3f", lenStr, angle_name, glm::degrees(angle));
}

wxString gcePlotModelBase::distanceHR(const geom::Coordinate &A, const geom::Coordinate &B) const
{
    double distance_m = gce::earth::distance(glm::dvec2(A.pos), glm::dvec2(B.pos));
    return to_wxstring(gceProjection::formatLength(distance_m));
}
