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
#include "CoordSeq.h"

namespace geom
{

class Polygon final : public Geometry
{
public:

    explicit Polygon(CoordinateType coordType) : Geometry(coordType), m_shell(coordType)
    {}

    Polygon(const Polygon &p) = default;

    ~Polygon() = default;


    bool Create(const CoordinateSeq &seq) final;

    Polygon *isPolygon() final
    {
        return this;
    }

    const Polygon *isPolygon() const final
    {
        return this;
    }

    GEOSGeom_scoped_t toGEOSGeom() const final;

    void apply_geometry_filter(GeometryFilter &filter) const final
    {
        filter(*this);
    }

    void Clear() final;

    Polygon *clone() const final { return new Polygon(*this); }

    bool equals(const Geometry *geom) const final;

    //CoordSeq* getCoordinates() const;


    size_t getNumPoints() const final;


    int getDimension() const final
    {
        return 2;
    }

    bool isPointInPolygon(const glm::dvec2 &pos) const;

    double distance(const glm::dvec2 &pos) const final;

    bool isEmpty() const final
    {
        return m_shell.empty();
    }


    bool isSimple() const final;


    bool isValid() const final;


    bool CanMakeValid() const final;


    void MakeValid() final;

    gceGeometryType getGeometryTypeId() const final
    {
        return gceGeometryType::Polygon;
    }

    double area() const override;

    double length() const override;//perimeter

    void apply_filter_rw(filter_rw &filter) override;

    void apply_filter_ro(filter_ro &filter) const override;

    void removeVertex(size_t idx) override;

    void updateVertex(size_t idx, const Coordinate &coo) override;

    void insertVertex(size_t idx, const Coordinate &coo) override;

    std::pair<size_t, const Geometry *> getGeometryForVertex(size_t idx) const override;
    std::pair<size_t, const CoordinateSeq *> getRingForVertex(size_t idx) const;

    CoordinateSeq m_shell;
    std::vector<CoordinateSeq> m_holes;
private:
    std::pair<size_t, CoordinateSeq *> getRingForVertex(size_t idx);
};

}; // namespace geom
