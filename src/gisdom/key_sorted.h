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
#include <vector>

namespace gce
{
template <class T, class K>
class table_data
{
public:
    std::vector<T> &get()
    {
        return m_data;
    }
    std::optional<T> get_optional(const K &key) const
    {
        auto it = std::lower_bound(m_data.begin(), m_data.end(), key, key_comparator{});
        if (it != m_data.end() && get_key(*it) == key)
        {
            return *it;
        }
        return {};
    }

    bool insert(const T &val)
    {
        const K key = get_key(val);
        auto it = std::lower_bound(m_data.begin(), m_data.end(), key, key_comparator{});
        if (it == m_data.end() || get_key(*it) != key)
        {
            m_data.insert(it, val);
            return true;
        }
        return false;
    }

    bool update(const T &val)
    {
        const K key = get_key(val);
        auto it = std::lower_bound(m_data.begin(), m_data.end(), key, key_comparator{});
        if (it != m_data.end() && get_key(*it) == key)
        {
            *it = val;
            return true;
        }
        return false;

    }

    bool erase(const K &key)
    {
        auto it = std::lower_bound(m_data.begin(), m_data.end(), key, key_comparator{});
        if (it != m_data.end() && get_key(*it) == key)
        {
            m_data.erase(it);
            return true;
        }
        return false;
    }

    template <class Pred>
    size_t erase_if(Pred p)
    {
        return gce::erase_if(m_data, p);
    }

    template <class UnaryPred>
    std::vector<T> select_if(UnaryPred p) const
    {
        std::vector<T> res;
        res.reserve(std::count_if(m_data.begin(), m_data.end(), p));
        std::copy_if(m_data.begin(), m_data.end(), std::back_inserter(res), p);
        return res;
    }
    size_t size() const
    {
        return m_data.size();
    }
    auto begin() const
    {
        return m_data.begin();
    }
    auto end() const
    {
        return m_data.end();
    }
    /*!
     * @brief sort after bulk load using underlying container interface
     */
    void sort()
    {
        std::sort(m_data.begin(), m_data.end(), key_comparator{});
    }
private:
    static K get_key(const T &val)
    {
        return val.get_key();
    }

    struct key_comparator
    {
        bool operator()(const T &a, const T &b) const
        {
            return get_key(a) < get_key(b);
        }

        bool operator()(const T &a, const K &key) const
        {
            return get_key(a) < key;
        }
    };

    std::vector<T> m_data;
};
}
