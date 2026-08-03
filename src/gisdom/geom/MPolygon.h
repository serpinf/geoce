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

#include "geom/MGeometry.h"
#include "geom/Polygon.h"

namespace geom
{
class MPolygon final : public MGeometry
{
public:
    MPolygon(const MPolygon &geom) : MGeometry(geom)
    {}

    explicit MPolygon(CoordinateType coordType) : MGeometry(coordType) {}

    virtual ~MPolygon() = default;

    bool Create(const CoordinateSeq &seq) override;

    MPolygon *isMPolygon() override
    {
        return this;
    }
    const MPolygon *isMPolygon() const override
    {
        return this;
    }

    Polygon *getGeometryN(size_t n) override;
    const Polygon *getGeometryN(size_t n) const override;

    MPolygon *clone() const override { return new MPolygon(*this); }

    int getDimension() const override;

    gceGeometryType getGeometryTypeId() const override;
};

}; // namespace geom
