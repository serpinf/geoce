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

#include "editorfrm.h"

#include "Canvas.h"

#include "DataTree.h"
#include "tools/ToolOptions.h"

#include <wx/aboutdlg.h>
#include <wx/file.h>
#include "EditorCmds.h"

#include "PIFrame.h"
#include <wx/persist/toplevel.h>

#include "tools/PointerTool.h"
#include "tools/HandTool.h"
#include "tools/RulerTool.h"

#include "tools/DotTool.h"

#include "tools/LineTool.h"
#include "tools/CircleTool.h"

#include "tools/PolygonTool.h"
#include "tools/RectTool.h"
#include "tree/wsfile.h"
#include <wx/config.h>
#include <wx/stdpaths.h>

namespace
{
class NodeTimer : public wxTimer
{
public:
    explicit NodeTimer(gceContext &ctx, gceEditorFrame *ge) : m_ctx(ctx), m_editor(ge)
    {}
    ~NodeTimer()
    {}
    void Notify() override
    {
        log_to_ui();

        m_editor->onTimer();
    }
    gceContext &m_ctx;
    gceEditorFrame *m_editor = nullptr;
private:
    void log_to_ui()
    {
        // log strings to UI, merge duplicated
        int rep = 1;
        std::string msg, prev;
        while (m_ctx.checkLog(msg))
        {
            if (msg == prev)
            {
                if (rep < 1000)
                {
                    ++rep;
                }
                else
                {
                    dolog(msg, rep);
                }
            }
            else
            {
                if (!prev.empty())
                {
                    dolog(prev, rep);
                }
                prev = msg;
            }
        }
        if (!prev.empty())
        {
            dolog(prev, rep);
        }
    }

    void dolog(const std::string &msg, int &rep)
    {
        if (rep == 1)
        {
            wxLogMessage(to_wxstring(msg));
        }
        else
        {
            wxLogMessage("(%d) %s", rep, to_wxstring(msg));
            rep = 1;
        }
    }
};

template <bool read> class ConfigReaderWriter;

struct ConfigReaderWriter_base
{
    wxConfigBase *const m_cfg = wxConfig::Get();
};

template <> class ConfigReaderWriter<true> : public ConfigReaderWriter_base
{
public:
    template <typename A, typename B>
    bool operator() (const wxString &name, A &val, const B &defVal)
    {
        return m_cfg->Read(name, &val, defVal);
    }
};
template <> class ConfigReaderWriter<false> : public ConfigReaderWriter_base
{
public:
    template <typename A, typename B>
    bool operator() (const wxString &name, A &val, const B &)
    {
        return m_cfg->Write(name, val);
    }
};

}

class PersistentEditorFrame final : public wxPersistentTLW
{
public:
    explicit PersistentEditorFrame(gceEditorFrame *win) : wxPersistentTLW(win), m_saved(false)
    {}
    virtual wxString GetKind() const
    {
        return wxT("PersistentEditorFrame");
    }
    gceEditorFrame *Get() const
    {
        return static_cast<gceEditorFrame *>(wxPersistentTLW::Get());
    }
    virtual void Save() const
    {
        if (!m_saved)
        {
            // call from parent
            wxPersistentTLW::Save();

            SaveValue(perspectiveAUI, Get()->m_AuiManager.SavePerspective());

            m_saved = true;
        }
    }
    virtual bool Restore()
    {
        // call from parent
        bool res = wxPersistentTLW::Restore();
        if (res)
        {
            wxString val_perspective;
            if (RestoreValue(perspectiveAUI, &val_perspective))
            {
                Get()->m_AuiManager.LoadPerspective(val_perspective, true);
            }
        }
        return res;
    }
    mutable bool m_saved;
};
static PersistentEditorFrame *wxCreatePersistentObject(gceEditorFrame *win)
{
    return new PersistentEditorFrame(win);
}

