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

#include <geom/LineString.h>
#include <geom/MLineString.h>
namespace geom
{

MLineString::~MLineString()
{}
bool MLineString::Create(const CoordinateSeq &cseq)
{
    Clear();
    auto &ls = collection.emplace_back(std::make_unique<LineString>(getCoordinateType()));
    return ls->Create(cseq);
}
bool MLineString::isClosed() const
{
    if (isEmpty())
    {
        return false;
    }

    for (auto &_val : *this)
    {
        if (!(static_cast<LineString *>(_val.get())->isClosed()))
        {
            return false;
        }
    }
    return true;
}

};//namespace geom{
