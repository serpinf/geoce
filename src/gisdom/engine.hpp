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
#include <queue>
#include <chrono>
#include <mutex>
#include <variant>
#include <map>

#include "tileid.h"
#include "type/entitypck.h"
#include "geom/aabb.h"

class gceModelSchema;

struct gcePyramidAOI
{
    geom::Box2D bbox;
    int levMin = 0;
    int levMax = 0;
};

enum class gceEntityStatus : uint8_t
{
    NONE,
    OK,
    FAILED
};

enum class gceRenderMode
{
    FLAT,
    GLOBE
};

enum class gceTechType
{
    NONE,
    BASIC,
    BASIC_RTE2,
    TILED2,
    TILED3,
    OBJECTS2,
    OBJECTS3
};

constexpr inline char gceModelType_TEST[] = "TEST";
constexpr inline char gceModelType_SELECTION[] = "SELECTION";

namespace gce
{
constexpr inline int sizep_l2 = 5;
constexpr inline int sizep = 1 + (1 << sizep_l2);

enum queueId : uint32_t
{
    WORKSPACE = 0x1,
    DATA = 0x2,
    MODEL = 0x4,
    RENDER = 0x8
};

}

struct gceDSTable
{
    gce::uuid id_table;
    const gceTypeSchema *m_typeSchema = nullptr;
    std::string m_tableName;
    std::string m_tableSchema;
};

namespace gce
{
struct datasrc_info
{
    gce::uuid id_datasrc{};
    std::string type;
    std::string name;
    std::string params;
    int autoConnect = 1;

    gce::uuid get_key() const
    {
        return id_datasrc;
    }

    template<class Archive>
    void serialize(Archive &ar, const unsigned int version);
};

struct table_info
{
    gce::uuid id_table{};
    gce::uuid id_datasrc{};
    gce::uuid id_schema{};
    uint32_t flags = 0;
    std::string label;
    std::string name;
    std::string dbSchema;

    gce::uuid get_key() const
    {
        return id_table;
    }

    template<class Archive>
    void serialize(Archive &ar, const unsigned int version);
};

struct model_table_input_info
{
    gce::uuid id_table;
    std::map<std::string, std::string> column_map; // first = model input column name, second = table column name

    bool operator== (const model_table_input_info &other) const
    {
        return id_table == other.id_table && column_map == other.column_map;
    }

    template<class Archive>
    void serialize(Archive &ar, const unsigned int version);
};

struct model_model_input_info
{
    gce::uuid id_model;

    bool operator== (const model_model_input_info &other) const
    {
        return id_model == other.id_model;
    }

    template<class Archive>
    void serialize(Archive &ar, const unsigned int version);
};

struct model_info
{
    gce::uuid id_model{};
    std::string label;
    std::string type;
    std::map<std::string, model_table_input_info> tableInputs;
    std::map<std::string, model_model_input_info> modelInputs;

    gce::uuid get_key() const
    {
        return id_model;
    }

    template<class Archive>
    void serialize(Archive &ar, const unsigned int version);
};

struct scene_info
{
    gce::uuid id_scene{};
    bool visible = false;
    std::string label;
    std::string srs;
    std::string style;

    gce::uuid get_key() const
    {
        return id_scene;
    }

    template<class Archive>
    void serialize(Archive &ar, const unsigned int version);
};

struct layer_info
{
    gce::uuid id_layer{};
    gce::uuid id_scene{};
    gce::uuid id_model{};
    std::string label;
    bool visible = false;
    int order = 0;

    gce::uuid get_key() const
    {
        return id_layer;
    }

    template<class Archive>
    void serialize(Archive &ar, const unsigned int version);
};

}

struct udataConnectMsg
{
    gce::uuid id_datasrc{};
    std::string svcType;
    std::string connStr;
    uint32_t sender = 0;
};

struct udataConnectReplyMsg
{
    gce::uuid id_datasrc{};
    gceEntityStatus status = gceEntityStatus::NONE;
    bool isOk() const
    {
        return status == gceEntityStatus::OK;
    }
};
struct udataConnectFinishMsg
{
    gce::uuid id_datasrc{};
    uint32_t sender = 0;
};

struct udataSvcRemoveMsg
{
    gce::uuid id_datasrc{};
};