///////////////////////////////////////////////////////////////////////////////
gceEditorFrame::gceEditorFrame(gceContext &ctx, const gceConfig &cfg)
    : wxFrame(nullptr, wxID_ANY, wxString()), m_selection(ctx, SELD_LAYER_UUID), m_AuiManager(this), m_actionProc(ctx), m_ctx(ctx), m_cfg(cfg)
{
    read_or_write<true>();

    /**/
    SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));

    CreateGUIControls();

    /* Events */
    wxEvtHandler::Bind(wxEVT_CLOSE_WINDOW, &gceEditorFrame::OnClose, this);

    // file menu
    wxEvtHandler::Bind(wxEVT_COMMAND_MENU_SELECTED, &gceEditorFrame::OnFileExit, this, wxID_EXIT);

    // menu view
    //wxEvtHandler::Bind (wxEVT_UPDATE_UI, &gceEditorFrame::OnLighterWindow_UUI, this, IDM_LTWND_PI);
    //wxEvtHandler::Bind (wxEVT_UPDATE_UI, &gceEditorFrame::OnWindowPI_UUI, this, IDM_WND_PI);

    wxEvtHandler::Bind(wxEVT_UPDATE_UI, &gceEditorFrame::OnViewPaneUUI, this, EditorCmds::__VIEW_PANE_FIRST, EditorCmds::__VIEW_PANE_LAST);

    wxEvtHandler::Bind(wxEVT_UPDATE_UI, &gceEditorFrame::OnSceneEnablesUI, this, EditorCmds::IDM_UTILS_ADDRASTER);

    wxEvtHandler::Bind(wxEVT_COMMAND_TOOL_CLICKED, &gceEditorFrame::OnToolRange, this, EditorCmds::ID_TOOL0, EditorCmds::ID_TOOL_MAX);
    wxEvtHandler::Bind(wxEVT_UPDATE_UI, &gceEditorFrame::OnToolRangeUUI, this, EditorCmds::ID_TOOL0, EditorCmds::ID_TOOL_MAX);

    wxEvtHandler::Bind(wxEVT_UPDATE_UI, &gceEditorFrame::OnEditUndo_UUI, this, wxID_UNDO);
    wxEvtHandler::Bind(wxEVT_UPDATE_UI, &gceEditorFrame::OnEditRedo_UUI, this, wxID_REDO);

    wxEvtHandler::Bind(wxEVT_CHAR_HOOK, &gceEditorFrame::onCharHook, this);

    if (!wxPersistentRegisterAndRestore(this, "MyGedit"))
    {
        SetSize(0, 0, 1024, 768);
        Center();
    }

    m_Timer = std::make_unique<NodeTimer>(ctx, this);
    m_Timer->Start(10);
}

gceEditorFrame::~gceEditorFrame()
{
    // call this here manualy - before AuiManager::UnInit, or perspective will not be saved correctly
    wxPersistenceManager::Get().Save(this);// AndUnregister(this);
    if (auto tool = this->GetCurrentTool()) tool->EndUse();

    read_or_write<false>();

    m_AuiManager.UnInit();
}

void gceEditorFrame::OnClose(wxCloseEvent &)
{
    m_Timer.reset();
    Destroy();
}

void gceEditorFrame::setToolString(const wxString &s)
{
    GetStatusBar()->SetStatusText(s, 0);
}

