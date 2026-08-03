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

#include "inputtool.h"
#include "editorfrm.h"
#include "type/entitypck.h"
#include "type/entity.h"
#include "alg/alg_geos.h"
#include "geom/CoordinateFilter.h"

class CollectCoordinates_filter : public geom::filter_ro
{
public:
    explicit CollectCoordinates_filter(geom::CoordinateSeq &data) : m_data(data) {}

    void operator()(const geom::Coordinate &coo) override
    {
        m_data.push_back(coo);
    }
private:
    geom::CoordinateSeq &m_data;
};

gceInputToolBase::gceInputToolBase(gceEditorFrame &owner, int dimension) : gceToolBase(owner), m_dimension(dimension)
{
    m_inputProc = std::make_shared<gceCmdProcessor::proc_t>(std::bind(&gceInputToolBase::onInputFinished, this, std::placeholders::_1));
}

bool gceInputToolBase::isAvailable()
{
    auto &al = m_owner.getActiveLayer();
    if (al.schema == nullptr)
        return false;
    auto geometryColumn = al.schema->getGeometryColumn();
    return geometryColumn != nullptr && geometryColumn->getGeometryDimension() == m_dimension;
}


std::unique_ptr<geom::Geometry> gceInputToolBase::createInputGeometry(geom::CoordinateType cooType) const
{
    if (auto pgeom = geom::Geometry::Create(m_dimension, cooType, m_ring); pgeom)
    {
        pgeom->MakeValid(); // nead this to close rings

        return geom::alg_geos::MakeValid(*pgeom);
    }
    return {};
}

bool gceInputToolBase::canInsertGeometry() const
{
    return (int)m_ring.size() > m_dimension;
}

bool gceInputToolBase::EndUse_Custom()
{
    bool result = false;
    if (EndUseChild())
    {
        DisconnectLayer();
        result = true;
    }
    return result;
}

void gceInputToolBase::onInputFinished(bool commit)
{
    // do not process unrelated actionprocessor events like undo/redo
    if (m_inputPosted)
    {
        m_inputPosted = false;
        if (commit)
        {
            m_ring.clear();
            updateModel();
        }
        saveGeometry();
    }
}

void gceInputToolBase::ConnectLayer()
{
    auto &al = m_owner.getActiveLayer();
    if (al.schema != nullptr)
    {
        auto &selection = m_owner.getSelection();
        selection.DeselectAll();
        selection.Select({{}, al.schema->createDefaultEntityPacked()});
        selection.signalSelection();
    }
    else
    {
        throw std::runtime_error("gceInputToolBase activated with no active layer");
    }
}
void gceInputToolBase::DisconnectLayer()
{
    // drop current selction
    m_owner.getSelection().DeselectAll();
    m_owner.getSelection().signalSelection();
}
bool gceInputToolBase::_insert_feature()
{
    bool res = false;
    if (auto &sel = m_owner.getSelection(); !sel.empty())
    {
        auto first = sel.begin();
        if (auto pgeom = createInputGeometry(*first->entity.getCoordinateType()); pgeom)
        {
            gceEntityVar inputEntity{first->entity, std::move(pgeom)};

            gceCommandGroup cmds("Insert");
            cmds.Insert(m_owner.getActiveLayer().id_table, gceEntityPacked{inputEntity});
            if (this->m_owner.getActionProcessor().postCommandGroup(cmds, m_inputProc))
            {
                this->m_inputPosted = true;
                res = true;
            }
        }
    }

    return res;
}

void gceInputToolBase::restoreGeometry()
{
    m_ring.clear();
    if (auto &sel = m_owner.getSelection(); !sel.empty())
    {
        auto first = sel.begin();

        if (auto xgeom = first->entity.get_geometry(); xgeom)
        {
            m_ring.reserve(xgeom->getNumPoints());
            CollectCoordinates_filter f(m_ring);
            xgeom->apply_filter_ro(f);
        }
    }
}

void gceInputToolBase::OnSetEnabled()
{
    restoreGeometry();
}

bool gceInputToolBase::DoEnter()
{
    if (CanInsert())
    {
        return _insert_feature();
    }
    return false;
}

bool gceInputToolBase::CanInsert() const
{
    return (!m_inputPosted && canInsertGeometry());
}

wxString gceInputToolBase::describeGeometry() const
{
    if (!m_ring.empty())
    {
        geom::CoordinateSeq seq(geom::CoordinateType::XY);
        seq.reserve(m_ring.size() + 1);
        seq.assign(m_ring);
        seq.push_back(m_NextCoord);
        if (auto pgeom = geom::Geometry::Create(m_dimension, geom::CoordinateType::XY, seq); pgeom)
        {
            return gceToolBase::describeGeometry(*pgeom);
        }
    }
    return {};
}

void gceInputToolBase::saveGeometry()
{
    if (auto &selection = m_owner.getSelection(); !selection.empty())
    {
        auto &entity = selection.begin()->entity;
        gceEntityVar en{entity, geom::Geometry::Create(m_dimension, *entity.getCoordinateType(), m_ring)};
        selection.Replace({{}, gceEntityPacked{en}}, true);
    }
}
