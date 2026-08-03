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

#include "wx/stdpaths.h"
#include <wx/snglinst.h>
#include "geos_c.h"

#include "editorfrm.h"

#include "gisdom/node/urendernode.hpp"
#include "gisdom/node/udatanode.hpp"
#include "gisdom/node/umodelnode.h"
#include "config.h"

//#define USE_VLD

#ifdef USE_VLD
#include "vld.h"
#endif

#include <array>
#include "gisdom/engine.hpp"

class wxSingleInstanceChecker;
static void LogMessage(const char *format, ...)
{
    char buf[2048];
    va_list args;
    va_start(args, format);
    std::vsnprintf(buf, 2047, format, args);
    va_end(args);

    wxLogMessage("geos message: %s", wxString::FromUTF8(buf));
}
static void LogError(const char *format, ...)
{
    char buf[2048];
    va_list args;
    va_start(args, format);
    std::vsnprintf(buf, 2047, format, args);
    va_end(args);

    wxLogError("geos error: %s", wxString::FromUTF8(buf));
}

class GisdomClientAppWX : public wxApp
{
public:
    GisdomClientAppWX() {}

    bool OnInit() override
    {
#ifdef USE_VLD
        //VLDEnable();
#endif
        SetAppName("Gisdom");
        m_checker = std::make_unique<wxSingleInstanceChecker>();

#ifdef NDEBUG
        if (m_checker->IsAnotherRunning())
        {
            wxMessageBox("Attempt to run program twice!");
            return false;
        }
#endif
        wxStandardPaths::Get().DontIgnoreAppSubDir();

        // load config before any worker thread starts 
        m_cfg.load("");

        if (!InitModules())
        {
            return false;
        }
        m_asyncSvc[0] = std::thread(gce::udata_worker, &m_ctx);
        m_asyncSvc[1] = std::thread(gce::umodel_worker, &m_ctx);
        //m_ctx = gisdom_context_create();


        gceEditorFrame *ge = new gceEditorFrame(m_ctx, m_cfg);
        ge->Show(true);
        SetTopWindow(ge);

        return true;
    }

    int OnExit() override
    {
        ::finishGEOS();
#ifdef USE_VLD
        //VLDDisable();
#endif

        m_ctx.postQueue(gce::queueId::DATA | gce::queueId::MODEL, gceQuitMessage{});

        for (auto &svc : m_asyncSvc)
        {
            svc.join();
        }

        return wxApp::OnExit();
    }

private:
    bool InitModules()
    {
        ::wxInitAllImageHandlers();
        ::initGEOS(LogMessage, LogError);

        return true;
    }

    std::unique_ptr<wxSingleInstanceChecker> m_checker;
    gceConfig m_cfg;
    gceContext m_ctx{m_cfg};
    //gceContext m_ctx;
    std::array<std::thread, 2> m_asyncSvc;
};

wxIMPLEMENT_APP(GisdomClientAppWX);