bool gceEditorFrame::CreateLogWindow(const wxAuiPaneInfo &pane)
{
    auto *p = new wxPanel(this);
    m_LogWindow = new wxTextCtrl(p, wxID_ANY, wxString(),
                                            wxDefaultPosition, wxDefaultSize,
                                            wxTE_MULTILINE | wxTE_READONLY);

    auto *tt = new wxTextCtrl(p, wxID_ANY, wxString(),
                                            wxDefaultPosition, wxDefaultSize, 0);
    auto *s = new wxBoxSizer(wxVERTICAL);
    p->SetSizer(s);
    s->Add(m_LogWindow, wxSizerFlags(1).Expand().Border(wxALL, 0));
    s->Add(tt, wxSizerFlags().Expand().Border(wxALL, 0));

    wxLog::SetActiveTarget(new wxLogTextCtrl(m_LogWindow));

    return m_AuiManager.AddPane(p, pane);
}
bool gceEditorFrame::CreateToolPanel(const wxAuiPaneInfo &pane)
{
    long style = wxTB_FLAT | wxTB_NODIVIDER | wxTB_VERTICAL;
    wxToolBar *m_toolBar = new wxToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, style, "Tools_ToolBar1");

    // now add some tools...
    int id = EditorCmds::ID_TOOL0;
    auto addTool = [&id, m_toolBar, this](std::unique_ptr<gceToolBase> &&tool){
        const gceToolInfo &info = tool->GetInfo();
        auto size = wxArtProvider::GetDIPSizeHint(wxART_TOOLBAR);
        auto size2 = wxArtProvider::GetSizeHint(wxART_TOOLBAR);
        auto icon = wxBitmapBundle::FromSVG(info.svgIcon, size).GetBitmap(size2);

        m_toolBar->AddRadioTool(id, info.name, icon, icon.ConvertToDisabled(), wxString(), info.help);
        this->m_tools[id] = std::move(tool);
        id++;
    };

    addTool(std::make_unique<gcePointerTool>(*this));
    addTool(std::make_unique<gceHandTool>(*this));
    addTool(std::make_unique<RulerTool>(*this));

    m_toolBar->AddSeparator();
    addTool(std::make_unique<DotTool>(*this));

    m_toolBar->AddSeparator();
    addTool(std::make_unique<gceLineTool>(*this));
    addTool(std::make_unique<CircleLineTool>(*this));

    m_toolBar->AddSeparator();
    addTool(std::make_unique<gcePolygonTool>(*this));
    addTool(std::make_unique<gceRectangleTool>(*this));
    addTool(std::make_unique<CirclePolyTool>(*this));
    m_toolBar->Realize();

    return m_AuiManager.AddPane(m_toolBar, pane);
}

void gceEditorFrame::OnSceneEnablesUI(wxUpdateUIEvent &evt)
{
    evt.Enable((bool)m_workspace);
}

std::vector<gce::uuid> gceEditorFrame::getModelsSelectable()
{
    std::set<gce::uuid> layers;
    for (auto &scene : m_workspace->getScenes())
    {
        if (scene.visible)
        {
            for (auto &scene_layer : m_workspace->getSceneLayers(scene.id_scene))
            {
                if (scene_layer.visible)
                {
                    layers.insert(scene_layer.id_model);
                }
            }
        }
    }
    return std::vector<gce::uuid>(layers.begin(), layers.end());
}

