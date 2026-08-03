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

#include <vector>
#include "Geometry.h"

namespace geom
{
class MGeometry : public Geometry
{
public:

    explicit MGeometry(CoordinateType coordType) : Geometry(coordType)
    {}
    explicit MGeometry(const MGeometry &other);

    virtual ~MGeometry();

    MGeometry *isMGeometry() final
    {
        return this;
    }

    const MGeometry *isMGeometry() const final
    {
        return this;
    }

    bool isValid() const override;


    bool CanMakeValid() const override;


    void MakeValid() override;


    GEOSGeom_scoped_t toGEOSGeom() const override;


    void Clear() override
    {
        collection.clear();
    }

    bool equals(const Geometry *geom) const override;

    MGeometry *clone() const override { return new MGeometry(*this); }

    bool isEmpty() const override;

    int getDimension() const override;

    double distance(const glm::dvec2 &pos) const override;

    size_t getNumPoints() const override;

    gceGeometryType getGeometryTypeId() const override;


    bool isSimple() const override;


    double area() const override;


    double length() const override;


    double length3d() const override;

    size_t getNumGeometries() const final;

    virtual Geometry *getGeometryN(size_t n);
    virtual const Geometry *getGeometryN(size_t n) const;

    void apply_geometry_filter(GeometryFilter &filter) const final;

    void apply_filter_rw(filter_rw &filter) final;

    void apply_filter_ro(filter_ro &filter) const final;

    std::pair<size_t, const Geometry *> getGeometryForVertex(size_t idx) const final;

    void removeVertex(size_t idx) override;

    void updateVertex(size_t idx, const Coordinate &coo) override;

    void insertVertex(size_t idx, const Coordinate &coo) override;

    size_t size() const
    {
        return collection.size();
    }

    auto begin() const
    {
        return collection.begin();
    }

    auto end() const
    {
        return collection.end();
    }
    std::vector<std::unique_ptr<Geometry>> collection;
private:
    /*!
     * @brief top level collection element for vertex with index idx,
     *
     * @param idx vertex index in geometry
     * @return vertex index realtive to returned geometry (always zero for Points), pointer to geometry
     */
    std::pair<size_t, Geometry *> getCollectionElementForVertex(size_t idx);

};

} // namespace geom
