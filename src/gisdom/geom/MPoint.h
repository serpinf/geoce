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

#include "MGeometry.h"

namespace geom
{

class MPoint final : public MGeometry
{
public:

    explicit MPoint(const MPoint &other) : MGeometry(other)
    {}

    explicit MPoint(CoordinateType coordType) : MGeometry(coordType) {}

    virtual ~MPoint() {}

    MPoint *clone() const override { return new MPoint(*this); }

    bool Create(const CoordinateSeq &seq) override;

    MPoint *isMPoint() override
    {
        return this;
    }
    const MPoint *isMPoint() const override
    {
        return this;
    }

    int getDimension() const override
    {
        return 0;
    }

    gceGeometryType getGeometryTypeId() const override
    {
        return gceGeometryType::MultiPoint;
    }
};

}; // namespace geom
