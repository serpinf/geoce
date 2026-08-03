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

#include "wslayer.hpp"
#include "wsmodeltableinputcolumn.h"

gceModelTableInputColumnItem::gceModelTableInputColumnItem(gceWorkspaceItem *parent, const gceColumnSchema &schema, const gce::model_info &modelInfo, const gce::model_table_input_info &tableInfo)
    : gceWorkspaceItem(parent), m_schema(schema), m_info(modelInfo)
{}

wxIcon gceModelTableInputColumnItem::icon() const
{
    return iconFromSVG(m_schema.getTypeIcon());
}

void gceModelTableInputColumnItem::onMenu(gceWorkspaceItemActionParams par)
{}

const std::string gceModelTableInputColumnItem::name() const
{
    return m_schema.getLabel();
}

void gceModelTableInputColumnItem::processMsg(gceWorkspaceItemActionParams par, const gce::MessageInfo &msg)
{}

