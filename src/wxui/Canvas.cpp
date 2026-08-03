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

#include "Canvas.h"
#include "teches/gl_fbo.h"
#include "editorfrm.h"
#include "geom/Geometry.h"
#include "gisdom/node/urendernode.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include "alg/wgsop.h"
#include "geom/aabb.h"

namespace
{
wxGLAttributes getAttrsGL()
{
    wxGLAttributes vAttrs;
    // Defaults should be accepted
    vAttrs.PlatformDefaults().Defaults().EndList();
    bool accepted = wxGLCanvas::IsDisplaySupported(vAttrs);

    if (!accepted)
    {
        // Try again without sample buffers
        vAttrs.Reset();
        vAttrs.PlatformDefaults().RGBA().DoubleBuffer().Depth(16).EndList();
        accepted = wxGLCanvas::IsDisplaySupported(vAttrs);
    }

    if (!accepted)
        throw std::runtime_error("Failed to find wxGLAttributes");

    return vAttrs;
}
std::string getDescriptionGL()
{
    return fmt::format("{} {} OpenGL {} GLSL {}", (const char *)glGetString(GL_VENDOR), (const char *)glGetString(GL_RENDERER), (const char *)glGetString(GL_VERSION),
        (const char *)glGetString(GL_SHADING_LANGUAGE_VERSION));
}
}

class Camera
{
public:
    glm::dvec2 unProject(const glm::ivec2 &coo, const glm::ivec4 &viewport) const
    {
        return glm::dvec2(glm::unProject({coo.x, coo.y, 0.0}, glm::dmat4(1.0), m_projMat, viewport));
    }

    glm::dvec2 unProject(const glm::ivec2 &coo, const glm::ivec4 &viewport, int winHeight) const
    {
        return unProject({coo.x, winHeight - coo.y - 1}, viewport);
    }

    wxPoint project(const glm::dvec3 coo, const glm::ivec4 viewport) const
    {
        glm::dvec3 win = glm::project(coo, glm::dmat4(1.0), m_projMat, viewport);
        return wxPoint((int)lround(win.x), (int)lround(viewport[3] - win.y - 1.0));
    }
    glm::ivec2 project(const glm::dvec2 &coo, const glm::ivec4 &viewport) const
    {
        auto pt = project(glm::dvec3(coo, 0.0), viewport);
        return {pt.x, pt.y};
    }

    const glm::dmat4 &getProj() const
    {
        return m_projMat;
    }
protected:
    glm::dmat4 m_projMat = glm::identity<glm::dmat4>();
};

class Camera2D : public Camera
{
public:
    void updateMatrix(const glm::dvec2 &size)
    {
        glm::dvec2 halfsize = 0.5 * size / scale;
        glm::dvec2 pmin = cen - halfsize;
        glm::dvec2 pmax = cen + halfsize;
        m_projMat = glm::ortho(pmin.x, pmax.x, pmin.y, pmax.y, -0.1, 1.0);
    }
    geom::aabb2 getBox(const glm::dvec2 &size) const
    {
        return {cen, 0.5 * size / scale};
    }

    glm::dvec3 calculateAOI(int radiusPX, double sizeY, const glm::ivec2 &coo, const glm::ivec4 &viewport, int winHeight) const
    {
        glm::dvec2 pos = this->unProject(coo, viewport, winHeight);
        double radius = (sizeY * radiusPX) / (winHeight * scale);
        return {pos, radius};
    }

    double scale = {1.0};
    glm::dvec2 cen = {0.0, 0.0};
};

class Camera3D : public Camera
{
public:
    void updateMatrix(const glm::dvec2 &size)
    {
        const double _near = 0.001;
        glm::dmat4 M = glm::infinitePerspective(glm::radians<double>(fovy), size.x / size.y, _near);
        const glm::dvec3 as = pos - basePoint;
        m_projMat = M * glm::lookAt(as, as + front, up);
    }
    glm::dvec3 basePoint{0.0};// { 10e+6, 10e+6, 0.0 };
    glm::dvec3 pos{6e+6, 6e+6, 0.0};
    glm::dvec3 front{-1.0, -1.0, 0.0};
    glm::dvec3 up{0.0, 0.0, 1.0};
    float fovy = 60.0; // vertical fov (degrees)

};

class gceCamera3DController1
{
public:
    void updateCamera(Camera3D &cam)
    {
        wgsop::Location place(glm::radians(m_wgsPos.x), glm::radians(m_wgsPos.y), m_wgsPos.z);

        cam.pos = place.getPosition();
        cam.front = place.getRotation() * m_localFront;
        cam.up = place.getRotation() * m_localUp;
    }
    glm::dvec3 m_localFront = {0.0, .001, -1.0};
    glm::dvec3 m_localUp = {0.0, 0.0, 1.0};
    glm::dvec3 m_wgsPos = {45.0, 45.0, 2000000.0};
};

