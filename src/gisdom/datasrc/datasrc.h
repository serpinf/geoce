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
#include "type/typeschema.hpp"
#include "tileid.h"
#include "datasrcshema.h"

struct gceDSTable;

struct ListTablesQueryReply;

class gceDataConnection
{
public:
    virtual ~gceDataConnection() = default;
    virtual void selectTest(const gceDSTable &table);
    virtual void createTable(const gceDSTable &table);
    virtual void dropTable(const gceDSTable &table);
    virtual void listTables(ListTablesQueryReply &reply);

    virtual gceEntityPacked execInsert(const gceDSTable &table, const gceEntityPacked &entity);
    virtual gceEntityPacked execUpdate(const gceDSTable &table, const gceEntityPacked &entity);
    virtual void execDelete(const gceDSTable &table, const gceEntityPacked &entity);

    virtual std::vector<gceEntityPacked> selectAll(const gceDSTable &table, int limit);

    virtual std::vector<gceEntityPacked> selectId(const gceDSTable &table, const gceEntityKey &key);

    virtual std::vector<gceEntityPacked> selectTile(const gce::uuid &id_table, const gce::tileid tile);

    virtual void begin();
    virtual bool end(bool commitFlag) noexcept;
};
