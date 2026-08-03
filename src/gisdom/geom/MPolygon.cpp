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

#include <geom/MPolygon.h>
namespace geom
{

bool MPolygon::Create(const CoordinateSeq &outer)
{
    Clear();
    auto &pl0 = collection.emplace_back(std::make_unique<Polygon>(getCoordinateType()));
    return pl0->Create(outer);
}

Polygon *MPolygon::getGeometryN(size_t n)
{
    return collection.at(n)->isPolygon();
}
const Polygon *MPolygon::getGeometryN(size_t n) const
{
    return collection.at(n)->isPolygon();
}

int MPolygon::getDimension() const
{
    return 2;
}

gceGeometryType MPolygon::getGeometryTypeId() const
{
    return gceGeometryType::MultiPolygon;
}
};//namespace geom{
