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
#include "entityset.h"
#include "node/umodelnode.h"
#include "node/udatanode.hpp"
#include "alg/geoproj.h"
#include "geom/LineString.h"
#include "geom/Polygon.h"

namespace
{
class ClosestVertexFinder : public geom::filter_ro
{
public:
    ClosestVertexFinder(const glm::dvec3 &pos, const gceProjection *proj) : pos(pos), proj(proj)
    {

    }

    void operator()(const geom::Coordinate &coo) override
    {
        glm::dvec3 cpos;
        proj->fromInternal(cpos, coo.pos);
        double pdist = glm::distance(pos, cpos);
        if (pdist < this->resultDist)
        {
            this->resultDist = pdist;
            this->resultVertexIndex = this->currentVertexIndex;
        }
        ++this->currentVertexIndex;
    }

    double getDistance() const
    {
        return this->resultDist;
    }

    size_t getVertexIndex() const
    {
        return this->resultVertexIndex;
    }
private:
    size_t currentVertexIndex = 0;
    size_t resultVertexIndex = 0;
    double resultDist = DoubleInfinity;
    const glm::dvec3 pos;
    const gceProjection *proj;
};

struct ClosestSegmentFinder final : public  geom::GeometryFilter
{
    explicit ClosestSegmentFinder(const glm::dvec3 &pos, const gceProjection *proj) : pos(pos), proj(proj) {}

    void operator()(const geom::Point &) final { ++currentVertexIndex; }

    void operator()(const geom::LineString &g) final
    {
        process(g.getCoordSeq());
    }

    void operator()(const geom::Polygon &poly) final
    {
        process(poly.m_shell);
        for (auto &ring : poly.m_holes)
        {
            process(ring);
        }
    }

    void process(const geom::CoordinateSeq &seq)
    {
        glm::dvec3 prev;
        geom::CoordinateXYZ coo;
        for (size_t n = 0, N = seq.size(); n < N; ++n)
        {
            if (n > 0)
            {
                glm::dvec3 cpos;
                seq.get(coo, n);
                proj->fromInternal(cpos, coo.pos);
                process_edge(prev, cpos);
                prev = cpos;
            }
            else
            {
                seq.get(coo, n);
                proj->fromInternal(prev, coo.pos);
            }
            ++this->currentVertexIndex;
        }
    }

    void process_edge(const glm::dvec3 &A, const glm::dvec3 &B)
    {
        // TODO: use distancePointToSegment2
        // move CS to vertex A
        const glm::dvec3 AB = B - A, AP = pos - A;
        // find clamped projection factor
        double r = glm::clamp(glm::dot(AP, AB) / glm::dot(AB, AB), 0.0, 1.0);
        // find distance to projection
        double pdist = glm::distance(AP, r * AB);
        if (pdist < this->resultDist)
        {
            this->resultDist = pdist;
            this->resultR = r;
            this->resultVertexIndex = this->currentVertexIndex;
        }
    }

    double getDistance() const
    {
        return this->resultDist;
    }
    double getR() const
    {
        return this->resultR;
    }
    size_t getVertexIndex() const
    {
        return this->resultVertexIndex;
    }
private:
    size_t currentVertexIndex = 0;
    size_t resultVertexIndex = 0;
    double resultR = 0.0;
    double resultDist = DoubleInfinity;
    const glm::dvec3 pos;
    const gceProjection *proj;
};

}

template <typename Iterator, typename _Op>
inline bool all_same(Iterator first, Iterator last, _Op op)
{
    if (first != last)
    {
        auto start = first;
        while (++first != last && op(*start, *first)) {}
    }
    return first == last;
}

static inline bool operator < (const gceEntityPackedRef &fk, const gceEntitySetKey &lk)
{
    return gceEntitySetKey{fk.id_table, fk.entity.get_pkey()} < lk;
}

static inline bool operator < (const gceEntitySetKey &lk, const gceEntityPackedRef &fk)
{
    return lk < gceEntitySetKey{fk.id_table, fk.entity.get_pkey()};
}

bool gceEntityPackedRef_set::IsSameLayer() const
{
    return all_same(begin(), end(), [](const gceEntityPackedRef &A, const gceEntityPackedRef &B){
        return A.id_table == B.id_table;
    });
}

bool gceEntityPackedRef_set::IsSameType() const
{
    return all_same(begin(), end(), [](const gceEntityPackedRef &A, const gceEntityPackedRef &B){
        return A.entity.get_schema()->getId() == B.entity.get_schema()->getId();
    });
}

bool gceEntityPackedRef_set::IsSameColumnValue(uint8_t index) const
{
    return all_same(begin(), end(), [index](const gceEntityPackedRef &A, const gceEntityPackedRef &B){
        return A.entity.equals(B.entity, index);
    });
}

bool gceEntityPackedRef_set::ContainsRec(const gceEntityPackedRef &ref) const
{
    return this->m_collection.find(ref) != this->m_collection.end();
}