struct udataRegisterTableMsg
{
    gce::uuid id_datasrc{};
    gceDSTable m_tableInfo;
    uint32_t sender = 0;
};

struct udataCreateTableMsg
{
    gce::uuid id_datasrc{};
    gceDSTable m_table;
    uint32_t sender = 0;
};

struct udataTableStatusMsg
{
    gce::uuid id_table;
    bool result;
};

struct udataDropTableMsg
{
    gce::uuid id_table;
};

struct udataListTablesMsg
{
    gce::uuid id_datasrc{};
    gce::uuid id_table;
    uint32_t sender = 0;
};

struct ListTablesQueryReply
{
    std::vector<std::pair<std::string, std::string>> tableNames; // schema + name
};

struct udataListTablesReplyMsg
{
    gce::uuid id_datasrc;
    gce::uuid id_table;
    ListTablesQueryReply result;
};

struct udataSelectTestMsg
{
    gce::uuid id_table;
    uint32_t sender = 0;
};


struct udataSelectAllMsg
{
    gce::uuid id_table;
    int limit = 100;
    uint32_t sender = 0;
};

struct udataSelectIDMsg
{
    gceEntitySetKey key;
    uint32_t sender = 0;
};

struct udataSelectTileMsg
{
    gce::uuid id_table;
    gce::tileid tile;
    uint32_t sender = 0;
};

struct udataSelectReplyMsg
{
    gce::uuid id_table;
    std::vector<gceEntityPacked> rows;
};


enum class gceActionType : uint8_t
{
    Insert,
    Update,
    Delete
};

struct gceRowAction
{
    gceRowAction(const gceActionType action, const gce::uuid &tableId, const gceEntityPacked &newRow, const gceEntityPacked &oldRow)
        : query(action), id_table(tableId), newEntity(newRow), oldEntity(oldRow)
    {}

    gceActionType query;
    gce::uuid id_table;
    gceEntityPacked newEntity;
    gceEntityPacked oldEntity;
};

struct udataMultiRowActionQueryMsg
{
    uint32_t sender = 0;
    std::string m_name;
    std::vector<gceRowAction> m_actions;
};

struct udataMultiRowActionNotifyMsg
{
    std::string m_name;
    std::vector<gceRowAction> m_actions; // TODO: use shared data storage to avoid copying
};

struct udataLogTableQueryMsg
{
    gce::uuid id_table;
};

struct umodelReplaceMsg
{
    gce::model_info info;
};

struct umodelCustomPropertyBase
{
    virtual ~umodelCustomPropertyBase() = default;
};
struct umodelUpdateMsg
{
    gce::uuid id_model{};
    uint32_t flags = 0;
    std::unique_ptr<umodelCustomPropertyBase> props;
};

struct umodelRemoveMsg
{
    gce::uuid id_model{};
};

struct umodelStatusMsg
{
    gce::uuid id_model{};
    uint32_t sender = 0;
};

struct umodelStatusReplyMsg
{
    gce::uuid id_model{};
    gceEntityStatus status = gceEntityStatus::NONE;
};

struct umodelQueryTileImageMsg
{
    gce::uuid id_model{};
    gce::tileid tileId{};
    uint32_t sender = 0;
};

struct umodelQueryTileDEMMsg
{
    gce::uuid id_model{};
    gce::tileid tileId{};
    uint32_t sender = 0;
};

struct umodelQueryMesh2DMsg
{
    gce::uuid id_model{};
    gce::tileid tileId{};
    uint32_t sender = 0;
};

struct umodelQueryMesh3DMsg
{
    gce::uuid id_model{};
    gce::tileid tileId{};
    uint32_t flags = 0; // query type, what data to query
};

// test if transfomed geometry fits into (-1, +1) space
struct umodelSelect2DMsg
{
    glm::dvec3 aoi{0.0}; // (posX, posY, radius)
    gce::uuid id_modelActive{};
    std::vector<gce::uuid> model_ids;
    int limit = 1;
    uint32_t sender = 0;
};

struct SelectedInfo
{
    SelectedInfo(const gceEntitySetKey &key, double radius) : key(key), radius(radius) {}

    gceEntitySetKey key;
    double radius;
};

struct umodelSelectXDResultMsg
{
    std::vector<SelectedInfo> data;
};

