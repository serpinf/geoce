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

#include "menucmd.h"

namespace
{
const char *perspectiveAUI = "perspectiveAUI";
const char *EventLogPaneLabel = "Event log";
const char *WorkspaceTreePaneLabel = "Workspace tree";
const char *LightingPaneLabel = "Lighting properties";
const char *TimerPaneLabel = "Timer properties";
const char *FeatureInspectorPaneLabel = "Feature inspector";

void SetToolLongHelp(wxUpdateUIEvent &event, const wxString &longHelp)
{
    wxToolBar *tb = dynamic_cast<wxToolBar *>(event.GetEventObject());
    if (tb)
        tb->SetToolLongHelp(event.GetId(), longHelp);
}
}
//const char layer_svg[] = {R"rawsvg()rawsvg"};
const char view_3D_svg[] = {R"rawsvg(<svg xmlns="http://www.w3.org/2000/svg" width="200" height="200" viewBox="0 0 20 20"><g fill="none"><path fill="url(#fluentColorGlobe201)" d="M10 18a8 8 0 1 0 0-16a8 8 0 0 0 0 16"/><path fill="url(#fluentColorGlobe200)" fill-rule="evenodd" d="M7.853 2.291a7 7 0 0 0-.816 1.51c-.368.906-.65 1.995-.826 3.199H2.58q-.195.485-.33 1h3.84a22 22 0 0 0 .001 4h-3.84q.135.515.33 1h3.63c.176 1.204.458 2.293.826 3.199a7 7 0 0 0 .816 1.51A8 8 0 0 0 10 18a8 8 0 0 0 2.147-.291a7 7 0 0 0 .816-1.51c.368-.906.65-1.995.826-3.199h3.63q.195-.485.329-1h-3.84a21.6 21.6 0 0 0 0-4h3.84a8 8 0 0 0-.33-1H13.79c-.176-1.204-.458-2.293-.826-3.199a7 7 0 0 0-.816-1.51A8 8 0 0 0 10 2a8 8 0 0 0-2.147.291M7.223 7c.166-1.076.42-2.035.74-2.822c.298-.733.642-1.292 1.003-1.66C9.324 2.153 9.672 2 10 2s.676.153 1.034.518c.36.368.705.927 1.003 1.66c.32.787.574 1.746.74 2.822zM10 18c.328 0 .676-.153 1.034-.518c.36-.368.705-.927 1.003-1.66c.32-.787.574-1.746.74-2.822H7.223c.167 1.076.421 2.035.741 2.822c.298.733.642 1.292 1.003 1.66c.358.365.706.518 1.034.518m-3-8c0 .692.033 1.362.096 2h5.808A21 21 0 0 0 13 10c0-.692-.033-1.362-.096-2H7.096A21 21 0 0 0 7 10" clip-rule="evenodd"/><defs><radialGradient id="fluentColorGlobe200" cx="0" cy="0" r="1" gradientTransform="rotate(-135 10.4 3.895)scale(12.7313)" gradientUnits="userSpaceOnUse"><stop stop-color="#25A2F0"/><stop offset=".974" stop-color="#3BD5FF"/></radialGradient><linearGradient id="fluentColorGlobe201" x1="5.556" x2="17.111" y1="4.667" y2="15.333" gradientUnits="userSpaceOnUse"><stop stop-color="#29C3FF"/><stop offset="1" stop-color="#2052CB"/></linearGradient></defs></g></svg>)rawsvg"};
const char view_2D_svg[] = {R"rawsvg(<svg xmlns="http://www.w3.org/2000/svg" width="200" height="200" viewBox="0 0 100 100"><path fill="#F2F2F2" fill-rule="evenodd" d="M75 87.425L50 100L25 87.425L0 100V12.576L25 0l25 12.576L75 0l25 12.576V100L75 87.425z" clip-rule="evenodd"/><path fill="none" stroke="#6BC9F2" stroke-miterlimit="10" stroke-width="4" d="M15 60V32l9.988-5.006L50 41l25-12l12 4" clip-rule="evenodd"/><path fill="none" stroke="#E64C3C" stroke-miterlimit="10" stroke-width="4" d="M15 61v-8l10-5l25 13l25-11l12-5V32" clip-rule="evenodd"/><path fill="none" stroke="#F29C1F" stroke-miterlimit="10" stroke-width="4" d="m15 61l35 18l17-8V43l20-10" clip-rule="evenodd"/><path fill="#fff" fill-rule="evenodd" d="M87 36.5c-1.93 0-3.5-1.57-3.5-3.5s1.57-3.5 3.5-3.5s3.5 1.57 3.5 3.5s-1.57 3.5-3.5 3.5z" clip-rule="evenodd"/><path fill="#2980BA" d="M87 31c1.103 0 2 .897 2 2s-.897 2-2 2s-2-.897-2-2s.897-2 2-2m0-3a5 5 0 1 0 .001 10.001A5 5 0 0 0 87 28z"/><path fill="#fff" fill-rule="evenodd" d="M15 64.5c-1.93 0-3.5-1.57-3.5-3.5s1.57-3.5 3.5-3.5s3.5 1.57 3.5 3.5s-1.57 3.5-3.5 3.5z" clip-rule="evenodd"/><path fill="#2980BA" d="M15 59c1.103 0 2 .897 2 2s-.897 2-2 2s-2-.897-2-2s.897-2 2-2m0-3a5 5 0 1 0 .001 10.001A5 5 0 0 0 15 56z"/><path fill="#2C3E50" fill-rule="evenodd" d="m0 100l25-12.576V0L0 12.576V100zm50-87.424V100l25-12.576V0L50 12.576z" clip-rule="evenodd" opacity=".15"/></svg>)rawsvg"};
const char import_svg[] = {R"rawsvg(<svg xmlns="http://www.w3.org/2000/svg" width="200" height="200" viewBox="0 0 50 50"><g fill="none" stroke-linecap="round" stroke-linejoin="round" stroke-width="2"><path stroke="#344054" d="M35.417 27.083h-12.5v-12.5M43.75 6.25L22.917 27.083"/><path stroke="#306CFE" d="M43.75 27.083v14.584a2.083 2.083 0 0 1-2.083 2.083H8.333a2.083 2.083 0 0 1-2.083-2.083V8.333A2.083 2.083 0 0 1 8.333 6.25h14.584"/></g></svg>)rawsvg"};
const char export_svg[] = {R"rawsvg(<svg xmlns="http://www.w3.org/2000/svg" width="200" height="200" viewBox="0 0 50 50"><g fill="none" stroke-linecap="round" stroke-linejoin="round" stroke-width="2"><path stroke="#344054" d="M31.25 6.25h12.5v12.5m-20.833 8.333L43.75 6.25"/><path stroke="#306CFE" d="M43.75 27.083v14.584a2.083 2.083 0 0 1-2.083 2.083H8.333a2.083 2.083 0 0 1-2.083-2.083V8.333A2.083 2.083 0 0 1 8.333 6.25h14.584"/></g></svg>)rawsvg"};
const char focus_svg[] = {R"rawsvg(<svg xmlns="http://www.w3.org/2000/svg" width="200" height="200" viewBox="0 0 48 48"><g fill="none"><path stroke="#000" stroke-linecap="round" stroke-linejoin="round" stroke-width="4" d="M16 6H8C6.89543 6 6 6.89543 6 8V16"/><path stroke="#000" stroke-linecap="round" stroke-linejoin="round" stroke-width="4" d="M16 42H8C6.89543 42 6 41.1046 6 40V32"/><path stroke="#000" stroke-linecap="round" stroke-linejoin="round" stroke-width="4" d="M32 42H40C41.1046 42 42 41.1046 42 40V32"/><path stroke="#000" stroke-linecap="round" stroke-linejoin="round" stroke-width="4" d="M32 6H40C41.1046 6 42 6.89543 42 8V16"/><rect width="20" height="20" x="14" y="14" fill="#2F88FF" stroke="#000" stroke-width="4" rx="10"/><circle r="3" fill="#fff" transform="matrix(-1 0 0 1 24 24)"/></g></svg>)rawsvg"};
const char focus_one_svg[] = {R"rawsvg(<svg xmlns="http://www.w3.org/2000/svg" width="200" height="200" viewBox="0 0 48 48"><g fill="none"><path stroke="#000" stroke-linecap="round" stroke-linejoin="round" stroke-width="4" d="M16 6H8C6.89543 6 6 6.89543 6 8V16"/><path stroke="#000" stroke-linecap="round" stroke-linejoin="round" stroke-width="4" d="M16 42H8C6.89543 42 6 41.1046 6 40V32"/><path stroke="#000" stroke-linecap="round" stroke-linejoin="round" stroke-width="4" d="M32 42H40C41.1046 42 42 41.1046 42 40V32"/><path stroke="#000" stroke-linecap="round" stroke-linejoin="round" stroke-width="4" d="M32 6H40C41.1046 6 42 6.89543 42 8V16"/><path fill="#2F88FF" stroke="#000" stroke-linecap="round" stroke-linejoin="round" stroke-miterlimit="10" stroke-width="4" d="M24 31C27.866 31 31 27.866 31 24C31 20.134 27.866 17 24 17C20.134 17 17 20.134 17 24C17 27.866 20.134 31 24 31Z"/><path stroke="#000" stroke-linecap="round" stroke-linejoin="round" stroke-miterlimit="10" stroke-width="4" d="M24 17L24 13"/><path stroke="#000" stroke-linecap="round" stroke-linejoin="round" stroke-miterlimit="10" stroke-width="4" d="M24 35L24 31"/><path stroke="#000" stroke-linecap="round" stroke-linejoin="round" stroke-miterlimit="10" stroke-width="4" d="M35 24H31"/><path stroke="#000" stroke-linecap="round" stroke-linejoin="round" stroke-miterlimit="10" stroke-width="4" d="M17 24H13"/><path fill="#fff" d="M24 26C25.1046 26 26 25.1046 26 24C26 22.8954 25.1046 22 24 22C22.8954 22 22 22.8954 22 24C22 25.1046 22.8954 26 24 26Z"/></g></svg>)rawsvg"};
const char grid_svg[] = {R"rawsvg(<svg xmlns="http://www.w3.org/2000/svg" width="200" height="200" viewBox="0 0 15 15"><path fill="none" stroke="currentColor" d="M0 5.5h15m-15-4h15m-15 8h15m-15 4h15M9.5 0v15m4-15v15m-8-15v15m-4-15v15"/></svg>)rawsvg"};
const char terrain_solid_svg[] = {R"rawsvg(<svg xmlns="http://www.w3.org/2000/svg" width="200" height="200" viewBox="0 0 24 24"><path fill="currentColor" d="m14 6l-3.75 5l2.85 3.8l-1.6 1.2C9.81 13.75 7 10 7 10l-6 8h22L14 6z"/></svg>)rawsvg"};
const char terrain_wire_svg[] = {R"rawsvg(<svg xmlns="http://www.w3.org/2000/svg" width="200" height="200" viewBox="0 0 24 24"><path fill="currentColor" d="m14 6l-4.22 5.63l1.25 1.67L14 9.33L19 16h-8.46l-4.01-5.37L1 18h22L14 6zM5 16l1.52-2.03L8.04 16H5z"/></svg>)rawsvg"};

class EditorCmds : public gceMenuCmds
{
    gceEditorFrame *handler = nullptr;
public:
    enum gcEditorFrameMenuIds
    {
        cmdFIRST = wxID_HIGHEST + 1,
        IDM_FILE_NEW_WORKSPACE,
        IDM_FILE_OPEN_WORKSPACE,

        __VIEW_PANE_FIRST,
        IDM_VIEW_PANE_PROJECTTREE,
        IDM_VIEW_PANE_EVENTLOG,
        IDM_VIEW_PANES_DEFAULT,
        IDM_WND_PI,
        __VIEW_PANE_LAST,

        IDM_VIEW_ZoomToFullExtent,
        IDM_VIEW_ZoomToSelected,
        IDM_VIEW_ShowGrid,
        IDM_VIEW_TerrainNormal,
        IDM_VIEW_TerrainGrid,
        IDM_VIEW_Refresh,
        IDM_VIEW_2D,
        IDM_VIEW_3D,

        IDM_LOG_SAVE,
        IDM_LOG_CLEAR,

        IDM_UTILS_ADDRASTER,

        IDM_UTILS_IMPORT_DEM,

        IDM_UTILS_IMPORT_MODEL3D,
        IDM_UTILS_EXPORT_MODEL3D,

        IDC_TB_Scale,

        cmdLAST,
        ID_TOOL0,
        ID_TOOL_MAX = ID_TOOL0 + 100,
    };
    typedef void (gceEditorFrame:: *Method)(wxCommandEvent &);

    explicit EditorCmds(gceEditorFrame *handler) : handler(handler)
    {
        add(&gceEditorFrame::OnFileNewWorkspace, IDM_FILE_NEW_WORKSPACE, _("New workspace...")).Icon(wxART_NEW);
        add(&gceEditorFrame::OnFileOpenWorkspace, IDM_FILE_OPEN_WORKSPACE, _("Open workspace...")).Icon(wxART_FILE_OPEN);

        add(&gceEditorFrame::OnEditUndo, wxID_UNDO).Icon(wxART_UNDO);
        add(&gceEditorFrame::OnEditRedo, wxID_REDO).Icon(wxART_REDO);

        add(&gceEditorFrame::OnZoomToFullExtent, IDM_VIEW_ZoomToFullExtent, _("Show all")).IconSVG(focus_svg);
        add(&gceEditorFrame::OnZoomToSelected, IDM_VIEW_ZoomToSelected, _("Show selected")).IconSVG(focus_one_svg);
        add(&gceEditorFrame::OnSceneRefresh, wxID_REFRESH).Icon(wxART_REFRESH);
        add(&gceEditorFrame::OnShowGrid, IDM_VIEW_ShowGrid, _("Show grid\tAlt+g")).IconSVG(grid_svg).Check();
        add(&gceEditorFrame::OnTerrainModeNormal, IDM_VIEW_TerrainNormal, _("Show terrain\tAlt+f")).IconSVG(terrain_solid_svg).Radio();
        add(&gceEditorFrame::OnTerrainModeGrid, IDM_VIEW_TerrainGrid, _("Show terrain as GRID\tAlt+t")).IconSVG(terrain_wire_svg).Radio();
        add(&gceEditorFrame::OnViewPane, IDM_VIEW_PANE_PROJECTTREE, WorkspaceTreePaneLabel).Check();
        add(&gceEditorFrame::OnViewPane, IDM_VIEW_PANE_EVENTLOG, EventLogPaneLabel).Check();
        add(&gceEditorFrame::OnViewPane, IDM_WND_PI, FeatureInspectorPaneLabel).Check();
        add(&gceEditorFrame::OnViewPanesDefault, IDM_VIEW_PANES_DEFAULT, _("Default layout"));
        add(&gceEditorFrame::OnViewModeFlat, IDM_VIEW_2D, _("2D mode")).IconSVG(view_2D_svg).Radio();
        add(&gceEditorFrame::OnViewModeGlobe, IDM_VIEW_3D, _("3D mode")).IconSVG(view_3D_svg).Radio();

        add(&gceEditorFrame::OnLogSave, IDM_LOG_SAVE, to_wxstring("Save log…"));
        add(&gceEditorFrame::OnLogClear, IDM_LOG_CLEAR, _("Clear log"));

        add(&gceEditorFrame::OnUtilsImportModel3D, IDM_UTILS_IMPORT_MODEL3D, _("Import 3D-model"));
        add(&gceEditorFrame::OnUtilsExportModel3D, IDM_UTILS_EXPORT_MODEL3D, _("Export 3D-model to file (3ds, dae)"));
        add(&gceEditorFrame::OnUtilsAddRaster, IDM_UTILS_ADDRASTER, _("Add raster image"));
        add(&gceEditorFrame::OnUtilsImportDEM, IDM_UTILS_IMPORT_DEM, _("Import DEM..."));

        add(&gceEditorFrame::OnAbout, wxID_ABOUT);
    }

    gceMenuCommand &add(Method method, int id, const wxString text = wxString())
    {
        handler->Bind(wxEVT_MENU, method, handler, id);
        return gceMenuCmds::add(id, text);
    }

    void CreateMenu(wxFrame *frame)
    {
        wxMenu *menuFile = new wxMenu;
        MenuItems(menuFile, IDM_FILE_OPEN_WORKSPACE, IDM_FILE_NEW_WORKSPACE);
        MenuItems(menuFile, wxID_PRINT, wxID_EXIT);

        // View menu
        wxMenu *menuView = new wxMenu;
        MenuItems(menuView, IDM_VIEW_ZoomToFullExtent, IDM_VIEW_ZoomToSelected, wxID_REFRESH);
        MenuItems(menuView, IDM_VIEW_ShowGrid);
        MenuItems(menuView, IDM_VIEW_TerrainNormal, IDM_VIEW_TerrainGrid);
        MenuItems(menuView, IDM_VIEW_PANE_PROJECTTREE, IDM_VIEW_PANE_EVENTLOG, IDM_WND_PI);
        MenuItems(menuView, IDM_VIEW_PANES_DEFAULT);

        // Edit menu
        wxMenu *menuEdit = new wxMenu;
        MenuItems(menuEdit, wxID_UNDO, wxID_REDO);
        MenuItems(menuEdit, IDM_LOG_SAVE, IDM_LOG_CLEAR);

        // utils
        wxMenu *menuUtils = new wxMenu;
        MenuItems(menuUtils, IDM_UTILS_IMPORT_MODEL3D, IDM_UTILS_EXPORT_MODEL3D);
        MenuItems(menuUtils, IDM_UTILS_ADDRASTER, IDM_UTILS_IMPORT_DEM);

        wxMenu *menuHelp = new wxMenu;
        MenuItems(menuHelp, wxID_HELP, wxID_ABOUT);

        // create the Menu bar
        wxMenuBar *menuBar = new wxMenuBar();
        menuBar->Append(menuFile, "&File");
        menuBar->Append(menuEdit, "&Edit");
        menuBar->Append(menuView, "&View");
        menuBar->Append(menuUtils, _("Utils"));
        menuBar->Append(menuHelp, _("&Help"));

        frame->SetMenuBar(menuBar);

    }
    void CreateToolbar(wxFrame *frame)
    {
        wxToolBar *tb = frame->CreateToolBar();

        ToolItems(tb, IDM_VIEW_ZoomToFullExtent, IDM_VIEW_ZoomToSelected);
        ToolItems(tb, wxID_REFRESH);
        ToolItems(tb, wxID_UNDO, wxID_REDO);

        ToolItems(tb, IDM_VIEW_ShowGrid, IDM_VIEW_TerrainNormal, IDM_VIEW_TerrainGrid);

        ToolItems(tb, IDM_VIEW_2D, IDM_VIEW_3D);

        tb->Realize();
    }
};
