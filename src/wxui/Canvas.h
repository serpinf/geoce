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

#include <wx/glcanvas.h>
#include <stack>
#include "gisdom/engine.hpp"
#include "gisdom/alg/geoproj.h"

class gceEditorFrame;
class gceRendererGL;
const uint8_t TOOL_LAYER_UUID_data[] = {'g', 'c', 'e', 't', 'o', 'o', 'l', 'l', 'a', 'y', 'e', 'r', 'u', 'u', 'i', 'd'};
const uint8_t SELD_LAYER_UUID_data[] = {'g', 'c', 'e', 's', 'e', 'l', 'd', 'l', 'a', 'y', 'e', 'r', 'u', 'u', 'i', 'd'};
const gce::uuid TOOL_LAYER_UUID{TOOL_LAYER_UUID_data};
const gce::uuid SELD_LAYER_UUID{SELD_LAYER_UUID_data};

class gceCanvas final : public wxGLCanvas
{
public:
    enum
    {
        fGrid = 0x01,
        fTransparency = 0x02,
        fRaster = 0x04,
    };
    explicit gceCanvas(wxWindow *parent, gceContext &ctx);
    virtual ~gceCanvas();

    bool getMousePosition(wxPoint &mousePosition) const;

    void CoordFromPoint(glm::dvec3 &cpt, const wxPoint &pt) const;
    void PointFromCoord(wxPoint &pt, const glm::dvec3 &cpt) const;

    wxImage getShot(const glm::ivec2 &sz, int samples);

    void PushCursor(const wxCursor &cursor);

    void PopCursor();

    void SetViewFlat(const glm::dvec2 &wgsPos, double scale);

    void SetViewGlobe(const glm::dvec2 &wgsPos, double height);

    glm::dvec3 calculateCursorAOI(int radiusPX) const;

    void ProjectInplace(geom::Geometry &g) const;
    void UnProjectInplace(geom::Geometry &g) const;
    double GetMapScale() const;

    void enableDragging(bool enable)
    {
        m_draggingEnabled = enable;
    }
    const gceProjection *getProj() const
    {
        return m_geoProj.get();
    }
    gceContext &ctx()
    {
        return m_ctx;
    }
    void postSceneParams();

    bool isFlat() const
    {
        return m_mode == gceRenderMode::FLAT;
    }

    bool isGlobe() const
    {
        return m_mode == gceRenderMode::GLOBE;
    }
    gceRenderMode getMode() const
    {
        return m_mode;
    }

    void setMode(gceRenderMode mode)
    {
        m_mode = mode;
    }

    void setTerrainWireFrame(bool wireFrameTerrain)
    {
        m_wireFrameTerrain = wireFrameTerrain;
    }
    //void 
private:


    void OnErase(wxEraseEvent &event);
    void OnSize(wxSizeEvent &event);
    void OnPaint(wxPaintEvent &event);
    void OnMotion(wxMouseEvent &event);
    void OnMouseWheel(wxMouseEvent &event);
    void OnLeaveWindow(wxMouseEvent &event);

    gceRenderMode m_mode = gceRenderMode::FLAT;

    std::stack<wxCursor> m_CursorStack;

    wxPoint m_mousePosition; //!< mouse cursor position in canvas coordinate system
    wxSize m_winSize; //!<  window size, logical pixels
    glm::dvec2 m_size{}; //!< dc size, meters
    glm::ivec4 m_viewport{};
    bool m_wireFrameTerrain = false;
    std::unique_ptr<wxGLContext> m_oglContext;
    bool m_draggingEnabled = false;
    gceContext &m_ctx;
    std::unique_ptr<gceProjection> m_geoProj;
    std::thread m_rt;
};

