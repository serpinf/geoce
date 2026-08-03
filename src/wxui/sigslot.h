// sigslot.h: Signal/Slot classes
// 
// Written by Sarah Thompson (sarah@telergy.com) 2002.
//
// License: Public domain. You are free to use this code however you like, with the proviso that
//          the author takes on no responsibility or liability for any use.
//
// QUICK DOCUMENTATION 
// 
// Only single thread / single parameter siplified version
//		

#pragma once

#include <set>
#include <list>

namespace sigslot
{

class has_slots;

template<class arg_type>
class _connection_base
{
public:
    virtual ~_connection_base() {}
    virtual has_slots *getdest() const = 0;
    virtual void emit(arg_type) = 0;
};

class _signal_base
{
public:
    virtual ~_signal_base() = default;
    virtual void slot_disconnect(has_slots *pslot) = 0;
};

class has_slots
{
public:
    has_slots() = default;

    has_slots(const has_slots &hs) = delete;
    has_slots &operator = (const has_slots &hs) = delete;

    void signal_connect(_signal_base *sender)
    {
        m_senders.insert(sender);
    }

    void signal_disconnect(_signal_base *sender)
    {
        m_senders.erase(sender);
    }

    virtual ~has_slots()
    {
        disconnect_all();
    }

    void disconnect_all()
    {
        for (auto &rec : m_senders)
        {
            rec->slot_disconnect(this);
        }
        m_senders.clear();
    }

private:
    std::set<_signal_base *> m_senders;
};

template<class dest_type, class arg_type>
class _connection final : public _connection_base<arg_type>
{
public:
    _connection(dest_type *pobject, void (dest_type:: *pmemfun)(arg_type)) :
        m_pobject(pobject), m_pmemfun(pmemfun)
    {}

    void emit(arg_type a1) final
    {
        (m_pobject->*m_pmemfun)(a1);
    }

    has_slots *getdest() const final
    {
        return m_pobject;
    }

private:
    dest_type *m_pobject;
    void (dest_type:: *m_pmemfun)(arg_type);
};

template<class arg_type>
class signal1 final : public _signal_base
{
public:
    signal1() = default;

    signal1(const signal1<arg_type> &s) = delete;

    ~signal1()
    {
        disconnect_all();
    }

    void disconnect_all()
    {
        for (auto &rec : m_connected_slots)
        {
            rec->getdest()->signal_disconnect(this);
        }

        m_connected_slots.clear();
    }

    void disconnect(has_slots *pclass)
    {
        slot_disconnect(pclass);
        pclass->signal_disconnect(this);
    }

    void slot_disconnect(has_slots *pslot)
    {
        m_connected_slots.remove_if([pslot](auto &c){ return c->getdest() == pslot; });
    }

    void emit(arg_type a1)
    {
        for (auto &rec : this->m_connected_slots)
        {
            rec->emit(a1);
        }
    }

    void operator()(arg_type a1)
    {
        emit(a1);
    }

    template<class desttype>
    void connect(desttype *pclass, void (desttype:: *pmemfun)(arg_type))
    {
        this->m_connected_slots.emplace_back(new _connection<desttype, arg_type>(pclass, pmemfun));
        pclass->signal_connect(this);
    }

private:
    std::list<std::unique_ptr<_connection_base<arg_type>>> m_connected_slots;
};

}; // namespace sigslot
