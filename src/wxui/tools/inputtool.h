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

#include "tool.h"
#include "type/typeschema.hpp"
#include "geom/CoordSeq.h"


class gceInputToolBase : public gceToolBase
{
public:
    gceInputToolBase(gceEditorFrame &owner, int dimension);

    bool isAvailable() override;

    bool isInputTool() const override
    {
        return true;
    }
protected:
    virtual std::unique_ptr<geom::Geometry> createInputGeometry(geom::CoordinateType cooType) const;
    virtual bool canInsertGeometry() const;
    bool EndUse_Custom() override;

    void onInputFinished(bool commit);
    void ConnectLayer();
    void DisconnectLayer();

    /**
    * put new feature to current map/layer
    *
    * @return op status
    */
    bool _insert_feature();

    wxString describeGeometry() const;

    //! save geometry from input sequence to m_inputRef
    virtual void saveGeometry();
    virtual void restoreGeometry();
    virtual void OnSetEnabled() override;
    bool DoEnter();
    virtual void updateModel() {}

    bool CanInsert() const;
    bool m_inputPosted = false;
    geom::CoordinateSeq m_ring{geom::CoordinateType::XYZM}; //!< input sequence
    geom::Coordinate m_NextCoord; //!< next coordinate for input
    int m_dimension;
    std::shared_ptr<std::function<void(bool)>> m_inputProc;
};

