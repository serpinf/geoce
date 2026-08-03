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
#include "demdata.h"
#include "type/entitypck.h"
#include "node/udatanode.hpp"
#include "type/entity.h"

#include <filesystem>
#include "alg/wgsop.h"
#include <gdal_priv.h>
#include <cpl_conv.h>
constexpr char datasrc_DEM[] = R"rawsvg(< symbol viewBox = "0 0 512 512" id = "terrain" > <title>Terrain SVG Icon< / title><path fill = "currentColor" d = "m40.841 312l103.652-112.88l71.904 71.904l76.29 76.289l22.626-22.626l-77.069-77.07l89.494-95.887L470.836 312H496v-19.864L328.262 104.27L215.603 224.976l-72.096-72.096L16 291.741V312zM16 392h480v32H16z">< / path>< / symbol>)rawsvg";

static std::string tileFileNameSqlite(const std::string &path, uint32_t x, uint32_t y, uint8_t z)
{
    return fmt::format("{}/z{}/{}/{}/{}.{}.sqlitedb", path, z, x >> 10, y >> 10, x >> 8, y >> 8);
}

namespace
{
static geom::Box2D flat_box(gce::tileid tile)
{
    double scale = std::ldexp(2.0 * M_PI, -tile.get_z());
    double xmin = -M_PI + scale * tile.get_x();
    double ymax = M_PI - scale * tile.get_y();
    return geom::Box2D{glm::dvec2(xmin, ymax - scale), glm::dvec2(xmin + scale, ymax)};
}

};

#include "sqlite3.h"
namespace
{
class MBTilesManager
{
    sqlite3 *db = nullptr;

public:
    MBTilesManager(const std::string &db_path)
    {
        if (sqlite3_open(db_path.c_str(), &db) != SQLITE_OK)
        {
            if (db != nullptr)
            {
                std::string msg = sqlite3_errmsg(db);
                sqlite3_close(db);
                throw std::runtime_error(msg);
            }
            throw std::runtime_error("Failed to open database");
        }
        create_schema();
    }

    ~MBTilesManager()
    {
        sqlite3_close(db);
    }

    void create_schema()
    {
        const char *sql =
            "CREATE TABLE IF NOT EXISTS metadata (name TEXT, value TEXT, PRIMARY KEY(name));"
            "CREATE TABLE IF NOT EXISTS tiles ("
            "zoom_level INTEGER,"
            "tile_column INTEGER,"
            "tile_row INTEGER,"
            "tile_data BLOB,"
            "PRIMARY KEY(zoom_level, tile_column, tile_row));";

        char *errMsg = nullptr;
        if (sqlite3_exec(db, sql, nullptr, nullptr, &errMsg) != SQLITE_OK)
        {
            std::string err = errMsg;
            sqlite3_free(errMsg);
            throw std::runtime_error("Schema error: " + err);
        }

        // Insert metadata if empty
        sqlite3_exec(db, "INSERT OR IGNORE INTO metadata (name, value) VALUES ('name', 'demtiles');", nullptr, nullptr, nullptr);
        sqlite3_exec(db, "INSERT OR IGNORE INTO metadata (name, value) VALUES ('format', 'float');", nullptr, nullptr, nullptr);
    }

    bool write_tile(int z, int x, int y, const std::string &data)
    {
        const char *sql = "INSERT OR REPLACE INTO tiles (zoom_level, tile_column, tile_row, tile_data) VALUES (?, ?, ?, ?);";
        sqlite3_stmt *stmt;

        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
        {
            gceContext::log_error("Prepare failed");
            return false;
        }

        sqlite3_bind_int(stmt, 1, z);
        sqlite3_bind_int(stmt, 2, x);
        sqlite3_bind_int(stmt, 3, y);
        sqlite3_bind_blob(stmt, 4, data.data(), static_cast<int>(data.size()), SQLITE_STATIC);

        if (sqlite3_step(stmt) != SQLITE_DONE)
        {
            gceContext::log_error("Execution failed");
            sqlite3_finalize(stmt);
            return false;
        }
        sqlite3_finalize(stmt);
        return true;
    }