struct umodelEntityAddMsg
{
    gce::uuid id_model{};
    gceEntityPackedRef data;
};

struct umodelEntityUpdateMsg
{
    gce::uuid id_model{};
    gceEntityPackedRef data;
};

struct umodelEntityRemoveMsg
{
    gce::uuid id_model{};
    gceEntityPackedRef data;
};

enum class gcePimitiveType : uint8_t
{
    TRIANGLES,
    LINE_STRIP,
    POINTS
};

struct urenderReplaceTechMsg
{
    gce::uuid id_layer{};
    gce::uuid id_model{};
    gceTechType techFlat;
    gceTechType techGlobe;
    bool visible = false;
    int order = 0;
};

struct urenderUpdateTechMsg
{
    gce::uuid id_layer{};
    bool visible = false;
    int order = 0;
};

struct urenderRemoveTechMsg
{
    gce::uuid id_layer{};
};

struct urenderTechStatusMsg
{
    gce::uuid id_layer{};
    uint32_t sender = 0;
};

struct urenderStatusReplyMsg
{
    gce::uuid id_layer{};
    gceEntityStatus status = gceEntityStatus::NONE;
};

struct urenderTechDataMsg
{
    explicit urenderTechDataMsg(gceTechType techType) : techType(techType) {}
    virtual ~urenderTechDataMsg() = default;
    gce::uuid id_model{};
    gceTechType techType;
};

struct urenderTexData
{
    uint16_t w = 0;
    uint16_t h = 0;
    uint16_t c = 4; // should be colorformat
    std::vector<std::uint8_t> data;
};

// raw RGB 256x256 for now
struct urenderTileImageDataMsg
{
    gce::uuid id_model{};
    gce::tileid tile;
    urenderTexData tex;
    bool modelOK = true;
};

struct gceQVertex
{
    glm::fvec3 pos;
    glm::fvec3 normal;
};

struct gceQPatch
{
    gceQVertex array[gce::sizep][gce::sizep];
};

// sizep*sizep vertex array data
struct urenderTileDEMDataMsg
{
    gce::uuid id_model{};
    gce::tileid tile;
    std::unique_ptr<gceQPatch> patch;
    geom::psAABB bbox;
    float hmax = 1.0f;
    bool modelOK = true;
};

struct urenderDrawFrameMsg
{
    glm::dmat4 proj; // TODO: replace with camera pos
    geom::aabb2 aoi;
    glm::ivec4 viewport;
};

struct urenderDrawFrame3DMsg
{
    glm::dvec3 basePoint;
    glm::dvec3 pos;
    glm::dvec3 front;
    glm::dvec3 up;
    float fovy = 60.0;
    glm::ivec4 viewport;
    bool wireFrameTerrain = false;
};

struct urenderFrameMsMsg
{
    double time_ms = 10.0;
};

struct wspSceneDeletedMsg
{
    gce::uuid id_scene;
};

struct wspSceneUpdatedMsg
{
    gce::scene_info info;
};

struct wspLayerUpdatedMsg
{
    gce::layer_info info;
};

struct wspLayerDeletedMsg
{
    gce::uuid id_layer;
};

struct wspModelUpdatedMsg
{
    gce::model_info info;
};

struct wspModelDeletedMsg
{
    gce::uuid id_model;
};

struct wspTableUpdatedMsg
{
    gce::table_info info;
};

struct wspTableDeletedMsg
{
    gce::uuid id_table;
};


struct wspResetMsg {};

struct wspActiveLayerInfo
{
    gce::uuid id_layer{};
    gce::uuid id_model{};
    gce::uuid id_table{};
    gce::uuid id_schema{};
    gce::uuid id_scene{};
    std::string sceneName;
    std::string layerName;
};

struct wspActiveLayerMsg
{
    wspActiveLayerInfo info;
};

struct gceQuitMessage {};

