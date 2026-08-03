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

#include "tools/tool.h"
#include "sigslot.h"

class gcePointerTool final : public gceToolBase, public sigslot::has_slots
{
public:
    explicit gcePointerTool(gceEditorFrame &owner);
    ~gcePointerTool();

    gceToolInfo GetInfo() const override;

    wxString GetString() const override;

    void BeginUse_Custom() override;

    bool EndUse_Custom() override;

    void processSelectResult(const umodelSelectXDResultMsg &msg) override;
    void processSelectDataResult(const udataSelectReplyMsg &msg) override;

    /*!
     * @brief selection changes slot
     */
    void onSelectionChanged(const int);
private:

    enum POPUP_PT
    {
        cmdFIRST = 100,
        cmdDELETE,
        cmdEDITP,
        cmdMOVE,
        cmdROTATE,
        cmdDIFFERENCE,
        cmdINTERSECTION,
        cmdCHANGELAYER,
        cmdUNION,
        cmdSPLIT,

        cmdDESEL,

        cmdLAST,
    };

    // window events
    void OnLeftDown(wxMouseEvent &event);
    void OnCmdDelete(wxCommandEvent &event);
    void OnCmdEditPoints(wxCommandEvent &event);
    void OnCmdEditMove(wxCommandEvent &event);
    void OnCmdEditRotate(wxCommandEvent &event);
    void OnCmdChangeLayer(wxCommandEvent &event);
    void OnCmdDifference(wxCommandEvent &event);
    void OnCmdIntersection(wxCommandEvent &event);
    void OnCmdUnion(wxCommandEvent &event);
    void OnCmdSplit(wxCommandEvent &event);
    void OnCmdEditDeselect(wxCommandEvent &event);

    void OnUpdateUIEvent(wxUpdateUIEvent &event);

    bool DoRecalc(const wxPoint &mousePosition) override;

    // service messages state
    bool m_querySent = false;
    bool m_shiftDown = false;
    bool m_controlDown = false;
};

