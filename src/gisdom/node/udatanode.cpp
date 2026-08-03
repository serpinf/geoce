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
#include "udatanode.hpp"
#include <map>
#include "config.h"
#include "datasrc/datasrc.h"

namespace gce
{

class udataProcessor
{
public:
    explicit udataProcessor(gceContext *ctx) : m_ctx(ctx) {}

    void operator()(const udataConnectMsg &msg)
    {
        gceEntityStatus status = gceEntityStatus::FAILED;
        try
        {
            if (auto it = m_dataConnections.find(msg.id_datasrc); it != m_dataConnections.end())
            {
                status = gceEntityStatus::OK;
            }
            else
            {
                auto schema = m_ctx->cfg.findDatasourceSchema(msg.svcType);
                if (!schema)
                {
                    throw std::logic_error("unknown datasrc type");
                }
                m_dataConnections.emplace(msg.id_datasrc, schema->m_factory(msg.connStr));
                status = gceEntityStatus::OK;
                gceContext::log_message("Datasrc {} ({}) connected", msg.svcType, msg.connStr);
            }
        }
        catch (std::exception &e)
        {
            gceContext::log_message("Datasrc {} ({}) failed with error: {}", msg.svcType, msg.connStr, e.what());
        }
        post_ConnectReply(msg.sender, msg.id_datasrc, status);
    }

    void operator()(const udataConnectFinishMsg &msg)
    {
        if (m_dataConnections.erase(msg.id_datasrc) > 0)
        {
            post_ConnectReply(msg.sender, msg.id_datasrc, gceEntityStatus::NONE);
        }
        else
        {
            gceContext::log_message("no such connection");
        }
    }

    void operator()(const udataCreateTableMsg &msg)
    {
        bool res = false;
        if (auto conn = findDataSource(msg.id_datasrc); conn)
        {
            try
            {
                conn->createTable(msg.m_table);
                this->registerTable(msg.id_datasrc, msg.m_table);
                res = true;
            }
            catch (std::exception &e)
            {
                gceContext::log_message("{}", e.what());
            }
        }
        post_CreateTableReply(msg.sender, msg.m_table.id_table, res);
    }

    void operator()(const udataDropTableMsg &msg)
    {
        if (const TableInfo *table = findTable(msg.id_table); table != nullptr)
        {
            if (auto conn = getTableSource(table))
            {
                try
                {
                    conn->dropTable(table->table);
                    gceContext::log_message("Table {}.{} deleted", table->table.m_tableSchema, table->table.m_tableName);
                }
                catch (std::exception &e)
                {
                    gceContext::log_error("Table {}.{} NOT deleted: {}", table->table.m_tableSchema, table->table.m_tableName, e.what());
                }
            }
        }
    }

    void operator()(const udataRegisterTableMsg &msg)
    {
        bool res = false;
        if (auto conn = findDataSource(msg.id_datasrc); conn)
        {
            try
            {
                conn->selectTest(msg.m_tableInfo);
                registerTable(msg.id_datasrc, msg.m_tableInfo);
                res = true;
            }
            catch (std::exception &e)
            {
                gceContext::log_message("{}", e.what());
            }
        }
        post_CreateTableReply(msg.sender, msg.m_tableInfo.id_table, res);
    }

    void post_ConnectReply(uint32_t sender, const gce::uuid &serviceId, gceEntityStatus status)
    {
        udataConnectReplyMsg reply;
        reply.status = status;
        reply.id_datasrc = serviceId;
        m_ctx->postQueue(sender, std::move(reply));
    }
    void post_CreateTableReply(uint32_t sender, const gce::uuid &id_table, bool res)
    {
        m_ctx->postQueue(sender, udataTableStatusMsg{id_table, res});
    }

    void operator()(const udataListTablesMsg &msg)
    {
        if (auto conn = findDataSource(msg.id_datasrc); conn && msg.sender != 0)
        {
            udataListTablesReplyMsg reply;
            reply.id_table = msg.id_table;
            reply.id_datasrc = msg.id_datasrc;
            try
            {
                conn->listTables(reply.result);
            }
            catch (std::exception &e)
            {
                gceContext::log_message("{}", e.what());
            }
            m_ctx->postQueue(msg.sender, std::move(reply));
        }
    }

    void operator()(const udataSelectTestMsg &msg)
    {
        bool tableOk = false;
        if (auto *info = this->findTable(msg.id_table); info != nullptr)
        {
            if (auto conn = this->getTableSource(info); conn)
            {
                try
                {
                    conn->selectTest(info->table);
                    tableOk = true;
                }
                catch (std::exception &e)
                {
                    gceContext::log_error("{}", e.what());
                }
            }
        }
        post_CreateTableReply(msg.sender, msg.id_table, tableOk);
    }

