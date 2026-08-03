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

#include "HandTool.h"
#include "Canvas.h"

/* XPM */
static const char *hand_xpm[] = {
    "16 16 6 1",
    " 	c None",
    ".	c #000000",
    "+	c #858585",
    "@	c #FFFFFF",
    "#	c #C3C3C3",
    "$	c #808080",
    "                ",
    "      ....      ",
    "   ...+@.@.     ",
    "   .@.+@.@..    ",
    "   .@++@.@.@.   ",
    "   .@#.@.@.@.   ",
    "   +@#.@$@$@.   ",
    "...++@$@$@$@.   ",
    ".@.+.@@@@@@@.   ",
    ".@@..@@@@@@@.   ",
    " .@@@@@@@@@@.   ",
    "  .@@@@@@@@@.   ",
    "  .@@@@@@@@@.   ",
    "   .@@@@@@@.    ",
    "    .@@@@@@.    ",
    "    .@@@@@@.    "};

/* XPM */
static const char *hand1_xpm[] = {
    "16 16 5 1",
    " 	c None",
    ".	c #000000",
    "+	c #858585",
    "@	c #808080",
    "#	c #FFFFFF",
    "                ",
    "                ",
    "                ",
    "     .....      ",
    "   ++@#@#@.     ",
    "   .##.#@#@.    ",
    "  ..##.#@#@#.   ",
    " .#.+#@#@#@#.   ",
    " .#++#######.   ",
    " .##.#######.   ",
    "  .#########.   ",
    "  .#########.   ",
    "  .#########.   ",
    "   .#######.    ",
    "    .######.    ",
    "    .######.    "};

static constexpr char hand_svg[] = R"rawsvg(<symbol viewBox="0 0 2000 2000" id="hand-tool"><title>hand-tool</title><path fill="none" stroke="currentColor" stroke-width="67.5" d="M806.673 1750.205h621.961c22.031-70.25 62.033-342.206 146.35-560.816c109.703-284.426 222.535-533.47 79.188-558.11c-114.053-22.16-164.268 222.17-239.25 378.398c0 0-.735-152.653-1.608-319.073c-.925-176.455 20.91-388.517-71.236-381.548c-95.054-6.969-102.434 176.632-127.533 313.704C1187.657 769.598 1163 921.667 1163 921.667s-25.608-129.884-43.734-309.888c-16.45-163.37-23.671-382.574-120.066-378.476c-114.205-4.098-91.583 212.301-89.508 386.42c1.627 136.477-3.108 300.727-3.108 300.727s-61.033-149.246-92.487-232.773c-62.058-160.334-116.378-320.83-230.62-269.78c-101.186 47.595-9.532 225.224 39.893 407.56c43.362 159.965 86.72 332.892 86.72 332.892s-293.095-367.544-429.6-246.644c-120.896 113.1 66.75 220.16 245.33 434.345c101.267 121.459 208.574 310.194 280.852 404.155z"></path></symbol>)rawsvg";

gceHandTool::gceHandTool(gceEditorFrame &owner) : gceToolBase(owner)
{
    m_cursor1 = makeCursor(0, hand_xpm, 9, 9);
    m_cursor2 = makeCursor(1, hand1_xpm, 9, 9);
    SetCursor(m_cursor1);

    Bind(wxEVT_LEFT_DOWN, &gceHandTool::OnMouseEvent, this);
    Bind(wxEVT_LEFT_UP, &gceHandTool::OnMouseEvent, this);
}

gceToolInfo gceHandTool::GetInfo() const
{
    return gceToolInfo{"Hand", hand_svg, "Dragging"};
}

void gceHandTool::BeginUse_Custom()
{
    getCanvas()->enableDragging(true);
}

bool gceHandTool::EndUse_Custom()
{
    getCanvas()->enableDragging(false);
    return true;
}
void gceHandTool::OnMouseEvent(wxMouseEvent &event)
{
    SetCursor(event.LeftIsDown() ? m_cursor2 : m_cursor1);
    event.Skip();
}
