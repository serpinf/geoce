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

#include "tools/tool.h"
#include "Canvas.h"
#include "tools/ToolOptions.h"
#include "editorfrm.h"
#include "models/selectionmodel.h"

void UICommandContainer::popupMenu(wxWindow *win, const wxString &name)
{
    wxMenu menu(name);
    for (auto &cmd : m_commands)
    {
        cmd.MenuItem(&menu, cmd.m_cmd);
    }
    win->PopupMenu(&menu);
}

const std::vector<wxAcceleratorEntry> &UICommandContainer::createAccelerators2()
{
    if (m_accCache.empty())
    {
        m_accCache.reserve(m_commands.size());
        for (auto &cmd : m_commands)
        {
            wxAcceleratorEntry parser;
            wxString astr = cmd.m_text.AfterFirst('\t');
            if (!astr.empty() && parser.FromString(astr))
            {
                m_accCache.emplace_back(parser.GetFlags(), parser.GetKeyCode(), cmd.m_cmd);
            }
        }

    }
    return m_accCache;
}

gceToolBase::gceToolBase(gceEditorFrame &owner) : m_owner(owner)
{
    this->Bind(wxEVT_RIGHT_UP, &gceToolBase::OnRightUp, this);
    this->Bind(wxEVT_MOTION, &gceToolBase::OnMotion, this);
    this->Bind(wxEVT_PAINT, &gceToolBase::OnPaint, this);
}

gceToolBase::~gceToolBase()
{
    delete m_optpanel;
}

void gceToolBase::OnPaint(wxPaintEvent &evt)
{
    //getCanvas()->RelaxOnPaint();
    DoPaint();
    evt.Skip();
}

void gceToolBase::DoPaint()
{
    wxPoint mousePosition;
    if (getCanvas()->getMousePosition(mousePosition))
    {
        //DoRecalc(mousePosition);
    }
    Display();
}


void gceToolBase::OnMotion(wxMouseEvent &event)
{
    // TODO: move update to separate method and call it on other tool model chages: hotkeys processing etc.
    DoRecalc(event.GetPosition());
    Display();
    //getCanvas()->calculateCursorAOI(5);

    updateToolString();
    event.Skip();
}

void gceToolBase::updateToolString()
{
    // compose help string
    const auto &info = GetInfo();
    wxString help = info.name;
    help << ": ";
    if (wxString stateStr = GetString(); !stateStr.empty())
    {
        help << stateStr;
    }
    else
    {
        help << info.help;
    }
    m_owner.setToolString(help);
}

bool gceToolBase::HaveMouse() const
{
    wxPoint pt;
    return getCanvas()->getMousePosition(pt);
}

void gceToolBase::updateOtionsPanel()
{
    if (m_optpanel)
    {
        m_optpanel->Update();
    }
}
wxString gceToolBase::getRecordsString() const
{
    // TODO: cache geometry metrics for selection: it may be costly to compute here
    wxString res;
    auto &sel = m_owner.getSelection();
    if (!sel.empty())
    {
        if (sel.IsSameType())
        {
            res << to_wxstring(sel.begin()->entity.get_schema()->getName()) << "; ";
        }
        else
        {
            res << "multiple types; ";
        }

        double perimeter = 0.0, area = 0.0;
        auto *proj = getCanvas()->getProj();
        for (auto &rec : sel)
        {
            if (auto g = rec.entity.get_geometry(); g)
            {
                double p = 0.0, a = 0.0;
                proj->computeMetrics(*g, p, a);
                perimeter += p;
                area += std::abs(a);
            }
        }
        res << wxString::Format("perimeter = %s, area = %s",
                                to_wxstring(gceProjection::formatLength(perimeter)),
                                to_wxstring(gceProjection::formatArea(area)));
    }
    return res;
}

void gceToolBase::postSelectionModelProps(bool drawPoints, const glm::dmat4 &pose, const std::vector<glm::dmat4> &poseArray)
{
    auto props = std::make_unique<gceSelectionModelProprties>();
    props->drawPoints = drawPoints;
    props->pose = pose;
    props->poseArray = poseArray;

    umodelUpdateMsg msg;
    msg.id_model = SELD_LAYER_UUID;
    msg.props = std::move(props);
    getCanvas()->ctx().postModelQueue(std::move(msg));
}
inline int getEffectiveKeyCode(const wxKeyEvent &evt)
{
    int code = evt.GetUnicodeKey();
    if (evt.GetKeyCode() == 0)
    {
        switch (code)
        {
        case L'ё':
            code = int('`');
            break;
        }
    }
    return code;
}
inline bool hasFlag(const wxAcceleratorEntry &acc, int flag)
{
    return (acc.GetFlags() & flag) != 0;
}
bool gceToolBase::processCommandKey(const wxKeyEvent &evt)
{
    for (auto &acc : uic.createAccelerators2())
    {
        if (hasFlag(acc, wxACCEL_CTRL) == evt.ControlDown() &&
            hasFlag(acc, wxACCEL_SHIFT) == evt.ShiftDown() &&
            hasFlag(acc, wxACCEL_ALT) == evt.AltDown() &&
            acc.GetKeyCode() == getEffectiveKeyCode(evt))
        {
            wxCommandEvent cmd(wxEVT_MENU, acc.GetCommand());
            this->ProcessEventLocally(cmd);
            return true;
        }
    }
    return false;
}