    void operator()(const udataMultiRowActionQueryMsg &msg)
    {
        // find data source
        std::shared_ptr<gceDataConnection> conn;

        for (auto &action : msg.m_actions)
        {
            if (const TableInfo *info0 = this->findTable(action.id_table, false); info0 != nullptr)
            {

                if (auto actionConn = this->getTableSource(info0); actionConn)
                {
                    if (!conn)
                    {
                        conn = actionConn;
                    }
                    else if (conn != actionConn)
                    {
                        conn.reset();
                        gceContext::log_message("All row actions in a group must referece the same data connection");
                        break;
                    }
                }
                else
                {
                    conn.reset();
                    gceContext::log_message("No datasource for table");
                    break;
                }
            }
            else
            {
                conn.reset();
                gceContext::log_message("Table not found for edit action");
                break;
            }
        }

        std::vector<gceRowAction> rowActions;
        if (conn)
        {
            bool commitFlag = false;
            try
            {
                rowActions.reserve(msg.m_actions.size());

                conn->begin();
                for (auto &action : msg.m_actions)
                {
// TODO: cache this search
                    const TableInfo *info = this->findTable(action.id_table, false);

                    if (action.query == gceActionType::Insert)
                    {
                        auto tup = conn->execInsert(info->table, action.newEntity);
                        rowActions.emplace_back(action.query, action.id_table, tup, gceEntityPacked{});
                        //m_ctx->log().message(tup.to_string());
                    }
                    else if (action.query == gceActionType::Update)
                    {
                        auto tup = conn->execUpdate(info->table, action.newEntity);
                        rowActions.emplace_back(action.query, action.id_table, tup, action.oldEntity);
                        //m_ctx->log().message(tup.to_string());
                    }
                    else if (action.query == gceActionType::Delete)
                    {
                        conn->execDelete(info->table, action.oldEntity);
                        rowActions.emplace_back(action);
                        //m_ctx->log().message(tup.to_string());
                    }
                }
                commitFlag = true;
            }
            catch (std::exception &e)
            {
                gceContext::log_error("{}", e.what());
            }
            if (!conn->end(commitFlag))
            {
                rowActions.clear();
            }
        }

        // notify other parts
        m_ctx->postQueue(gce::queueId::MODEL | gce::queueId::WORKSPACE, udataMultiRowActionNotifyMsg{msg.m_name, rowActions});
    }

    void operator()(const udataLogTableQueryMsg &msg)
    {

        if (auto *info = this->findTable(msg.id_table); info)
        {
            if (auto conn = this->getTableSource(info); conn)
            {
                try
                {
                    auto arr = conn->selectAll(info->table, 100);
                    for (auto &en : arr)
                    {
                        gceContext::log_message("{}", en.to_string());
                    }
                }
                catch (std::exception &e)
                {
                    gceContext::log_error("{}", e.what());
                }
            }
        }
    }

    void operator()(const udataSelectAllMsg &msg)
    {
        if (auto *info = this->findTable(msg.id_table); info != nullptr)
        {
            if (auto conn = this->getTableSource(info); conn)
            {
                try
                {
                    m_ctx->postQueue(msg.sender, udataSelectReplyMsg{msg.id_table, conn->selectAll(info->table, msg.limit)});
                }
                catch (std::exception &e)
                {
                    gceContext::log_error("{}", e.what());
                }
            }
        }
    }

    void operator()(const udataSelectIDMsg &msg)
    {
        if (auto *info = this->findTable(msg.key.id_table); info)
        {
            if (auto conn = this->getTableSource(info); conn)
            {
                try
                {
                    m_ctx->postQueue(msg.sender, udataSelectReplyMsg{msg.key.id_table, conn->selectId(info->table, msg.key.id_entity)});
                }
                catch (std::exception &e)
                {
                    gceContext::log_error("{}", e.what());
                }
            }
        }
    }

    template <class T>
    void operator() (const T &)
    {
        gceContext::log_message("udataProcessor: unsupported msg type {}", typeid(T).name());
    }
private:
    struct TableInfo
    {
        gce::uuid id_datasrc{};
        std::weak_ptr<gceDataConnection> wpDataSrc;
        gceDSTable table;
    };

    std::shared_ptr<gceDataConnection> findDataSource(const gce::uuid &datasrcId, bool log = true)
    {
        if (auto it = m_dataConnections.find(datasrcId); it != m_dataConnections.end())
        {
            return it->second;
        }
        if (log)
        {
            gceContext::log_message("no such connection");
        }
        return {};
    }

    std::shared_ptr<gceDataConnection> getTableSource(const TableInfo *table, bool log = true)
    {
        auto conn = table->wpDataSrc.lock();
        if (!conn)
        {
            conn = findDataSource(table->id_datasrc, log);
        }
        return conn;
    }

    const TableInfo *findTable(const gce::uuid &tableId, bool log = true)
    {
        if (auto it = m_tables.find(tableId); it != m_tables.end())
        {
            return &it->second;
        }
        if (log)
        {
            gceContext::log_message("no such id_table");
        }
        return nullptr;
    }


    void registerTable(const gce::uuid &id_datasrc, const gceDSTable &table)
    {
        TableInfo info;
        info.id_datasrc = id_datasrc;
        if (auto it = m_dataConnections.find(id_datasrc); it != m_dataConnections.end())
        {
            info.wpDataSrc = it->second;
        }
        info.table = table;
        m_tables[table.id_table] = info;
        gceContext::log_message("register table: {}", info.table.m_tableName);
    }

    gceContext *m_ctx = nullptr;
    std::map<gce::uuid, std::shared_ptr<gceDataConnection>> m_dataConnections;
    std::map<gce::uuid, TableInfo> m_tables;
};

void udata_worker(gceContext *ctx)
{
    udataProcessor up{ctx};
    while (true)
    {
        auto msg = ctx->dataQueue.get();
        if (std::holds_alternative<gceQuitMessage>(msg))
        {
            break;
        }
        std::visit(up, msg);
        //up.processMsg(*msgInfo);
    }
    // process pending tasks?
}
}