void gceEditorFrame::processMsg(const gce::MessageInfo &msg)
{
    // the hack to handle child tools
    auto *activeTool = dynamic_cast<gceToolBase *>(getCanvas()->GetEventHandler());

    if (std::holds_alternative<wspActiveLayerMsg>(msg))
    {
        processSetActiveLayer(msg);
        notifyOfActiveLayerChange();
    }
    else if (std::holds_alternative<wspLayerUpdatedMsg>(msg))
    {
        auto &lu_msg = std::get<wspLayerUpdatedMsg>(msg);
        if (lu_msg.info.id_layer == m_activeLayer.id_layer)
        {
            // update active layer info
            m_activeLayer.layerName = lu_msg.info.label;
        }
    }
    else if (std::holds_alternative<wspLayerDeletedMsg>(msg))
    {
        auto &lu_msg = std::get<wspLayerDeletedMsg>(msg);
        if (lu_msg.id_layer == m_activeLayer.id_layer)
        {
            m_activeLayer = EditorActiveLayerInfo{};
            notifyOfActiveLayerChange();
        }
    }
    else if (std::holds_alternative<wspModelUpdatedMsg>(msg))
    {
    }
    else if (std::holds_alternative<umodelSelectXDResultMsg>(msg))
    {
        if (activeTool != nullptr)
        {
            activeTool->processSelectResult(std::get<umodelSelectXDResultMsg>(msg));
        }
    }

    else if (std::holds_alternative<udataConnectReplyMsg>(msg))
    {
    // do nothing here
    }
    else if (std::holds_alternative<udataSelectReplyMsg>(msg))
    {
        if (activeTool != nullptr)
        {
            activeTool->processSelectDataResult(std::get<udataSelectReplyMsg>(msg));
        }
    }
    else if (std::holds_alternative<udataMultiRowActionNotifyMsg>(msg))
    {
        if (activeTool != nullptr)
        {
            activeTool->processActionNotify(std::get<udataMultiRowActionNotifyMsg>(msg));
        }
        this->m_selection.processActionNotify(std::get<udataMultiRowActionNotifyMsg>(msg));
    }
    else if (std::holds_alternative<urenderFrameMsMsg>(msg))
    {
        m_frameTime_ms = std::get<urenderFrameMsMsg>(msg).time_ms;
    }
}

void gceEditorFrame::processSetActiveLayer(const gce::MessageInfo &msg)
{
    auto &al_msg = std::get<wspActiveLayerMsg>(msg);
    m_activeLayer.id_layer = al_msg.info.id_layer;
    m_activeLayer.id_table = al_msg.info.id_table;
    m_activeLayer.id_scene = al_msg.info.id_scene;
    m_activeLayer.id_model = al_msg.info.id_model;
    m_activeLayer.sceneName = al_msg.info.sceneName;
    m_activeLayer.layerName = al_msg.info.layerName;

    if (m_workspace)
    {
        m_activeLayer.schema = m_workspace->findSchema(al_msg.info.id_schema);
    }
}

void gceEditorFrame::CreateGUIControls()
{
    //create base UI
    EditorCmds editorCmds(this);
    editorCmds.CreateMenu(this);
    editorCmds.CreateToolbar(this);
    createStatusBar();

    // create main canvas window
    m_canvas = new gceCanvas(this, m_ctx);
    m_AuiManager.AddPane(m_canvas, wxAuiPaneInfo().Name("graph_content").CenterPane());

    CreateToolPanel(wxAuiPaneInfo().Name("tbTools").ToolbarPane().Left().Layer(100).Gripper(false).DockFixed());

    wxPanel *toolpanel = new wxPanel(this);
    toolpanel->SetSizer(new wxBoxSizer(wxVERTICAL));
    m_AuiManager.AddPane(toolpanel, wxAuiPaneInfo().Name("test12").Bottom().CaptionVisible(false).
                         DockFixed().LeftDockable(false).RightDockable(false).
        BestSize(FromDIP(wxSize(800, 32))).Show(true).Gripper());
    m_toolOptParent = toolpanel;

    CreateLogWindow(wxAuiPaneInfo().Name(std::to_string(EditorCmds::IDM_VIEW_PANE_EVENTLOG)).Caption(EventLogPaneLabel)
                    .Bottom().Layer(10).LeftDockable(false).RightDockable(false)
        .BestSize(FromDIP(wxSize(800, 70))).MinSize(FromDIP(wxSize(-1, 70))));

    // workspace struct
    m_workspace = std::make_unique<gceWorkspace>(m_ctx, m_cfg);
    m_workspace->loadFile(to_string(m_lastWorkspaceFile), false);
    m_sceneTree = new gceDataTreeCtrl(this, *m_workspace, gceDataTreeCtrl::SCENE);
    m_modelsTree = new gceDataTreeCtrl(this, *m_workspace, gceDataTreeCtrl::MODELS);
    m_dataTree = new gceDataTreeCtrl(this, *m_workspace, gceDataTreeCtrl::DATASRC);
    wxAuiNotebook *ctrl = new wxAuiNotebook(this, wxID_ANY,
                                     wxDefaultPosition,
                                     wxDefaultSize,
                                     wxAUI_NB_BOTTOM | wxAUI_NB_TAB_FIXED_WIDTH | wxNO_BORDER);
    //ctrl->Freeze();

    ctrl->AddPage(m_sceneTree, "Scene", false);
    ctrl->AddPage(m_modelsTree, "Models", false);
    ctrl->AddPage(m_dataTree, "Data", false);

    m_AuiManager.AddPane(ctrl, wxAuiPaneInfo().Name(std::to_string(EditorCmds::IDM_VIEW_PANE_PROJECTTREE)).Caption(WorkspaceTreePaneLabel)
                         .Right().Dockable(true).TopDockable(false).BottomDockable(false)
                         .MinSize(FromDIP(wxSize(150, -1))).BestSize(FromDIP(wxSize(300, 400))));

    CreatePIFrame(wxAuiPaneInfo().Name(std::to_string(EditorCmds::IDM_WND_PI)).Caption(FeatureInspectorPaneLabel).Right()
                  .Dockable(true).TopDockable(false).BottomDockable(false)
        .BestSize(FromDIP(wxSize(300, 300))).MinSize(FromDIP(wxSize(150, -1))));


    m_AuiManager.Update();
    m_defaultPerspectiveAUI = m_AuiManager.SavePerspective();
    this->set_title();
}

