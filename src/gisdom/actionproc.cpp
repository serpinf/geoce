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

#include "actionproc.h"

void gceCmdProcessor::processMsg(const gce::MessageInfo &msg)
{
    if (std::holds_alternative<udataMultiRowActionNotifyMsg>(msg))
    {
        auto &editNotify = std::get<udataMultiRowActionNotifyMsg>(msg);
        if (!editNotify.m_actions.empty())
        {
            if (this->m_state == ActionExec)
            {
                // TODO: limit buffer size to m_Limit
                this->m_buffer.resize(m_sid + 1);
                this->m_buffer[m_sid].m_name = editNotify.m_name;
                this->m_buffer[m_sid].m_commands.assign(editNotify.m_actions.begin(), editNotify.m_actions.end());
                this->m_sid++;
            }
            else if (this->m_state == ActionUndo)
            {
                assert(this->m_sid != 0);
                this->m_sid--;
            }
            else if (this->m_state == ActionRedo)
            {
                assert(this->m_sid < this->m_buffer.size());
                this->m_sid++;
            }
        }
        else
        {
            if (this->m_state == ActionUndo || this->m_state == ActionRedo)
            {
                clearBuffer();
            }
        }
        if (auto proc = m_weakProc.lock(); proc)
        {
            (*proc)(!editNotify.m_actions.empty());
        }
        m_weakProc = {};
        this->m_state = ActionIdle;
    }

}

bool gceCmdProcessor::postCommandGroup(const gceCommandGroup &cmdGroup, const std::shared_ptr<proc_t> &proc)
{
    if (this->isIdle())
    {
        this->doPostCommandGroup(cmdGroup, ActionExec);
        m_weakProc = proc;
        return true;
    }
    gceContext::log_message("gceCmdProcessor::postCommandGroup failed, previous action in progress");
    return false;
}

bool gceCmdProcessor::postUndo()
{
    if (canUndo())
    {
        this->doPostCommandGroup(this->m_buffer[m_sid - 1], ActionUndo);
        return true;
    }
    return false;
}

bool gceCmdProcessor::postRedo()
{
    if (canRedo())
    {
        this->doPostCommandGroup(this->m_buffer[m_sid], ActionRedo);
        return true;
    }
    return false;
}


bool gceCmdProcessor::canUndo(std::string &s) const
{
    bool res = canUndo();
    if (res)
    {
        s = this->m_buffer[this->m_sid - 1].m_name;
    }
    return res;
}

bool gceCmdProcessor::canRedo(std::string &s) const
{
    bool res = canRedo();
    if (res)
    {
        s = this->m_buffer[this->m_sid].m_name;
    }
    return res;
}

void gceCmdProcessor::doPostCommandGroup(const gceCommandGroup &cmdGroup, ActionState state)
{
    udataMultiRowActionQueryMsg msg;
    msg.sender = gce::queueId::WORKSPACE;
    msg.m_name = cmdGroup.m_name;
    msg.m_actions.assign(cmdGroup.m_commands.begin(), cmdGroup.m_commands.end());
    if (state == ActionUndo)
    {
        for (auto &action : msg.m_actions)
        {
            if (action.query == gceActionType::Delete)
            {
                action.query = gceActionType::Insert;
            }
            else if (action.query == gceActionType::Insert)
            {
                action.query = gceActionType::Delete;
            }
            std::swap(action.newEntity, action.oldEntity);
        }
    }
    m_ctx.postDataQueue(msg);
    m_state = state;
}