void gceToolBase::OnRightUp(wxMouseEvent &event)
{
    uic.popupMenu(getCanvas(), GetInfo().name);
    event.Skip();
}

void gceToolBase::BeginUse(gceToolBase *parentTool)
{
    // prevent using the tool twice
    if (GetUse()) return;

    m_parentTool = parentTool;

    enableParent(false);

    // push the tool as canvas's event handler
    getCanvas()->PushEventHandler(this);

    // try to get panel
    if (m_optpanel == nullptr)
    {
        m_optpanel = create_to(m_owner.GetToolOptParent());
    }
    // set options panel
    if (m_optpanel != nullptr)
    {
        m_optpanel->Update();
        m_owner.PushToolOptions(m_optpanel);
    }

    if (m_currentCursor.first >= 0)
    {
        getCanvas()->PushCursor(m_currentCursor.second);
    }

    m_fInUse = true;

    // now call tool-specific BeginUse
    BeginUse_Custom();
}

bool gceToolBase::EndUse()
{
    // check if BeginUse was called
    if (!GetUse()) return true;

    if (EndUse_Custom())
    {
        // remove the tool options panel
        if (m_optpanel != nullptr)
        {
            m_owner.PopToolOptions();
        }

        // remove handler
        getCanvas()->PopEventHandler();

        enableParent(true);

        if (m_currentCursor.first >= 0)
        {
            // restore cursor
            getCanvas()->PopCursor();
        }

        m_fInUse = false;
        m_parentTool = nullptr;
        return true;
    }
    return false;
}
void gceToolBase::finishChild()
{
    if (m_childTool && m_childTool->EndUse())
    {
        m_childTool.reset();
    }
}
void gceToolBase::enableParent(bool enable)
{
    if (gceToolBase *parent = getParent(); parent)
    {
        parent->SetEnabled(enable);
        parent->CallAfter(&gceToolBase::finishChild);
    }
}
gceToolBase *gceToolBase::getParent() const
{
    return m_parentTool;
}

bool gceToolBase::GetUse() const
{
    return m_fInUse;
}

bool gceToolBase::IsActive()
{
    return (m_owner.GetCurrentTool() == this);
}
gceToolOptions *gceToolBase::create_to(wxWindow *)
{
    return nullptr;
}

void gceToolBase::SetEnabled(bool enabled)
{
    SetEvtHandlerEnabled(enabled);
    if (enabled)
    {
        OnSetEnabled();
    }
}
wxString gceToolBase::GetString() const
{
    return wxString();
}

void gceToolBase::setBaseLine(double, bool) {}

bool gceToolBase::isInputTool() const
{
    return false;
}

void gceToolBase::processSelectResult(const umodelSelectXDResultMsg &)
{}

void gceToolBase::processSelectDataResult(const udataSelectReplyMsg &)
{}

void gceToolBase::processActionNotify(const udataMultiRowActionNotifyMsg &)
{}

wxString gceToolBase::describeGeometry(const geom::Geometry &g) const
{
    std::string info = fmt::format("points: {}; ", g.getNumPoints());
    double perimeter = 0.0, area = 0.0;
    getCanvas()->getProj()->computeMetrics(g, perimeter, area);
    if (auto dimension = g.getDimension(); dimension == 2)
    {
        info += fmt::format("area = {}; perimeter = {}; ", gceProjection::formatArea(area), gceProjection::formatLength(perimeter));
    }
    else if (dimension == 1)
    {
        info += fmt::format("length = {}; ", gceProjection::formatLength(perimeter));
    }
    return to_wxstring(info);
}

gceToolInfo gceToolBase::GetInfo() const
{
    return gceToolInfo{"gceToolBase", nullptr, "this is base tool"};
}


bool gceToolBase::isAvailable()
{
    return true;
}

bool gceToolBase::DoRecalc(const wxPoint & /*mousePosition*/)
{
    return false;
}

void gceToolBase::Display()
{}

void gceToolBase::SetCursor(const std::pair<int, wxCursor> &cursor)
{
    if (m_currentCursor.first != cursor.first)
    {
        if (GetUse())
        {
            if (m_currentCursor.first < 0)
            {
                // first time we set cursor for this tool: use push to save original
                getCanvas()->PushCursor(cursor.second);
            }
            else
            {
                // simple cursor switch
                getCanvas()->SetCursor(cursor.second);
            }
        }
        m_currentCursor = cursor;
    }
}

std::pair<int, wxCursor> gceToolBase::makeCursor(int cursorID, wxImage img, int x, int y)
{
    img.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_X, x);
    img.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_Y, y);
    wxSize newSize = m_owner.FromDIP(wxSize(img.GetWidth(), img.GetHeight()));
    return std::make_pair(cursorID, wxCursor(img.Rescale(newSize)));
}

void gceToolBase::ProjectInplace(geom::Geometry &g) const
{
    getCanvas()->ProjectInplace(g);
}

void gceToolBase::UnProjectInplace(geom::Geometry &g) const
{
    getCanvas()->UnProjectInplace(g);
}

void gceToolBase::OnSetEnabled()
{}

gceCanvas *gceToolBase::getCanvas() const
{
    return m_owner.getCanvas();
}