bool gceEditorFrame::CreatePIFrame(const wxAuiPaneInfo &pane)
{
    return m_AuiManager.AddPane(new gcePIFrame(this), pane);
}

void gceEditorFrame::set_title()
{
    wxString title = to_wxstring(m_activeLayer.sceneName);
    if (!title.empty())
    {
        title << "/";
    }
    title << to_wxstring(m_activeLayer.layerName);
    if (!title.empty())
    {
        title << " - ";
    }
    if (m_workspace && m_workspace->isOk())
    {
        title << to_wxstring(m_workspace->getName());
    }
    title << " | Geographic Client Engine";

    if (title != GetTitle())
    {
        SetTitle(title);
    }
}

void gceEditorFrame::notifyOfActiveLayerChange()
{
    auto *tool = this->GetCurrentTool();
    if (tool != nullptr && tool->isInputTool())
    {
        if (!this->SelectTool(-1))
        {
            wxLogError("Failed to deselect tool");
        }
    }
    set_title();
}

gceToolBase *gceEditorFrame::GetCurrentTool()
{
    if (auto itCurrent = m_tools.find(m_currentToolId); itCurrent != m_tools.end())
    {
        return itCurrent->second.get();
    }
    return nullptr;
}

void gceEditorFrame::UpdateStatus()
{

    auto *sb = GetStatusBar();
#if 1 // 2D
    {
        // mouse position in project space
        wxPoint     mp;
        if (m_canvas->getMousePosition(mp))
        {
            glm::dvec3 cpt;
            m_canvas->CoordFromPoint(cpt, mp);

            sb->SetStatusText(to_wxstring(gceProjection::formatPosition(cpt)), 1);
        }

        // current scale
        sb->SetStatusText(wxString::Format("1:%.0f", m_canvas->GetMapScale()), 2);

    }
    // camera position in project space
#else
    {
        psMoveUnit &glutI = m_canvas->glutI;

        const dvec3 cpos = IScene::s_camWGS;
        fields[1].clear();
        wgsop::formatString_WGS(fields[1], cpos.y, cpos.x);

        if (cpos.z < 10000.0)
        {
            fields[1] << wxString::Format(wxT("; %.2fm"), cpos.z);
        }
        else
        {
            fields[1] << wxString::Format(wxT("; %.2fkm"), cpos.z / 1000.0);
        }
        // current scale
        {
            double scale = 25.0;
            double v = scale * sqrt(glutI.v_move * glutI.v_move + glutI.v_lift * glutI.v_lift + glutI.v_strafe * glutI.v_strafe);
            double a = scale * glutI.a;
            fields[2] = wxString::Format("v=%.3f; a=%.3f", v, a);
        }
    }
#endif

    sb->SetStatusText(wxString::Format("%.1fms", m_frameTime_ms), 3);

}


