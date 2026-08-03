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
#include <wx/dataview.h>
#include "engine.hpp"

class WSTreeModel;
class gceWorkspace;

class gceDataTreeCtrl final : public wxDataViewCtrl
{
public:
    enum NodeType
    {
        SCENE,
        MODELS,
        DATASRC
    };

    gceDataTreeCtrl(wxWindow *parent, gceWorkspace &wsp, NodeType nodeType);

    void OnContextMenu(wxDataViewEvent &event);
    void OnItemActivated(wxDataViewEvent &event);
    void onLoadFile();
    void processMsg(const gce::MessageInfo &msg);
private:
    wxObjectDataPtr<WSTreeModel> m_dataModel;
};