    static std::string read_tile(const std::string &db_path, int z, int x, int y)
    {
        std::string data;

        sqlite3 *db = nullptr;
        if (sqlite3_open_v2(db_path.c_str(), &db, SQLITE_OPEN_READONLY, nullptr) == SQLITE_OK)
        {
            const char *sql = "SELECT tile_data FROM tiles WHERE zoom_level = ? AND tile_column = ? AND tile_row = ?;";

            sqlite3_stmt *stmt;
            if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK)
            {
                sqlite3_bind_int(stmt, 1, z);
                sqlite3_bind_int(stmt, 2, x);
                sqlite3_bind_int(stmt, 3, y);

                if (sqlite3_step(stmt) == SQLITE_ROW)
                {
                    const char *blob = reinterpret_cast<const char *>(sqlite3_column_blob(stmt, 0));
                    int bytes = sqlite3_column_bytes(stmt, 0);
                    data.assign(blob, blob + bytes);
                }
            }
            sqlite3_finalize(stmt);
        }
        if (db != nullptr)
        {
            sqlite3_close(db);
        }
        return data;
    }
};
}

static bool saveFile2(const std::string &cachePath, gce::tileid tile, const std::string &contents)
{
    auto path = std::filesystem::path(tileFileNameSqlite(cachePath, tile.get_x(), tile.get_y(), tile.get_z()));

    std::filesystem::create_directories(path.parent_path());
    MBTilesManager tmon(path.string());
    return tmon.write_tile(tile.get_z(), tile.get_x(), tile.get_y(), contents);
}

static std::string getFile3(const std::string &cachePath, gce::tileid tile)
{
    const std::string path = tileFileNameSqlite(cachePath, tile.get_x(), tile.get_y(), tile.get_z());
    return MBTilesManager::read_tile(path, tile.get_z(), tile.get_x(), tile.get_y());
}

gceDataConnectionDEM::gceDataConnectionDEM(const std::string &connStr) : m_path(connStr)
{
    std::filesystem::path p = m_path;

    if (!std::filesystem::exists(p) || !std::filesystem::is_directory(p))
    {
        throw std::runtime_error(fmt::format("Folder {} does not exist.", m_path));
    }
}

gceDataConnectionDEM::~gceDataConnectionDEM() = default;

void gceDataConnectionDEM::selectTest(const gceDSTable &table)
{}

void gceDataConnectionDEM::listTables(ListTablesQueryReply &reply)
{
    reply.tableNames.emplace_back(std::string{}, "demcache");
}

#include "type/coretypes.h"
std::vector<gceEntityPacked> gceDataConnectionDEM::selectId(const gceDSTable &table, const gceEntityKey &id)
{
    if (table.m_typeSchema->getId() != gce::raster_schema_id)
    {
        throw std::logic_error("gceDataConnectionDEM data schema must be raster_schema_id");
    }
    std::vector<gceEntityPacked> res;
    auto keyId = table.m_typeSchema->getKeyIndex();
    auto dataId = table.m_typeSchema->findIndex("data");

    auto tid = std::get<int64_t>(id);
    gce::tileid tile(tid);
    auto buf = getFile3(m_path, tile);

    gceEntityVar en{table.m_typeSchema};
    en[keyId] = tid;
    en[dataId] = std::move(buf);

    res.emplace_back(en);

    return res;
}
std::unique_ptr<const gceDatasourceSchema> gceDataConnectionDEM::schema()
{
    auto schema = std::make_unique<gceDatasourceSchema>();
    schema->m_name = "demcache";
    schema->m_svgIcon = datasrc_DEM;
    schema->m_factory = [](const std::string &connStr)->std::unique_ptr<gceDataConnection>{
        return std::make_unique<gceDataConnectionDEM>(connStr);
    };
    return schema;
}

inline int posIndex(const double tmin, const double tmax, const double t, const int count)
{
    return std::clamp(int(count * (t - tmin) / (tmax - tmin)), 0, count - 1);
}
static std::string format_fabdem_tile_name(const std::string &path, int x, int y)
{
    char lat = (y >= 0) ? 'N' : 'S';
    char lon = (x >= 0) ? 'E' : 'W';
    return fmt::format("{}/{}{:02d}{}{:03d}_FABDEM_V1-2.tif", path, lat, std::abs(y), lon, std::abs(x));
}

