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

#include <stack>
#include <map>

#include "wx/aui/aui.h"

#include "type/entityset.h"
#include "actionproc.h"

class gceCanvas;
class gceDataTreeCtrl;
class gceWorkspace;
class gceContext;
class gceConfig;
class gceTypeSchema;
class gceToolBase;
class gceToolOptions;

struct EditorActiveLayerInfo
{
    gce::uuid id_layer{};
    gce::uuid id_table{};
    gce::uuid id_model{};
    gce::uuid id_scene{};
    const gceTypeSchema *schema = nullptr;
    std::string sceneName;
    std::string layerName;
};

/**
 *  Main editor window
 */
class gceEditorFrame final : public wxFrame
{
    friend class EditorCmds;
    friend class PersistentEditorFrame;
public:
    explicit gceEditorFrame(gceContext &ctx, const gceConfig &cfg);

    virtual ~gceEditorFrame();

    void OnToolRange(wxCommandEvent &event);
    void OnToolRangeUUI(wxUpdateUIEvent &event);

    void onTimer();

    wxWindow *GetToolOptParent();
    void PushToolOptions(gceToolOptions *tool_options);
    void PopToolOptions();
    void AttachTopToolOptions();
    gceToolBase *GetCurrentTool();

    void setToolString(const wxString &s);


    gceCanvas *getCanvas()
    {
        return m_canvas;
    }

    gceEntityPackedRef_set &getSelection()
    {
        return m_selection;
    }

    gceCmdProcessor &getActionProcessor()
    {
        return m_actionProc;
    }

    void ShowSelection();
    const EditorActiveLayerInfo &getActiveLayer() const
    {
        return m_activeLayer;
    }
    std::vector<gce::uuid> getModelsSelectable();
private:
    void processMsg(const gce::MessageInfo &msg);
    void processSetActiveLayer(const gce::MessageInfo &msg);
    void CreateGUIControls();
    std::stack<gceToolOptions *> m_toolOptStack;
    std::map<int, std::unique_ptr<gceToolBase> > m_tools;
    int m_currentToolId = -1;
    wxWindow *m_toolOptParent = nullptr;

    void createStatusBar();

    void UpdateStatus();

    void set_title();

    int GetCurrentToolId() const
    {
        return m_currentToolId;
    }

    void notifyOfActiveLayerChange();


    /*!
     * @brief select current tool and start using it, can be blocked by current tool
     * @param id tool id
     * @return true if tool is activated
     */
    bool SelectTool(int id);

    void OnFileOpenWorkspace(wxCommandEvent &event);
    void doOpenWorkspace(const wxString &fileName);

    void OnFileNewWorkspace(wxCommandEvent &event);

    void OnFileExport(wxCommandEvent &event);
    void OnFileImport(wxCommandEvent &event);

    void OnFileExportDB(wxCommandEvent &event);
    void OnFileImportDB(wxCommandEvent &event);
    void OnFileExit(wxCommandEvent &event);

    void OnViewModeFlat(wxCommandEvent &event);
    void OnViewModeGlobe(wxCommandEvent &event);
    void OnViewPane(wxCommandEvent &event);
    void OnViewPanesDefault(wxCommandEvent &event);

    void OnLogSave(wxCommandEvent &event);

    void OnLogClear(wxCommandEvent &event);

    void OnAbout(wxCommandEvent &event);
    void OnViewPaneUUI(wxUpdateUIEvent &event);

    void OnUtilsAddRaster(wxCommandEvent &event);
    void OnUtilsImportDEM(wxCommandEvent &event);
    void OnUtilsImportModel3D(wxCommandEvent &event);
    void OnUtilsExportModel3D(wxCommandEvent &event);

    bool CreatePIFrame(const wxAuiPaneInfo &pane);
    void OnSceneEnablesUI(wxUpdateUIEvent &evt);

    void OnClose(wxCloseEvent &event);

    void OnEditUndo(wxCommandEvent &event);
    void OnEditUndo_UUI(wxUpdateUIEvent &event);

    void OnEditRedo(wxCommandEvent &event);
    void OnEditRedo_UUI(wxUpdateUIEvent &event);
    void OnZoomToFullExtent(wxCommandEvent &);
    void OnZoomToSelected(wxCommandEvent &);
    void OnSceneRefresh(wxCommandEvent &);
    void OnShowGrid(wxCommandEvent &);
    void OnTerrainModeNormal(wxCommandEvent &);
    void OnTerrainModeGrid(wxCommandEvent &);

    void onCharHook(wxKeyEvent &event);

    template <bool read> void read_or_write();

    gceDataTreeCtrl *m_sceneTree = nullptr;
    gceDataTreeCtrl *m_modelsTree = nullptr;
    gceDataTreeCtrl *m_dataTree = nullptr;
    std::unique_ptr<gceWorkspace> m_workspace;


    bool CreateLogWindow(const wxAuiPaneInfo &pane);
    bool CreateToolPanel(const wxAuiPaneInfo &pane);

    gceEntityPackedRef_set m_selection;
    wxAuiManager m_AuiManager;
    wxString m_defaultPerspectiveAUI;
    wxString m_lastWorkspaceFile;

    double m_frameTime_ms = 1.0;
    wxTextCtrl *m_LogWindow;
    gceCanvas *m_canvas;
    std::unique_ptr<wxTimer> m_Timer;
    EditorActiveLayerInfo m_activeLayer;
    gceCmdProcessor m_actionProc;
    gceContext &m_ctx;
    const gceConfig &m_cfg;
};
