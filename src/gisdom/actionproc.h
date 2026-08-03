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
#include <deque>
#include "type/entitypck.h"
#include "engine.hpp"

class gceCommandGroup
{
public:
    explicit gceCommandGroup(const std::string &name = {}) : m_name(name)
    {}

    bool empty() const
    {
        return m_commands.empty();
    }

    void Insert(const gce::uuid &id_table, const gceEntityPacked &rec)
    {
        m_commands.emplace_back(gceActionType::Insert, id_table, rec, gceEntityPacked{});
    }

    void Update(const gce::uuid &id_table, const gceEntityPacked &rec, const gceEntityPacked &oldRec)
    {
        m_commands.emplace_back(gceActionType::Update, id_table, rec, oldRec);
    }

    void Delete(const gce::uuid &id_table, const gceEntityPacked &rec)
    {
        m_commands.emplace_back(gceActionType::Delete, id_table, gceEntityPacked{}, rec);
    }

    std::list<gceRowAction> m_commands;
    std::string m_name;
};

// used from UI thread only
class gceCmdProcessor final
{
public:
    using proc_t = std::function<void(bool)>;

    enum ActionState
    {
        ActionIdle,
        ActionExec,
        ActionUndo,
        ActionRedo
    };

    explicit gceCmdProcessor(gceContext &ctx) : m_ctx(ctx) {}

    void processMsg(const gce::MessageInfo &msg);

    /*!
     * @brief perform undo step async
     * @return true if posted successfully
     */
    bool postUndo();

    /*!
     * @brief perform redo step async
     * @return true if posted successfully
     */
    bool postRedo();

    /*!
     * @brief check if one or more undo operations are available and get operation name
     * @param s [out] undo operation name
     * @return true if an undo operation is available
     */
    bool canUndo(std::string &s) const;

    /*!
     * @brief check if one or more redo operations are available and get operation name
     * @param s [out] redo operation name
     * @return true if an redo operation is available
     */
    bool canRedo(std::string &s) const;

    /*!
     * @brief post command group and save it until processing result is recieved,
     * may fail if waiting for pevious command group processing
     * @param cmdGroup command group, command list is cleared if send is successfull
     * @return true if message is created and posted
     */
    bool postCommandGroup(const gceCommandGroup &cmdGroup, const std::shared_ptr<proc_t> &proc = {});

    /**
    * Clear the Undo/Redo buffer
    */
    void clearBuffer()
    {
        this->m_buffer.clear();
        this->m_sid = 0;
    }

    bool isIdle() const
    {
        return this->m_state == ActionIdle;
    }
private:
    bool canUndo() const
    {
        return m_sid > 0 && isIdle();
    }

    bool canRedo() const
    {
        return m_sid < m_buffer.size() && isIdle();
    }
    /*!
     * @brief post command group and save it until processing result is recieved
     * @param cmdGroup command group
     */
    void doPostCommandGroup(const gceCommandGroup &cmdGroup, ActionState state);
    gceContext &m_ctx;
    ActionState m_state = ActionIdle;
    size_t m_limit = 1000; //!< Limit of Undo buffer depth
    size_t m_sid = 0; //!< Undo buffer current position, 0 is the newest command
    std::deque<gceCommandGroup>	m_buffer; //!< Undo command stack
    std::weak_ptr<proc_t> m_weakProc;
};