#include <set>
class CachedSourceTile
{
public:
    CachedSourceTile(const std::string &filePath, const std::pair<int, int> &tileXY)
    {
        poDataset = (GDALDataset *)GDALOpen(filePath.c_str(), GA_ReadOnly);
        if (poDataset == nullptr)
        {
            //gceContext::log_error("Failed to open file: {}", filePath);
            return;
        }

        poBand = poDataset->GetRasterBand(1);
        if (poBand == nullptr)
        {
            gceContext::log_error("Failed to get band 1");
            GDALClose(poDataset);
            poDataset = nullptr;
            return;
        }
        width = poDataset->GetRasterXSize();
        height = poDataset->GetRasterYSize();
        this->tileXY = tileXY;
    }
    ~CachedSourceTile()
    {
        if (poDataset)
        {
            GDALClose(poDataset);
        }
    }
    CachedSourceTile(const CachedSourceTile &) = delete;
    void operator=(const CachedSourceTile &) = delete;

    CachedSourceTile(CachedSourceTile &&other) noexcept
        : tileXY(other.tileXY), poDataset(other.poDataset), poBand(other.poBand), width(other.width), height(other.height)
    {
        other.poDataset = nullptr;
        other.poBand = nullptr;
        other.width = 0;
        other.height = 0;
    }

    void operator=(CachedSourceTile &&other) noexcept
    {
        if (this != &other)
        {
            if (poDataset)
            {
                GDALClose(poDataset);
            }
            tileXY = other.tileXY;
            poDataset = other.poDataset;
            poBand = other.poBand;
            width = other.width;
            height = other.height;
            other.poDataset = nullptr;
            other.poBand = nullptr;
            //other.tileXY = {-1, -1};
            //other.width = 0;
            //other.height = 0;
        }
    }

    bool isValid() const
    {
        return poDataset != nullptr;
    }
    std::pair<int, int> tileXY{-1, -1};
    GDALDataset *poDataset = nullptr;
    GDALRasterBand *poBand = nullptr;
    int width = 0;
    int height = 0;
};
class FABDEMReader
{
public:
    FABDEMReader(const std::string &path) : m_path(path)
    {
        // Register all drivers
        GDALAllRegister();
    }

    std::optional<float> readFABDEMPixel(const std::string &filePath, const glm::dvec2 &frac)
    {
        // Open the dataset
        GDALDataset *poDataset = (GDALDataset *)GDALOpen(filePath.c_str(), GA_ReadOnly);

        if (poDataset == nullptr)
        {
            //gceContext::log_error("Failed to open file: {}", filePath);
            return 0.0f;
        }

        // Get dimensions
        int width = poDataset->GetRasterXSize();
        int height = poDataset->GetRasterYSize();

        //int 

        // Get GeoTransform (Origin, Pixel Size, Rotation)
        //if (poDataset->GetGeoTransform(result.geoTransform) != CE_None)
        //{
          //  gceContext::log_error("Failed to get GeoTransform");
        //}

        // Get the first band (DEMs are single-band)
        GDALRasterBand *poBand = poDataset->GetRasterBand(1);
        if (poBand == nullptr)
        {
            gceContext::log_error("Failed to get band 1");
            GDALClose(poDataset);
            return 0.0f;
        }

        int pixelX = static_cast<int>(frac.x * (width - 1));
        int pixelY = height - 1 - static_cast<int>(frac.y * (height - 1));
        // Clamp pixel coordinates to valid range
        pixelX = std::clamp(pixelX, 0, width - 1);
        pixelY = std::clamp(pixelY, 0, height - 1);

        // Allocate memory for pixels
        //result.data.resize(result.width * result.height);
        float result = 0.0;
        // Read the data into the vector
        // FABDEM is typically Float32
        CPLErr err = poBand->RasterIO(GF_Read,
                                      pixelX, pixelY, 1, 1,
                                      &result,
                                      1, 1,
                                      GDT_Float32, 0, 0);

        // Cleanup
        GDALClose(poDataset);

        if (err != CE_None)
        {
            gceContext::log_error("Error reading raster data");
            return {};
        }
        return result;
    }
    std::optional<float> readFABDEMPixel(CachedSourceTile &src, const glm::dvec2 &frac)
    {
        if (!src.isValid())
        {
            //gceContext::log_error("Failed to open file: {}", filePath);
            return std::nullopt;
        }

        int pixelX = static_cast<int>(frac.x * (src.width - 1));
        int pixelY = src.height - 1 - static_cast<int>(frac.y * (src.height - 1));
        // Clamp pixel coordinates to valid range
        pixelX = std::clamp(pixelX, 0, src.width - 1);
        pixelY = std::clamp(pixelY, 0, src.height - 1);

        // Allocate memory for pixels
        //result.data.resize(result.width * result.height);
        float result = 0.0;
        // Read the data into the vector
        // FABDEM is typically Float32
        CPLErr err = src.poBand->RasterIO(GF_Read,
                                      pixelX, pixelY, 1, 1,
                                      &result,
                                      1, 1,
                                      GDT_Float32, 0, 0);
        return result;
    }

