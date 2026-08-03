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
#include "wmsnode.h"
#include "type/entitypck.h"
#include "node/udatanode.hpp"
#include "type/entity.h"

#include <filesystem>
#include <fstream>

#include "io/utils.h"
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/serialization/vector.hpp>

constexpr inline char datasrc_WMS[] = R"rawsvg(<svg xmlns="http://www.w3.org/2000/svg" width="200" height="200" viewBox="0 0 48 48"><g fill="none" stroke="#000" stroke-linecap="round" stroke-linejoin="round" stroke-width="4"><path d="M37.8261 4C41.6276 7.58886 44 12.6753 44 18.3158C44 29.1871 35.1871 38 24.3158 38C18.6753 38 13.5889 35.6276 10 31.8261"/><path fill="#2F88FF" fill-rule="evenodd" d="M24 32C31.732 32 38 25.732 38 18C38 10.268 31.732 4 24 4C16.268 4 10 10.268 10 18C10 25.732 16.268 32 24 32Z" clip-rule="evenodd"/><path d="M24 38V44"/><path d="M18 44H30"/></g></svg>)rawsvg";

struct wmsLayer
{
    std::string name;
    std::string urlFormat;
    std::string tileExt;

    std::string get_key() const
    {
        return name;
    }

    template<class Archive>
    void serialize(Archive &ar, const unsigned int)
    {
        ar &BOOST_SERIALIZATION_NVP(name);
        ar &BOOST_SERIALIZATION_NVP(urlFormat);
        ar &BOOST_SERIALIZATION_NVP(tileExt);
    }

};
static std::string format_tile_name(const std::string &prov, gce::tileid tile, const std::string &ext)
{
    return fmt::format("{}/{}/{}_{}.{}", prov, tile.get_z(), tile.get_x(), tile.get_y(), ext);
}
static std::string getFile(const std::string &prov, gce::tileid tile)
{
    auto path = std::filesystem::temp_directory_path() /= format_tile_name(prov, tile, "jpg");
    //wxMilliSleep(1000);
    //gceContext::log_message("{}", path.string());
    return get_file_contents(path.string().c_str());
}
static bool save_file_contents(const char *filename, const std::string &contents)
{
    if (auto *fp = std::fopen(filename, "wb"); fp != nullptr)
    {
        auto sz = std::fwrite(contents.data(), contents.size(), 1, fp);
        std::fclose(fp);
        return sz == contents.size();
    }
    return false;
}

static bool saveFile(const std::string &prov, gce::tileid tile, const std::string &contents)
{
    auto path = std::filesystem::temp_directory_path() /= format_tile_name(prov, tile, "jpg");

    std::filesystem::create_directories(path.parent_path());
    //gceContext::log_message("{}", path.string());
    return save_file_contents(path.string().c_str(), contents);
}
gceDataConnectionWMS::gceDataConnectionWMS(const std::string &connStr) : m_filename(connStr)
{
    curl = curl_easy_init();
    if (!curl)
    {
        throw std::runtime_error("curl init failed");
    }
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "Geographic client engine/0.1pre");
    if (!m_filename.empty())
    {
        if (std::ifstream ifs{m_filename}; ifs.good())
        {
            boost::archive::xml_iarchive ia(ifs);

            // restore the schedule from the archive
            ia >> boost::serialization::make_nvp("wmslayers", m_tables.get());
            m_tables.sort();
        }
        else
        {
            // save dummy file
            m_tables.insert({"dummy", "http://none", ".jpg"});
            if (std::ofstream ofs{m_filename}; ofs.good())
            {
                boost::archive::xml_oarchive oa(ofs);
                // write class instance to archive
                oa << boost::serialization::make_nvp("wmslayers", m_tables.get());
                // archive and stream closed when destructors are called
            }
        }
    }
}

gceDataConnectionWMS::~gceDataConnectionWMS()
{
    curl_easy_cleanup(curl);
}

size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t totalSize = size * nmemb;
    auto *buffer = static_cast<std::vector<unsigned char>*>(userp);
    buffer->insert(buffer->end(), (unsigned char *)contents, (unsigned char *)contents + totalSize);
    return totalSize;
}
void gceDataConnectionWMS::selectTest(const gceDSTable &table)
{
#if 0
    if (auto addr = getAddress(table, 1, 2, 3); !addr.empty())
    {
        std::vector<unsigned char> buffer;
        curl_easy_setopt(curl, CURLOPT_URL, addr.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK)
        {
            throw std::runtime_error(curl_easy_strerror(res));
        }
        m_buffer = buffer;
        //int x = 0, y = 0, comp = 0;
        //stbi_info_from_memory(buffer.data(), buffer.size(), &x, &y, &comp);
        //stbi_load_from_callbacks
        //gceContext::log_message("Image info {}, {}, {}", x, y, comp);
    }
#endif
}

void gceDataConnectionWMS::listTables(ListTablesQueryReply &reply)
{
    for (auto &tbl : m_tables)
    {
        reply.tableNames.emplace_back(std::string{}, tbl.name);
    }
}

#include "type/coretypes.h"
std::vector<gceEntityPacked> gceDataConnectionWMS::selectId(const gceDSTable &table, const gceEntityKey &id)
{
    if (table.m_typeSchema->getId() != gce::raster_schema_id)
    {
        throw std::logic_error("gceDataConnectionWMS data schema must be raster_schema_id");
    }
    std::vector<gceEntityPacked> res;
    auto keyId = table.m_typeSchema->getKeyIndex();
    auto dataId = table.m_typeSchema->findIndex("data");

    auto tid = std::get<int64_t>(id);
    gce::tileid tile(tid);
    auto buf = getFile(table.m_tableSchema + table.m_tableName, tile);
    if (buf.empty())
    {
        if (auto addr = getAddress(table, tile.get_x(), tile.get_y(), tile.get_z()); !addr.empty())
        {
            wxStopWatch ws;
            ws.Start();

            std::vector<unsigned char> buffer;
            curl_easy_setopt(curl, CURLOPT_URL, addr.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
            CURLcode cres = curl_easy_perform(curl);
            if (cres != CURLE_OK)
            {
                throw std::runtime_error(curl_easy_strerror(cres));
            }
            m_buffer = buffer;
            buf.assign((char *)buffer.data(), buffer.size());

            saveFile(table.m_tableSchema + table.m_tableName, tile, buf);
            gceContext::log_message("loaded {} : {}us", addr, ws.TimeInMicro().GetValue());
        }
    }
    //vid.get<int64_t>();
    gceEntityVar en{table.m_typeSchema};
    en[keyId] = tid;
    en[dataId] = std::move(buf);

    res.emplace_back(en);

    return res;
}

std::string gceDataConnectionWMS::getAddress(const gceDSTable &table, int x, int y, int z) const
{
    if (auto tbl = m_tables.get_optional(table.m_tableName); tbl)
    {
        return fmt::format(fmt::runtime(tbl->urlFormat), fmt::arg("x", x), fmt::arg("y", y), fmt::arg("z", z));
    }
    return {};
}

std::unique_ptr<const gceDatasourceSchema> gceDataConnectionWMS::schema()
{
    auto schema = std::make_unique<gceDatasourceSchema>();
    schema->m_name = "wms";
    schema->m_svgIcon = datasrc_WMS;
    schema->m_factory = [](const std::string &connStr)->std::unique_ptr<gceDataConnection>{
        return std::make_unique<gceDataConnectionWMS>(connStr);
    };
    return schema;
}