void gceEditorFrame::createStatusBar()
{
    const std::array widths{
        -1,
        this->GetTextExtent("88.888888, 88.888888, 8888.88m").x,
        this->GetTextExtent("1:8888888888").x,
        this->GetTextExtent("8.888ms").x,
    };

    wxFrame::CreateStatusBar()->SetFieldsCount(widths.size(), widths.data());
}


bool gceEditorFrame::SelectTool(int id)
{
    if (id != m_currentToolId)
    {
        if (auto itCurrent = m_tools.find(m_currentToolId); itCurrent == m_tools.end() || itCurrent->second->EndUse())
        {
            m_currentToolId = id;
            if (auto itNew = m_tools.find(id); itNew != m_tools.end())
            {
                itNew->second->BeginUse(nullptr);
            }
            return true;
        }
        return false;
    }
    return true;
}

void gceEditorFrame::OnFileOpenWorkspace(wxCommandEvent &)
{
    const wxString fileName = wxLoadFileSelector(_("Open workspace"), "Workspace file (*.xml)|*.xml");
    doOpenWorkspace(fileName);
}

void gceEditorFrame::doOpenWorkspace(const wxString &fileName)
{
    if (fileName.empty())
    {
        return;
    }
    if (m_workspace->loadFile(to_string(fileName), true))
    {
        m_dataTree->onLoadFile();
        m_sceneTree->onLoadFile();
        m_lastWorkspaceFile = fileName;
        this->set_title();
    }
}

void gceEditorFrame::OnFileNewWorkspace(wxCommandEvent &)
{
    const wxString fileName = wxSaveFileSelector(_("Save new workspace"), "Workspace file (*.xml)|*.xml", "New Globe workspace.xml");

    doOpenWorkspace(fileName);
}

void gceEditorFrame::OnFileExit(wxCommandEvent &)
{
    int answer = wxMessageBox("Quit program?", "Confirm", wxYES_NO, this);
    if (answer == wxYES)
    {
        m_Timer.reset();
        Destroy();
    }
}


void gceEditorFrame::OnViewPane(wxCommandEvent &event)
{
    wxAuiPaneInfo &pane = m_AuiManager.GetPane(std::to_string(event.GetId()));
    if (pane.IsOk())
    {
        pane.Show(event.IsChecked());
    }
    m_AuiManager.Update();
}

void gceEditorFrame::OnViewPanesDefault(wxCommandEvent &)
{
    m_AuiManager.LoadPerspective(m_defaultPerspectiveAUI);
}