    float get_dem_value(const glm::dvec2 &coordsdeg)
    {
        int tileX = static_cast<int>(std::floor(coordsdeg.x));
        int tileY = static_cast<int>(std::floor(coordsdeg.y));
        std::pair<int, int> tileXY{tileX, tileY};
        if (m_cachedTile.tileXY == tileXY)
        {
            return readFABDEMPixel(m_cachedTile, glm::fract(coordsdeg)).value_or(0.0f);
        }
        if (m_missing.count(tileXY) > 0)
        {
            return 0.0f; // or some error value
        }

        m_cachedTile = CachedSourceTile(format_fabdem_tile_name(m_path, tileX, tileY), tileXY);

        if (!m_cachedTile.isValid())
        {
            m_missing.insert(tileXY);
            return 0.0f; // or some error value
        }
        return readFABDEMPixel(m_cachedTile, glm::fract(coordsdeg)).value_or(0.0f);
    }

    std::string get_dem_tile_values(const gce::tileid tile)
    {
        std::string buf;
        buf.resize(gce::sizep * gce::sizep * sizeof(float));
        float *data = (float *)buf.data();
        auto box = flat_box(tile);
        glm::dvec2 flatpos;
        for (int j = 0; j < gce::sizep; ++j)
        {
            flatpos.y = glm::mix(box.cmax.y, box.cmin.y, double(j) / (gce::sizep - 1));
            for (int i = 0; i < gce::sizep; ++i)
            {
                flatpos.x = glm::mix(box.cmin.x, box.cmax.x, double(i) / (gce::sizep - 1));
                float val = get_dem_value(wgsop::flat2wgs_degrees(flatpos));
                data[j * gce::sizep + i] = val;
            }
        }
        return buf;
    }

private:
    std::set < std::pair<int, int>> m_missing;
    std::string m_path;
    CachedSourceTile m_cachedTile{"", {-1, -1}};
};

void dem_updateCache(const std::string &pathDest, const std::string &pathSrc, const std::string &srctype, const gcePyramidAOI &aoi)
{
    //test_read(pathSrc, aoi);
    FABDEMReader reader(pathSrc);
    const glm::dvec2 cmin = wgsop::wgs_degrees2flat(aoi.bbox.cmin);
    const glm::dvec2 cmax = wgsop::wgs_degrees2flat(aoi.bbox.cmax);
    for (int z = aoi.levMin; z <= aoi.levMax; ++z)
    {
        int count = 1 << z;
        int x0 = posIndex(-M_PI, M_PI, cmin.x, count);
        int x1 = posIndex(-M_PI, M_PI, cmax.x, count);
        int y0 = posIndex(-M_PI, M_PI, cmin.y, count);
        int y1 = posIndex(-M_PI, M_PI, cmax.y, count);
        for (int x = x0; x <= x1; ++x)
        {
            for (int y = y0; y <= y1; ++y)
            {
                const gce::tileid tile(x, count - 1 - y, z);
                auto buf = reader.get_dem_tile_values(tile);
                //auto buf = unpack_image(tile, 4);
                saveFile2(pathDest, tile, buf);
                //gceContext::log_message("DEM cache updated for tile {} from {} to {}", tile.to_string(), pathSrc, pathDest);
            }
        }
        gceContext::log_message("DEM cache updated for tile level {} from {} to {}", z, pathSrc, pathDest);
    }
}
