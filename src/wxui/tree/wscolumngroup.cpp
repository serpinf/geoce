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
#include "wscolumngroup.h"
#include "wscolumn.h"
#include "type/typeschema.hpp"

static constexpr char columns_svg[] = R"rawsvg(<svg xmlns="http://www.w3.org/2000/svg" width="512" height="512" viewBox="0 0 512 512"><title>Table-columns SVG Icon</title><path fill="currentColor" fill-rule="evenodd" d="M64 213.334h64v192H64zm0-106.667h64v64H64zm213.333 106.667h64v192h-64zm0-106.667h64v64h-64zM170.667 213.334h64v192h-64zm0-106.667h64v64h-64zM384 213.334h64v192h-64zm0-106.667h64v64h-64z"/></svg>)rawsvg";

gceColumnGroup::gceColumnGroup(gceWorkspaceItem *parent, const gceTypeSchema *schema) : gceWorkspaceItem(parent)
{
    if (schema != nullptr)
    {
        const auto &columns = schema->getColumns();
        m_columns.reserve(columns.size());
        for (auto &col : columns)
        {
            m_columns.emplace_back(std::make_unique<gceColumnItem>(this, &col));
        }
    }
}
gceColumnGroup::~gceColumnGroup()
{

}
unsigned int gceColumnGroup::getChildren(wxDataViewItemArray &array) const
{
    array.reserve(m_columns.size());
    for (const auto &item : m_columns)
    {
        array.Add(wxDataViewItem(item.get()));
    }
    return static_cast<unsigned>(m_columns.size());
}


wxIcon gceColumnGroup::icon() const
{
    return iconFromSVG(columns_svg);
}