void gceEditorFrame::OnLogSave(wxCommandEvent &)
{
    wxFileDialog dlg(nullptr, wxFileSelectorPromptStr, wxString(), "log.txt", "Text files (*.txt)|*.txt|All files (*.*)|*.*", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (dlg.ShowModal() == wxID_OK)
    {
        if (!m_LogWindow->SaveFile(dlg.GetPath()))
        {
            wxMessageBox(wxString::Format("Failed to save %s", dlg.GetPath()));
        }
    }
}


void gceEditorFrame::OnLogClear(wxCommandEvent &)
{
    m_LogWindow->SetValue({});
}

void gceEditorFrame::OnViewPaneUUI(wxUpdateUIEvent &event)
{
    wxAuiPaneInfo &pane = m_AuiManager.GetPane(std::to_string(event.GetId()));
    if (pane.IsOk())
    {
        event.Check(pane.IsShown());
    }
}

void gceEditorFrame::OnViewModeFlat(wxCommandEvent &)
{
    getCanvas()->setMode(gceRenderMode::FLAT);
}

void gceEditorFrame::OnViewModeGlobe(wxCommandEvent &)
{
    getCanvas()->setMode(gceRenderMode::GLOBE);
}

void gceEditorFrame::OnAbout(wxCommandEvent &)
{
    wxAboutDialogInfo info;

    info.SetName("FreeGVE");
    info.SetDescription("Free globe visualization engine");
    info.SetVersion("based on " + wxGetLibraryVersionInfo().ToString());

    wxAboutBox(info);
}

void gceEditorFrame::OnFileExport(wxCommandEvent &)
{}

void gceEditorFrame::OnFileImport(wxCommandEvent &)
{}

void gceEditorFrame::OnUtilsAddRaster(wxCommandEvent &)
{}

#include "demcachedlg.h"
#include "alg/wgsop.h"
#include "datasrc/demdata.h"
void gceEditorFrame::OnUtilsImportDEM(wxCommandEvent &)
{
    wxString pathSrc, pathCache;
    gcePyramidAOI pyramidAOI;
    pyramidAOI.bbox = {{-180.0, -wgsop::flat2wgs_Y(M_PI)}, {180.0, wgsop::flat2wgs_Y(M_PI)}};
    pyramidAOI.levMin = 1;
    pyramidAOI.levMax = 6;

    gceDEMCacheDialog dlg(this, pathCache, pathSrc, pyramidAOI);
    if (dlg.ShowModal() == wxID_OK)
    {
        std::thread t(dem_updateCache, to_string(pathCache), to_string(pathSrc), "", pyramidAOI);
        t.detach();
    }
}

void gceEditorFrame::OnUtilsImportModel3D(wxCommandEvent &)
{}

void gceEditorFrame::OnUtilsExportModel3D(wxCommandEvent &)
{}

void gceEditorFrame::OnFileExportDB(wxCommandEvent & /*event*/)
{}

void gceEditorFrame::OnFileImportDB(wxCommandEvent & /*event*/)
{}

void gceEditorFrame::onTimer()
{
    gce::MessageInfo msg;
    while (m_ctx.workspaceQueue.tryGet(msg))
    {
        this->processMsg(msg);
        this->m_actionProc.processMsg(msg);
        this->m_dataTree->processMsg(msg);
        this->m_modelsTree->processMsg(msg);
        this->m_sceneTree->processMsg(msg);
    }

    UpdateStatus();
    m_canvas->postSceneParams();
}

void gceEditorFrame::OnEditUndo(wxCommandEvent &)
{
    m_actionProc.postUndo();
}

void gceEditorFrame::OnEditUndo_UUI(wxUpdateUIEvent &event)
{
    std::string s;
    event.Enable(m_actionProc.canUndo(s));
    SetToolLongHelp(event, s);
}

void gceEditorFrame::OnEditRedo(wxCommandEvent &)
{
    m_actionProc.postRedo();
}

void gceEditorFrame::OnEditRedo_UUI(wxUpdateUIEvent &event)
{
    std::string s;
    event.Enable(m_actionProc.canRedo(s));
    SetToolLongHelp(event, s);
}
void gceEditorFrame::OnZoomToFullExtent(wxCommandEvent &)
{
    if (getCanvas()->isFlat())
    {
        getCanvas()->SetViewFlat(glm::dvec2(0.0, 0.0), 1.0e-8);
    }
    else // if (getCanvas()->isGlobe())
    {
        getCanvas()->SetViewGlobe(glm::dvec2(45.0), 4000000.0);
    }
}

void gceEditorFrame::OnZoomToSelected(wxCommandEvent &)
{
    ShowSelection();
}

void gceEditorFrame::OnSceneRefresh(wxCommandEvent &)
{}
void gceEditorFrame::OnShowGrid(wxCommandEvent &)
{
    // TODO: implement coordinate grid rendering
}

void gceEditorFrame::OnTerrainModeNormal(wxCommandEvent &)
{
    m_canvas->setTerrainWireFrame(false);
}

void gceEditorFrame::OnTerrainModeGrid(wxCommandEvent &)
{
    m_canvas->setTerrainWireFrame(true);
}

void gceEditorFrame::onCharHook(wxKeyEvent &event)
{
    if (auto *activeTool = dynamic_cast<gceToolBase *>(getCanvas()->GetEventHandler()); activeTool)
    {
        if (!activeTool->processCommandKey(event))
        {
            event.Skip();
        }
    }
    else
    {
        event.Skip();
    }
}

void gceEditorFrame::PushToolOptions(gceToolOptions *tool_options)
{
    // remove current options
    if (!m_toolOptStack.empty())
    {
        gceToolOptions *tbar = m_toolOptStack.top();
        GetToolOptParent()->GetSizer()->Detach(tbar);
        tbar->Show(false);
    }
    // push new tool
    m_toolOptStack.push(tool_options);

    AttachTopToolOptions();
}

void gceEditorFrame::PopToolOptions()
{
    // remove current options and pop
    if (!m_toolOptStack.empty())
    {
        gceToolOptions *tbar = m_toolOptStack.top();
        m_toolOptStack.pop();

        GetToolOptParent()->GetSizer()->Detach(tbar);
        tbar->Show(false);

        AttachTopToolOptions();
    }
}
void gceEditorFrame::AttachTopToolOptions()
{
    if (!m_toolOptStack.empty())
    {
        auto *tbar = m_toolOptStack.top();
        tbar->Layout();
        GetToolOptParent()->GetSizer()->Add(tbar, 0, wxGROW | wxALL, 3);
        tbar->Show(true);
        GetToolOptParent()->GetSizer()->Layout();
    }
}
wxWindow *gceEditorFrame::GetToolOptParent()
{
    if (!m_toolOptParent)
    {
        wxMessageBox("!m_toolOptParent");
    }
    return m_toolOptParent;
}

void gceEditorFrame::ShowSelection()
{
    geom::Box3D box;
    for (auto &ref : this->getSelection())
    {
        if (auto g = ref.entity.get_geometry())
        {
            box.expand(g->bbox());
        }
    }
    if (box.empty()) return;
    const glm::dvec2 location = box.GetCenter();

    auto *proj = getCanvas()->getProj();
    if (!proj) return;

    if (getCanvas()->isFlat())
    {
        geom::Box3D flatbox;
        proj->fromInternal(flatbox.cmin, box.cmin);
        proj->fromInternal(flatbox.cmax, box.cmax);
        double size = glm::dot(flatbox.size(), glm::dvec3(1.0)) * proj->getScale(location);
        getCanvas()->SetViewFlat(location, 0.2 / size);

    }
    else // if (getCanvas()->isGlobe())
    {
        double size = glm::dot(glm::radians(box.size()), glm::dvec3(proj->getScale(location), wgsop::Rb, 1.0));
        getCanvas()->SetViewGlobe(location, 2.0 * size);
    }
}

void gceEditorFrame::OnToolRange(wxCommandEvent &event)
{
    this->SelectTool(event.GetId());
}


void gceEditorFrame::OnToolRangeUUI(wxUpdateUIEvent &event)
{
    if (auto it = m_tools.find(event.GetId()); it != m_tools.end())
    {
        event.Enable(it->second->isAvailable());
        event.Check(event.GetId() == this->GetCurrentToolId());
    }
    else
    {
        event.Skip();
    }
}
template<bool read>
inline void gceEditorFrame::read_or_write()
{
    ConfigReaderWriter<read> cfg;

    cfg("lastWorkspacePath", m_lastWorkspaceFile, wxStandardPaths::Get().GetDocumentsDir() + "/gisdom.xml");
}
