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

#include <optional>
#include "entitypck.h"
#include "engine.hpp"
#include "sigslot.h"

struct udataMultiRowActionNotifyMsg;
class gceProjection;

struct gceSearchResultVertex
{
    gceEntityPackedRef ref;
    std::unique_ptr<geom::Geometry> model;
    size_t index = 0;
    double factor = 0.0;
};

class gceEntityPackedRef_set : boost::noncopyable
{
public:
    sigslot::signal1<int> selectionChanged;

    gceEntityPackedRef_set(gceContext &ctx, const gce::uuid &id_model) : m_ctx(ctx), id_model(id_model)
    {}

    bool IsSameLayer() const;

    bool IsSameType() const;

    bool IsSameColumnValue(uint8_t index) const;

    bool ContainsRec(const gceEntityPackedRef &ref) const;

    bool ContainsRec(const gceEntitySetKey &key) const;

    void DeselectAll();
    size_t Deselect(const gceEntityPackedRef &ref, bool notifyModel);
    size_t Deselect(const gceEntitySetKey &key, bool notifyModel);
    bool DeselectExcept(const gceEntitySetKey &key);
    size_t Replace(const gceEntityPackedRef &ref, bool notifyModel);

    void Select(const gceEntityPackedRef &ref);

    void SelectXOR(const gceEntityPackedRef &ref);

    void signalSelection()
    {
        selectionChanged(m_collection.size());
    }

    bool empty() const
    {
        return m_collection.empty();
    }
    auto begin() const
    {
        return m_collection.begin();
    }
    auto end() const
    {
        return m_collection.end();
    }
    const auto &getCollection() const
    {
        return m_collection;
    }
    auto size() const
    {
        return m_collection.size();
    }
    void processActionNotify(const udataMultiRowActionNotifyMsg &msg);

    std::optional<gceSearchResultVertex> search_vertex2D(const glm::dvec2 &pos, double r, const gceProjection *proj) const;

    std::optional<gceSearchResultVertex> search_segment2D(const glm::dvec2 &pos, double r, const gceProjection *proj) const;
private:
    std::set<gceEntityPackedRef, std::less<>> m_collection;
    gceContext &m_ctx;
    const gce::uuid id_model; // selection virtual layer/model id
};