static Camera2D cam2d;
static Camera3D cam3d;

static gceCamera3DController1 cam3dCtrl1;


gceCanvas::gceCanvas(wxWindow *parent, gceContext &ctx)
    : wxGLCanvas(parent, getAttrsGL(), wxID_ANY, wxDefaultPosition, wxDefaultSize, wxCLIP_CHILDREN | wxWANTS_CHARS),
    m_ctx(ctx)
    //m_MGeom(owner.getSelection()),
{
    m_geoProj = gceProjection::create(gceProjectionType::WGS84_VISUALIZATION, {});
    //m_render = gisdom_service_create(gce::urender_worker2, ctx, "rendersrv");
    SetCursor(wxCursor(wxCURSOR_CROSS));

    // Explicitly create a new rendering context instance for this canvas.
    wxGLContextAttrs ctxAttrs;
    ctxAttrs.PlatformDefaults().CoreProfile().OGLVersion(4, 6).EndList();
    m_oglContext = std::make_unique<wxGLContext>(this, nullptr, &ctxAttrs);

    if (m_oglContext && !m_oglContext->IsOK())
    {
        wxMessageBox("This app needs an OpenGL 4.6 capable driver.",
                     "OpenGL version error", wxOK | wxICON_ERROR, this);
        m_oglContext.reset();
    }

    // tool layer tech
    {
        urenderReplaceTechMsg msg;
        msg.techFlat = gceTechType::BASIC_RTE2;
        msg.techGlobe = gceTechType::NONE;
        msg.id_layer = TOOL_LAYER_UUID;
        msg.id_model = TOOL_LAYER_UUID;
        msg.order = -10;
        msg.visible = true;
        ctx.postRenderQueue(msg);
    }

    // selection layer model/tech
    {
        urenderReplaceTechMsg msg;
        msg.techFlat = gceTechType::BASIC;
        msg.techGlobe = gceTechType::NONE;
        msg.id_layer = SELD_LAYER_UUID;
        msg.id_model = SELD_LAYER_UUID;
        msg.order = -9;
        msg.visible = true;
        ctx.postRenderQueue(msg);
    }

    {
        umodelReplaceMsg msg;
        msg.info.type = gceModelType_SELECTION;
        msg.info.id_model = SELD_LAYER_UUID;
        ctx.postModelQueue(msg);
    }

    /* events */
    wxEvtHandler::Bind(wxEVT_SIZE, &gceCanvas::OnSize, this);
    wxEvtHandler::Bind(wxEVT_PAINT, &gceCanvas::OnPaint, this);
    wxEvtHandler::Bind(wxEVT_MOTION, &gceCanvas::OnMotion, this);
    wxEvtHandler::Bind(wxEVT_MOUSEWHEEL, &gceCanvas::OnMouseWheel, this);
    wxEvtHandler::Bind(wxEVT_LEAVE_WINDOW, &gceCanvas::OnLeaveWindow, this);
    wxEvtHandler::Bind(wxEVT_ERASE_BACKGROUND, &gceCanvas::OnErase, this);
}


gceCanvas::~gceCanvas()
{
    if (m_rt.joinable())
    {
        m_ctx.postRenderQueue(gceQuitMessage{});
        m_rt.join();
    }
}

void gceCanvas::OnErase(wxEraseEvent &)
{}

void gceCanvas::OnSize(wxSizeEvent &event)
{
    event.Skip();

    // If this window is not fully initialized, dismiss this event
    if (!IsShownOnScreen())
        return;

    if (!m_oglContext)
        return;

    if (!m_rt.joinable())
    {
        m_rt = std::thread([this](){
            SetCurrent(*m_oglContext);
            gceContext::log_message("{}", getDescriptionGL());
            gce::urender_worker(&m_ctx, this);
        });
    }

    m_winSize = event.GetSize();
    wxClientDC dc(this);
    const wxSize size = dc.GetSize();
    const wxSize ppi = this->GetDPI();

    m_size = 0.0254 * glm::dvec2(size.x, size.y) / glm::dvec2(ppi.x, ppi.y);

    const auto s = this->GetContentScaleFactor();
    m_viewport = {0, 0, s * size.x, s * size.y};

    postSceneParams();
}

