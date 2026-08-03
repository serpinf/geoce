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

#include "PlotModel.h"
#include "Canvas.h"
#include "geom/CoordSeq.h"

geom::Coordinate gcePlotModel::DoRecalc(const geom::CoordinateSeq &seq, const wxPoint &mousePosition, std::vector<geom::Coordinate> &hint_seq)
{
    std::vector<geom::Coordinate> hot_stick_seq;
    geom::Coordinate pt_coord;
    m_canvas->CoordFromPoint(pt_coord.pos, mousePosition);

    std::unique_ptr<PlotConstraint> pc = CreateConstraint(seq, pt_coord);
    geom::Coordinate coord2 = pc->Project(pt_coord);
    return coord2;
}

