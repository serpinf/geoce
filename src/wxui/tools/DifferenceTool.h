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

class DifferenceTool final : public gceToolBase
{
public:
    explicit DifferenceTool(gceEditorFrame &owner);

    virtual wxString GetString() const override;

    virtual void BeginUse_Custom() override;

    virtual bool EndUse_Custom() override;
protected:

    void OnLeftDown(wxMouseEvent &event);

    void processSelectResult(const umodelSelectXDResultMsg &msg) final;

    void processSelectDataResult(const udataSelectReplyMsg &msg) final;

    enum POPUP_PT
    {
        cmdFIRST,
        cmdFINISH,
        cmdLAST,
    };
    void OnCmdFinish(wxCommandEvent &event);
};