double gceCanvas::GetMapScale() const
{
    double projScaleAt = 1.0;
    if (m_geoProj)
    {
        projScaleAt = m_geoProj->getScaleFromProjected(cam2d.cen);
    }
    return projScaleAt / cam2d.scale;
}

void gceCanvas::OnPaint(wxPaintEvent &)
{
    wxPaintDC dc(this);
    //this->postSceneParams();
}

void gceCanvas::OnLeaveWindow(wxMouseEvent &)
{
    m_mousePosition = wxDefaultPosition;
}

void gceCanvas::OnMotion(wxMouseEvent &event)
{
    if (m_draggingEnabled && event.LeftIsDown() && m_mousePosition != wxDefaultPosition)
    {
        if (m_mode == gceRenderMode::FLAT)
        {
            auto pt1 = cam2d.unProject({m_mousePosition.x, m_mousePosition.y}, m_viewport, m_winSize.y);
            auto pt2 = cam2d.unProject({event.GetX(), event.GetY()}, m_viewport, m_winSize.y);
            cam2d.cen += pt1 - pt2;
        }
        else // if (m_mode == gceRenderMode::GLOBE)
        {
            const double actX = double(m_mousePosition.x - event.GetX()) / m_winSize.y;
            const double actY = double(event.GetY() - m_mousePosition.y) / m_winSize.y;
            if (event.ControlDown())
            {
                double ax = gce::angleX(cam3dCtrl1.m_localFront);
                double az = gce::angleZ(cam3dCtrl1.m_localFront);

                ax -= 0.7 * actX;
                az = std::clamp(az + 0.7 * actY, -1.57, 0.0);

                const double cos_az = std::cos(az);
                cam3dCtrl1.m_localFront.x = cos(ax) * cos_az;
                cam3dCtrl1.m_localFront.y = sin(ax) * cos_az;
                cam3dCtrl1.m_localFront.z = sin(az);
            }
            else
            {
                const glm::dvec2 dir = glm::normalize(glm::dvec2(cam3dCtrl1.m_localFront));
                double dirActX = glm::dot(dir, glm::dvec2(actY, actX));
                double dirActY = glm::dot(dir, glm::dvec2(-actX, actY));

                double surfDist = cam3dCtrl1.m_wgsPos.z; // TODO: normally needs to use terrain info: distance to nearest visible terrain element maybe?
                // estimate the width of visible Globe area in degrees based on surface distance and camera FOV
                double scrSurfDeg = glm::degrees(2.0 * surfDist / wgsop::Ra * std::tan(glm::radians(0.5 * cam3d.fovy)));
                cam3dCtrl1.m_wgsPos.x += scrSurfDeg / cos(glm::radians(cam3dCtrl1.m_wgsPos.y)) * dirActX;
                cam3dCtrl1.m_wgsPos.y += scrSurfDeg * dirActY;
            }

            cam3dCtrl1.updateCamera(cam3d);
            cam3d.updateMatrix(m_size);
        }
    }
    m_mousePosition = event.GetPosition();
}

void gceCanvas::OnMouseWheel(wxMouseEvent &event)
{
    if (m_mode == gceRenderMode::FLAT)
    {
        auto pt_coord = cam2d.unProject({event.GetX(), event.GetY()}, m_viewport, m_winSize.y); // cursor coordinates
        double s = pow(1.333, float(event.GetWheelRotation()) / event.GetWheelDelta());

        double newScale = glm::clamp(cam2d.scale * s, 0.03, 1.0e+7);

        // move camera to cursor coordinates, scale and move back
        cam2d.cen = (cam2d.cen - pt_coord) * cam2d.scale / newScale + pt_coord;
        cam2d.scale = newScale;

        cam2d.updateMatrix(m_size);
    }
    else // if (m_mode == gceRenderMode::GLOBE)
    {
        glm::dvec3 wgs = cam3dCtrl1.m_wgsPos;
        double s = 1.0 + 0.03 * fabs(wgs.z);

        double dz = s * event.GetWheelRotation() / event.GetWheelDelta();
        // move faster if we look horizontally
        //dz /= std::max(0.2, cos(glutI.s.psi));
        cam3dCtrl1.m_wgsPos.z -= dz;
        cam3dCtrl1.updateCamera(cam3d);
        cam3d.updateMatrix(m_size);
        //glutI.move(dz, 0.5);

    }
}

void gceCanvas::PushCursor(const wxCursor &cursor)
{
    m_CursorStack.push(this->GetCursor());
    this->SetCursor(cursor);
}

void gceCanvas::PopCursor()
{
    if (!m_CursorStack.empty())
    {
        this->SetCursor(m_CursorStack.top());
        m_CursorStack.pop();
    }
}