bool gceEntityPackedRef_set::ContainsRec(const gceEntitySetKey &key) const
{
    return this->m_collection.find(key) != this->m_collection.end();
}

void gceEntityPackedRef_set::DeselectAll()
{
    if (!m_collection.empty())
    {
        m_collection.clear();

        umodelEntityRemoveMsg msg;
        msg.id_model = id_model;
        msg.data = {};
        m_ctx.postModelQueue(msg);
    }
}

size_t gceEntityPackedRef_set::Deselect(const gceEntityPackedRef &ref, bool notifyModel)
{
    size_t cnt = m_collection.erase(ref);
    if (cnt > 0 && notifyModel)
    {
        umodelEntityRemoveMsg msg;
        msg.id_model = id_model;
        msg.data = ref;
        m_ctx.postModelQueue(msg);
    }
    return cnt;
}

size_t gceEntityPackedRef_set::Deselect(const gceEntitySetKey &key, bool notifyModel)
{
    if (auto it = m_collection.find(key); it != m_collection.end())
    {
        if (notifyModel)
        {
            umodelEntityRemoveMsg msg;
            msg.id_model = id_model;
            msg.data = *it;
            m_ctx.postModelQueue(msg);
        }
        m_collection.erase(it);
        return 1;
    }
    return 0;
}

bool gceEntityPackedRef_set::DeselectExcept(const gceEntitySetKey &key)
{
    if (auto it = m_collection.find(key); it != m_collection.end() && m_collection.size() > 1)
    {
        auto ref = *it;

        DeselectAll();
        Select(ref);
        return true;
    }
    return false;
}

size_t gceEntityPackedRef_set::Replace(const gceEntityPackedRef &ref, bool notifyModel)
{
    if (auto it = m_collection.find(ref); it != m_collection.end())
    {
        it = m_collection.erase(it);
        m_collection.insert(it, ref);
        if (notifyModel)
        {
            umodelEntityUpdateMsg msg;
            msg.id_model = id_model;
            msg.data = ref;
            m_ctx.postModelQueue(msg);
        }
        return 1;
    }
    return 0;
}

void gceEntityPackedRef_set::Select(const gceEntityPackedRef &ref)
{
    if (m_collection.insert(ref).second)
    {
        umodelEntityAddMsg msg;
        msg.id_model = id_model;
        msg.data = ref;
        m_ctx.postModelQueue(msg);
    }
}

void gceEntityPackedRef_set::SelectXOR(const gceEntityPackedRef &ref)
{
    if (Deselect(ref, true) == 0)
    {
        Select(ref);
    }
}

void gceEntityPackedRef_set::processActionNotify(const udataMultiRowActionNotifyMsg &msg)
{
    size_t cnt = 0;
    for (auto &action : msg.m_actions)
    {
        // do not notify model as udataMultiRowActionNotifyMsg is sent to it
        if (action.query == gceActionType::Delete)
        {
            cnt += this->Deselect(gceEntityPackedRef{action.id_table, action.oldEntity}, false);
        }
        else if (action.query == gceActionType::Update)
        {
            cnt += this->Replace(gceEntityPackedRef{action.id_table, action.newEntity}, false);
        }
    }
    if (cnt > 0)
    {
        this->signalSelection();
    }
}


std::optional<gceSearchResultVertex> gceEntityPackedRef_set::search_vertex2D(const glm::dvec2 &pos, double r, const gceProjection *proj) const
{
    gceSearchResultVertex res;
    double rDistance = DoubleInfinity;
    for (auto &ref : this->getCollection())
    {
        if (auto cGeom = ref.entity.get_geometry(); cGeom)
        {
            //project_filter p(proj);
            //cGeom->apply_filter_rw(p);

            ClosestVertexFinder f{glm::dvec3(pos, 0.0), proj};
            cGeom->apply_filter_ro(f);
            //if (m)
            if (f.getDistance() < r && f.getDistance() < rDistance)
            {
                res.ref = ref;
                res.model = std::move(cGeom);
                rDistance = f.getDistance();
                res.index = f.getVertexIndex();
            }
        }
    }
    if (res.model)
    {
        return res;
    }
    return {};
}

std::optional<gceSearchResultVertex> gceEntityPackedRef_set::search_segment2D(const glm::dvec2 &pos, double r, const gceProjection *proj) const
{
    gceSearchResultVertex res;
    double rDistance = DoubleInfinity;
    for (auto &ref : this->getCollection())
    {
        if (auto cGeom = ref.entity.get_geometry(); cGeom)
        {
            //project_filter p(proj);
            //cGeom->apply_filter_rw(p);

            ClosestSegmentFinder f{glm::dvec3(pos, 0.0), proj};
            cGeom->apply_geometry_filter(f);
            if (f.getDistance() < r && f.getDistance() < rDistance)
            {
                res.ref = ref;
                res.model = std::move(cGeom);
                rDistance = f.getDistance();
                res.index = f.getVertexIndex();
                res.factor = f.getR();
            }
        }
    }
    if (res.model)
    {
        return res;
    }
    return {};
}
