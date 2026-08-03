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
#pragma once

#include "tools/Tool.h"
#include "type/entityset.h"
#include "tools/PlotModelBase.h"

class gceVertexEditorModel : public gcePlotModelBase
{
public:
    explicit gceVertexEditorModel(gceCanvas *ctx) : gcePlotModelBase(ctx)
    {}

    geom::Coordinate DoRecalc(const geom::CoordinateSeq &seq, const wxPoint &mousePosition, std::vector<geom::Coordinate> &hint_seq) override;
};

class gceVertexEditor final : public gceToolBase
{
public:
    explicit gceVertexEditor(gceEditorFrame &owner);

    gceToolInfo GetInfo() const override;

    void resetEditor();

    void BeginUse_Custom() override;

    bool EndUse_Custom() override;

    void Display() override;

    void processActionNotify(const udataMultiRowActionNotifyMsg &msg) override;
private:
    gceToolOptions *create_to(wxWindow *parent) override;

    bool DoRecalc(const wxPoint &mousePosition) override;
    enum POPUP_PT
    {
        cmdFIRST,
        cmdFINISH,
        cmdLAST,
    };
    void updateNeighborCoords();
    void updateGeometry(const gceEntityPackedRef &ref, gceEntityVar &newRec, const std::string &actionName);

    void OnLeftDown(wxMouseEvent &event);

    void OnRightUp(wxMouseEvent &event);

    void OnCmdFinish(wxCommandEvent &event);

    gceVertexEditorModel m_VEModel;
    bool m_fReverse = false;

    std::optional<gceSearchResultVertex> cVertex;
    std::optional<geom::Coordinate> prevCoo;
    std::optional<geom::Coordinate> nextCoo;
    geom::Coordinate currentCoo;
};

