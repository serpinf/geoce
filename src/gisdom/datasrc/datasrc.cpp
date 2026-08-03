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
#include "datasrc.h"
#include "fmt/std.h"
#include "type/entitypck.h"

constexpr char msg_fromat[] = "not implemented: {}";
void gceDataConnection::selectTest(const gceDSTable &)
{
#if has_cpp20
    throw std::runtime_error(fmt::format(msg_fromat, std::source_location::current()));
#else
    throw std::runtime_error(fmt::format(msg_fromat, __FUNCTION__));
#endif
}

void gceDataConnection::createTable(const gceDSTable &)
{
#if has_cpp20
    throw std::runtime_error(fmt::format(msg_fromat, std::source_location::current()));
#else
    throw std::runtime_error(fmt::format(msg_fromat, __FUNCTION__));
#endif
}

void gceDataConnection::dropTable(const gceDSTable &)
{
#if has_cpp20
    throw std::runtime_error(fmt::format(msg_fromat, std::source_location::current()));
#else
    throw std::runtime_error(fmt::format(msg_fromat, __FUNCTION__));
#endif
}

void gceDataConnection::listTables(ListTablesQueryReply &)
{
#if has_cpp20
    throw std::runtime_error(fmt::format(msg_fromat, std::source_location::current()));
#else
    throw std::runtime_error(fmt::format(msg_fromat, __FUNCTION__));
#endif
}

gceEntityPacked gceDataConnection::execInsert(const gceDSTable &, const gceEntityPacked &)
{
#if has_cpp20
    throw std::runtime_error(fmt::format(msg_fromat, std::source_location::current()));
#else
    throw std::runtime_error(fmt::format(msg_fromat, __FUNCTION__));
#endif
}

gceEntityPacked gceDataConnection::execUpdate(const gceDSTable &, const gceEntityPacked &)
{
#if has_cpp20
    throw std::runtime_error(fmt::format(msg_fromat, std::source_location::current()));
#else
    throw std::runtime_error(fmt::format(msg_fromat, __FUNCTION__));
#endif
}

void gceDataConnection::execDelete(const gceDSTable &, const gceEntityPacked &)
{
#if has_cpp20
    throw std::runtime_error(fmt::format(msg_fromat, std::source_location::current()));
#else
    throw std::runtime_error(fmt::format(msg_fromat, __FUNCTION__));
#endif
}

std::vector<gceEntityPacked> gceDataConnection::selectAll(const gceDSTable &, int)
{
#if has_cpp20
    throw std::runtime_error(fmt::format(msg_fromat, std::source_location::current()));
#else
    throw std::runtime_error(fmt::format(msg_fromat, __FUNCTION__));
#endif
}

std::vector<gceEntityPacked> gceDataConnection::selectId(const gceDSTable &, const gceEntityKey &)
{
#if has_cpp20
    throw std::runtime_error(fmt::format(msg_fromat, std::source_location::current()));
#else
    throw std::runtime_error(fmt::format(msg_fromat, __FUNCTION__));
#endif
}

std::vector<gceEntityPacked> gceDataConnection::selectTile(const gce::uuid &id_table, const gce::tileid tile)
{
#if has_cpp20
    throw std::runtime_error(fmt::format(msg_fromat, std::source_location::current()));
#else
    throw std::runtime_error(fmt::format(msg_fromat, __FUNCTION__));
#endif
}

void gceDataConnection::begin()
{
#if has_cpp20
    throw std::runtime_error(fmt::format(msg_fromat, std::source_location::current()));
#else
    throw std::runtime_error(fmt::format(msg_fromat, __FUNCTION__));
#endif
}

bool gceDataConnection::end(bool) noexcept
{
    return false;
}