using gce_msg = std::variant <
    gceQuitMessage,
    udataConnectMsg,
    udataConnectReplyMsg,
    udataConnectFinishMsg,
    udataSvcRemoveMsg,
    udataTableStatusMsg,
    udataListTablesMsg,
    udataListTablesReplyMsg,
    udataCreateTableMsg,
    udataRegisterTableMsg,
    udataMultiRowActionQueryMsg,
    udataMultiRowActionNotifyMsg,
    udataSelectIDMsg,
    udataSelectTileMsg,
    umodelSelectXDResultMsg,
    udataSelectReplyMsg,
    udataSelectTestMsg,
    udataSelectAllMsg,
    udataDropTableMsg,
    udataLogTableQueryMsg,

    umodelReplaceMsg,
    umodelUpdateMsg,
    umodelRemoveMsg,
    umodelSelect2DMsg,
    umodelEntityRemoveMsg,
    umodelEntityUpdateMsg,
    umodelEntityAddMsg,
    umodelStatusMsg,
    umodelStatusReplyMsg,
    umodelQueryMesh2DMsg,
    umodelQueryTileImageMsg,
    umodelQueryTileDEMMsg,

    urenderStatusReplyMsg,
    urenderReplaceTechMsg, // create or replace
    urenderUpdateTechMsg, // update only visiblity and order
    urenderRemoveTechMsg,
    urenderDrawFrameMsg,
    urenderDrawFrame3DMsg,
    urenderFrameMsMsg,
    urenderTileImageDataMsg,
    urenderTileDEMDataMsg,
    std::shared_ptr<urenderTechDataMsg>, // TODO: replace with message data field inheritance

    wspActiveLayerMsg,
    wspLayerUpdatedMsg,
    wspLayerDeletedMsg,
    wspModelUpdatedMsg,
    wspModelDeletedMsg,
    wspTableUpdatedMsg,
    wspTableDeletedMsg,
    wspSceneUpdatedMsg,
    wspSceneDeletedMsg,
    wspResetMsg
> ;
namespace gce
{
using MessageInfo = gce_msg;
template <typename T>
struct my_remove_cvref
{
    using type = typename std::remove_cv<typename std::remove_reference<T>::type>::type;
};

template <typename T>
using my_remove_cvref_t = typename my_remove_cvref<T>::type;
template< class T>
constexpr bool is_message_v = boost::mp11::mp_contains<gce::MessageInfo, my_remove_cvref_t<T>>::value;
}

//-----------------------------------------------------------------------------
class gceContext;
class gceConfig;

namespace gce
{
template <class MSG>
class ThreadQueue
{
    friend class ::gceContext;
public:

    MSG get()
    {
        std::unique_lock<std::mutex> lk(cv_m);
        while (msg_queue.empty())
        {
            cv.wait(lk);
        }
        MSG msg_result = std::move(msg_queue.front());
        msg_queue.pop();
        return msg_result;
    }

    std::queue<MSG> get_all()
    {
        std::unique_lock<std::mutex> lk(cv_m);
        while (msg_queue.empty())
        {
            cv.wait(lk);
        }
        std::queue<MSG> result;
        msg_queue.swap(result);
        return result;
    }

    std::queue<MSG> try_get_all()
    {
        std::queue<MSG> result;
        std::lock_guard<std::mutex> lk(cv_m);
        if (!msg_queue.empty())
        {
            msg_queue.swap(result);
        }
        return result;
    }

    bool tryGet(MSG &msg_result)
    {
        std::lock_guard<std::mutex> lk(cv_m);
        bool res = !msg_queue.empty();
        if (res)
        {
            msg_result = std::move(msg_queue.front());
            msg_queue.pop();
        }
        return res;
    }
    /// <summary>
    /// Get message from queue until the specified time point.
    /// </summary>
    /// <returns>true if a message was recieved</returns>
    template< class Clock, class Duration >
    bool get_until(MSG &msg_result, const std::chrono::time_point<Clock, Duration> &timeout_time)
    {
        std::unique_lock<std::mutex> lk(cv_m);
        while (msg_queue.empty())
        {
            if (cv.wait_until(lk, timeout_time) == std::cv_status::timeout)
            {
                return false;
            }
        }
        msg_result = std::move(msg_queue.front());
        msg_queue.pop();
        return true;
    }

    /// <summary>
    /// Get message from queue until the specified time point.
    /// </summary>
    /// <returns>true if a message was recieved</returns>
    template< class Clock, class Duration >
    std::queue<MSG> get_all_until(const std::chrono::time_point<Clock, Duration> &timeout_time)
    {
        std::queue<MSG> result;
        std::unique_lock<std::mutex> lk(cv_m);
        while (msg_queue.empty())
        {
            if (cv.wait_until(lk, timeout_time) == std::cv_status::timeout)
            {
                return result;
            }
        }
        msg_queue.swap(result);
        return result;
    }
    size_t size()
    {
        std::lock_guard<std::mutex> lk(cv_m);
        return msg_queue.size();
    }
private:

