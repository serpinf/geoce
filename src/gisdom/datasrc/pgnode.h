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
#include "datasrc.h"
#include "pgconn.h"


class gceDataConnectionPG final : public gceDataConnection
{
public:
    gceDataConnectionPG(const std::string &connStr);

    void selectTest(const gceDSTable &table) override;
    void createTable(const gceDSTable &table) override;
    void dropTable(const gceDSTable &table) override;
    void listTables(ListTablesQueryReply &reply) override;
    gceEntityPacked execInsert(const gceDSTable &table, const gceEntityPacked &entity) override;
    gceEntityPacked execUpdate(const gceDSTable &table, const gceEntityPacked &entity) override;
    void execDelete(const gceDSTable &table, const gceEntityPacked &entity) override;

    std::vector<gceEntityPacked> selectAll(const gceDSTable &table, int limit) override;
    std::vector<gceEntityPacked> selectId(const gceDSTable &table, const gceEntityKey &id) override;
    void begin() override;
    bool end(bool commitFlag) noexcept override;

    static std::unique_ptr<const gceDatasourceSchema> schema();

private:
    PGresult_sp exec(const std::string &query) const;
    PGresult_sp execParams(const std::string &query, const gceEntityPacked &en) const;

    gcePGconn m_conn;
};


