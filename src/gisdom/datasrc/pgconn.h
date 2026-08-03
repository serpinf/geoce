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
#include <libpq-fe.h>

struct PGexception : public std::runtime_error
{
    PGexception(const ExecStatusType rc, const std::string &error, const std::string &query)
        : std::runtime_error(format_error(rc, error, query)) {}

private:

#ifdef DEBUG
    std::string format_error(const ExecStatusType, const std::string &error, const std::string &query)
    {
        return error + "\n" + query;
    }
#else
    std::string format_error(const ExecStatusType, const std::string &error, const std::string &)
    {
        return error;
    }
#endif
};

class PGresult_sp
{
public:
    explicit PGresult_sp(PGresult *res = nullptr) : m_res(res) {}
    ~PGresult_sp()
    {
        if (m_res) PQclear(m_res);
    }
    ExecStatusType resultStatus()
    {
        return ::PQresultStatus(m_res);
    }
    int fnumber(const char *field_name)
    {
        return ::PQfnumber(m_res, field_name);
    }
    Oid ftype(int field_num)
    {
        return ::PQftype(m_res, field_num);
    }
    const char *fname(int field_num)
    {
        return ::PQfname(m_res, field_num);
    }
    int	ntuples() const
    {
        return ::PQntuples(m_res);
    }
    int	nfields() const
    {
        return ::PQnfields(m_res);
    }
    char *cmdTuples()
    {
        return ::PQcmdTuples(m_res);
    }
    char *getvalue(int tup_num, int field_num)
    {
        return ::PQgetvalue(m_res, tup_num, field_num);
    }
    int getlength(int tup_num, int field_num)
    {
        return ::PQgetlength(m_res, tup_num, field_num);
    }
    int getisnull(int tup_num, int field_num)
    {
        return ::PQgetisnull(m_res, tup_num, field_num);
    }
    PGresult_sp(PGresult_sp &&other) noexcept : m_res(other.m_res)
    {
        other.m_res = nullptr;
    }
    PGresult_sp &operator = (PGresult_sp &&other) noexcept
    {
        std::swap(m_res, other.m_res);
        return *this;
    }
private:
    PGresult_sp(const PGresult_sp &) = delete;
    PGresult_sp &operator = (const PGresult_sp &) = delete;
    PGresult *m_res = nullptr;
};

struct PQfreemem_caller
{
    void operator ()(void *p) { PQfreemem(p); }
};
typedef std::unique_ptr<PGnotify, PQfreemem_caller> PGnotify_sp;

typedef std::unique_ptr<char, PQfreemem_caller> PGchar_sp;

class gcePGconn : boost::noncopyable
{
public:
    ~gcePGconn()
    {
        this->finish();
    }

    void connectdb(const std::string &conninfo);

    void finish();

    bool IsOk() const;

    PGresult_sp exec(const std::string &query) const;

    PGresult_sp exec(const std::string &query,
                     int nParams,
                     const Oid *paramTypes,
                     const char *const *paramValues,
                     const int *paramLengths,
                     const int *paramFormats,
                     int outfmt) const;

    PGnotify_sp get_notify(int &flag) const;

    bool IsTransactionOk() const
    {
        return PQtransactionStatus(m_conn) == PQTRANS_INTRANS;
    }

    const char *db() const
    {
        return PQdb(m_conn);
    }
    const char *user() const
    {
        return PQuser(m_conn);
    }
    const char *host() const
    {
        return PQhost(m_conn);
    }
    const char *port() const
    {
        return PQport(m_conn);
    }
    PGconn *get()
    {
        return m_conn;
    }
    PGchar_sp escapeIdentifier(const std::string &name)
    {
        return PGchar_sp(PQescapeIdentifier(m_conn, name.c_str(), name.size()));
    }
private:
    bool Init();
    PGconn *m_conn = nullptr;
};
