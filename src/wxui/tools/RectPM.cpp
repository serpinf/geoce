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

#include "RectPM.h"
#include "Canvas.h"

/**
 * orientaion of new segment is done using a given base line (fbase=1)
 * or the last segment (fbase=0), angle is always 90 degrees
 *
 * @param mousePosition
 * @param hint_seq
 *
 * @return
 */
geom::Coordinate gceRectPM::DoRecalc(const geom::CoordinateSeq &seq, const wxPoint &mousePosition, std::vector<geom::Coordinate> &hint_seq)
{
    geom::Coordinate pt_coord;

    m_fAngleStep = true;
    m_angle = 90.0;

    m_canvas->CoordFromPoint(pt_coord.pos, mousePosition);

    std::unique_ptr<PlotConstraint> pc = CreateConstraint(seq, pt_coord);
    return pc->Project(pt_coord);
}
std::unique_ptr<PlotConstraint> gceRectPM::CreateConstraint(const geom::CoordinateSeq &seq, const geom::Coordinate &nextCoord) const
{
    if (const size_t nPoints = seq.size(); nPoints > 0)
    {
        geom::CoordinateXY cBack;
        seq.getBack(cBack);
        std::optional<double> azi_base;
        double next_s12, next_azi1, next_azi2;
        gce::earth::getGeodesic().Inverse(cBack.pos.y, cBack.pos.x, nextCoord.pos.y, nextCoord.pos.x, next_s12, next_azi1, next_azi2);

        if (nPoints > 1)
        {
            // from direction of last input segment
            geom::CoordinateXY cBack1;
            seq.get_rev(cBack1, 1);
            double prev_s12, prev_azi1, prev_azi2;
            gce::earth::getGeodesic().Inverse(cBack1.pos.y, cBack1.pos.x, cBack.pos.y, cBack.pos.x, prev_s12, prev_azi1, prev_azi2);

            azi_base = gce::round2_base(next_azi1, 180.0, prev_azi2 + 90.0);
        }
        else if (m_fBase)
        {
            azi_base = gce::round2_base(next_azi1, m_angle, m_base);
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