    template <class T> void post(T &&msg)
    {
        {
            std::lock_guard<std::mutex> lk(cv_m);
            msg_queue.emplace(std::forward<T>(msg));
        }
        cv.notify_one();
    }

    template <class T> bool post(T &&msg, size_t hwm)
    {
        bool res = false;
        {
            std::lock_guard<std::mutex> lk(cv_m);
            if (msg_queue.size() < hwm)
            {
                msg_queue.emplace(std::forward<T>(msg));
                res = true;
            }
        }
        if (res)
        {
            cv.notify_one();
        }
        return res;

    }

    std::condition_variable cv;
    std::mutex cv_m;
    std::queue<MSG> msg_queue;
};

using MessageQueue = ThreadQueue<MessageInfo>;
using LogQueue = ThreadQueue<std::string>;

inline MessageInfo makeQuitMsg()
{
    return MessageInfo();
}

} // gce

class gceContext
{
public:
    explicit gceContext(const gceConfig &cfg) : cfg(cfg) {}

    //TODO: split to postQueue(msg&&) and postMultipleQueue(const msg&) ?
    template <class Tmsg>
    void postQueue(uint32_t target, const Tmsg &msg) GCE_WITH_REQUIRES(gce::is_message_v<Tmsg>)
    {
        static_assert(gce::is_message_v<Tmsg>, "Tmsg must be a gce::MessageInfo type");

        if (target & gce::queueId::DATA)
        {
            this->dataQueue.post(msg);
        }
        if (target & gce::queueId::MODEL)
        {
            this->modelQueue.post(msg);
        }
        if (target & gce::queueId::RENDER)
        {
            this->renderQueue.post(msg);
        }
        if (target & gce::queueId::WORKSPACE)
        {
            this->workspaceQueue.post(msg);
        }
    }

    void postWorkspaceQueue(gce::MessageInfo &&msg)
    {
        workspaceQueue.post(std::move(msg));
    }

    template <class Tmsg>
    void postDataQueue(Tmsg &&msg) GCE_WITH_REQUIRES(gce::is_message_v<Tmsg>)
    {
        static_assert(gce::is_message_v<Tmsg>, "Tmsg must be a gce::MessageInfo type");
        dataQueue.post(std::forward<Tmsg>(msg));
    }

    template <class Tmsg>
    bool postDataQueue(Tmsg &&msg, int hwm) GCE_WITH_REQUIRES(gce::is_message_v<Tmsg>)
    {
        static_assert(gce::is_message_v<Tmsg>, "Tmsg must be a gce::MessageInfo type");
        return dataQueue.post(std::forward<Tmsg>(msg), hwm);
    }

    void postModelQueue(gce::MessageInfo &&msg)
    {
        modelQueue.post(std::move(msg));
    }

    bool postModelQueue(gce::MessageInfo &&msg, int hwm)
    {
        return modelQueue.post(std::move(msg), hwm);
    }

    template <class Tmsg>
    void postRenderQueue(Tmsg &&msg) GCE_WITH_REQUIRES(gce::is_message_v<Tmsg>)
    {
        static_assert(gce::is_message_v<Tmsg>, "Tmsg must be a gce::MessageInfo type");
        renderQueue.post(std::forward<Tmsg>(msg));
    }
    template <typename... Args>
    static void log_message(fmt::format_string<Args...> s, Args&&... args)
    {
        m_logQueue.post(fmt::format(s, std::forward<Args>(args)...));
    }

    template <typename... Args>
    static void log_error(fmt::format_string<Args...> s, Args&&... args)
    {
        m_logQueue.post(fmt::format(s, std::forward<Args>(args)...));
    }

    static bool checkLog(std::string &msg);

    gce::MessageQueue workspaceQueue;
    gce::MessageQueue dataQueue;
    gce::MessageQueue modelQueue;
    gce::MessageQueue renderQueue;
    const gceConfig &cfg;
private:
    static gce::LogQueue m_logQueue;
};

