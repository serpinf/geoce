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

#include "pgconn.h"
#include "engine.hpp"

void gcePGconn::connectdb(const std::string &conninfo)
{
    // this call results in one-time memory leak detected by CRTDebug that happens in openssl
    m_conn = PQconnectdb(conninfo.c_str());

    /* Check to see that the backend connection was successfully made */
    if (PQstatus(m_conn) != CONNECTION_OK)
    {
        throw PGexception(PGRES_EMPTY_QUERY, PQerrorMessage(m_conn), conninfo);
    }
    // init state
    Init();
}

void gcePGconn::finish()
{
    if (m_conn)
    {
        PQfinish(m_conn);
        m_conn = nullptr;
    }
}
bool gcePGconn::IsOk() const
{
    return (m_conn != nullptr) && (PQstatus(m_conn) == CONNECTION_OK);
}

PGresult_sp gcePGconn::exec(const std::string &query) const
{
    PGresult_sp result(PQexec(m_conn, query.c_str()));

    ExecStatusType rc = result.resultStatus();
    if (rc == PGRES_EMPTY_QUERY || rc == PGRES_BAD_RESPONSE || rc == PGRES_FATAL_ERROR)
    {
        throw PGexception(rc, PQerrorMessage(m_conn), query);
    }
    return result;
}

PGresult_sp gcePGconn::exec(const std::string &query,
    int nParams,
    const Oid *paramTypes,
    const char *const *paramValues,
    const int *paramLengths,
    const int *paramFormats,
    int outfmt) const
{

    PGresult_sp result(
        PQexecParams(m_conn, query.c_str(), nParams, paramTypes, paramValues, paramLengths, paramFormats, outfmt)
    );

    ExecStatusType rc = result.resultStatus();
    if (rc == PGRES_EMPTY_QUERY || rc == PGRES_BAD_RESPONSE || rc == PGRES_FATAL_ERROR)
    {
        throw PGexception(rc, PQerrorMessage(m_conn), query);
    }
    return result;
}

static void gcePG_NoticeProcessor(void *, const char *message)
{
    gceContext::log_message("{}", message);
}

bool gcePGconn::Init()
{
    // init connection notice processing
    PQsetNoticeProcessor(m_conn, gcePG_NoticeProcessor, nullptr);

    /*FILE *f=fopen( "pglog.txt","w");
    PQtrace(m_conn,f);*/
    exec("SET client_encoding TO 'UTF-8'");
    return true;
}

PGnotify_sp gcePGconn::get_notify(int &flag) const
{
    PQconsumeInput(m_conn);
    PGnotify_sp notify(PQnotifies(m_conn));
    if (notify)
    {
        // set flag to 0 if sent from this connection, to 1 - otherwise
        flag = (PQbackendPID(m_conn) != notify->be_pid);
    }
    return notify;
}

