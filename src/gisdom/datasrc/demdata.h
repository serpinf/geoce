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
struct gcePyramidAOI;
class gceDataConnectionDEM final : public gceDataConnection
{
public:
    explicit gceDataConnectionDEM(const std::string &connStr);
    ~gceDataConnectionDEM();

    void selectTest(const gceDSTable &table) override;
    void listTables(ListTablesQueryReply &reply) override;

    std::vector<gceEntityPacked> selectId(const gceDSTable &table, const gceEntityKey &id) override;
    static std::unique_ptr<const gceDatasourceSchema> schema();

private:
    std::string getAddress(const gceDSTable &table, int x, int y, int z) const;

    std::string m_path;
    std::vector<unsigned char> m_buffer;
};

void dem_updateCache(const std::string &pathDest, const std::string &pathSrc, const std::string &srctype, const gcePyramidAOI &aoi);