bool gceCanvas::getMousePosition(wxPoint &mousePosition) const
{
    mousePosition = m_mousePosition;
    return (m_mousePosition != wxDefaultPosition);
}

wxImage gceCanvas::getShot(const glm::ivec2 &sz, int samples)
{
    wxImage img;

    //try
    {
        // create fbo
        gl_fbo::FrameBuffer fboFinal(sz, 0);
        gl_fbo::FBOLayerRenderBuffer rbFinal(GL_RGBA, sz, 0);

        fboFinal.Begin();
        fboFinal.Attach(&rbFinal, GL_COLOR_ATTACHMENT0);

        img = fboFinal.ResolveToImage(GL_RGB);
        fboFinal.End();
    }
    //catch (const gl::Exception& e)
    {
        //wxLogMessage("Canvas::getShot: %s", e.error);
    }
    return img;
}

void gceCanvas::SetViewFlat(const glm::dvec2 &wgsPos, double scale)
{
    glm::dvec2 cen;
    if (m_geoProj && m_geoProj->fromInternal(cen, wgsPos))
    {
        cam2d.cen = cen;
        cam2d.scale = scale * m_geoProj->getScale(wgsPos);
        cam2d.updateMatrix(m_size);
    }

}
void gceCanvas::SetViewGlobe(const glm::dvec2 &wgsPos, double height)
{
    cam3dCtrl1.m_localFront = {0.0, .001, -1.0};
    cam3dCtrl1.m_localUp = {0.0, 0.0, 1.0};
    cam3dCtrl1.m_wgsPos = {wgsPos, height};
    cam3dCtrl1.updateCamera(cam3d);

}
glm::dvec3 gceCanvas::calculateCursorAOI(int radiusPX) const
{
    return cam2d.calculateAOI(radiusPX, m_size.y, {m_mousePosition.x, m_mousePosition.y}, m_viewport, m_winSize.y);
}

void gceCanvas::postSceneParams()
{
    if (this->isFlat())
    {
        urenderDrawFrameMsg msg;
        cam2d.updateMatrix(m_size);
        msg.aoi = cam2d.getBox(m_size);
        msg.proj = cam2d.getProj();
        msg.viewport = m_viewport;
        //msg.sender = gce::queueId::WORKSPACE;
        m_ctx.postRenderQueue(msg);
    }
    else //if (this->isGlobe())
    {
        urenderDrawFrame3DMsg msg;
        cam3dCtrl1.updateCamera(cam3d);
        cam3d.updateMatrix(m_size);
        msg.basePoint = cam3d.basePoint;
        msg.pos = cam3d.pos;
        msg.fovy = cam3d.fovy;
        msg.front = cam3d.front;
        msg.up = cam3d.up;
        msg.viewport = m_viewport;
        msg.wireFrameTerrain = m_wireFrameTerrain;
        //msg.sender = gce::queueId::WORKSPACE;
        m_ctx.postRenderQueue(msg);
    }
}

void gceCanvas::CoordFromPoint(glm::dvec3 &cpt, const wxPoint &pt) const
{
    cpt = glm::dvec3(cam2d.unProject({pt.x, pt.y}, m_viewport, m_winSize.y), 0.0);
    m_geoProj->toInternal(cpt, glm::dvec3(cam2d.unProject({pt.x, pt.y}, m_viewport, m_winSize.y), 0.0));
}

void gceCanvas::PointFromCoord(wxPoint &pt, const glm::dvec3 &cpt) const
{
    //const dvec3 obj(cpt.x+m_transform.deltaX, cpt.y+m_transform.deltaY, 0.0/*cpt.z+m_Carta.deltaZ*/);
    //dvec3 win = glm::project(obj, m_modelviewMatrix, m_projectionMatrix, m_viewport);
    //pt.x = (int)lround(win.x);
    //pt.y = (int)lround(m_viewport[3]-win.y-1.0);
}

void gceCanvas::ProjectInplace(geom::Geometry &g) const
{
    project_filter f(m_geoProj.get());
    g.apply_filter_rw(f);
}

//void gceCanvas::ProjectInplace(geom::CoordSeq &seq) const
//{
    //project_filter f(m_geoProj.get());
    //seq.apply_filter_rw(f);
//}
void gceCanvas::UnProjectInplace(geom::Geometry &g) const
{
    unproject_filter f(m_geoProj.get());
    g.apply_filter_rw(f);
}

//void gceCanvas::UnProjectInplace(geom::CoordSeq &seq) const
//{
//    unproject_filter f(m_geoProj.get());
//    seq.apply_filter_rw(f);
//}

