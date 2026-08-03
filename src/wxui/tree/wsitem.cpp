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
#include "wsitem.h"
#include <boost/uuid/uuid_generators.hpp>


bool gceWorkspaceItem::update(gceWorkspace &, bool, const std::string &)
{
    return false;
}

bool gceWorkspaceItem::GetAttr(gceWorkspace &, wxDataViewItemAttr &) const
{
    return false;
}

wxIcon gceWorkspaceItem::iconFromSVG(const char *data)
{
    auto size = wxArtProvider::GetDIPSizeHint(wxART_MENU);
    auto size2 = wxArtProvider::GetSizeHint(wxART_MENU);
    return wxBitmapBundle::FromSVG(data, size).GetIcon(size2);
}

gce::uuid gceWorkspaceItem::makeUUID()
{
    return boost::uuids::random_generator()();
}
