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
// c++17 and c++20 features compatibility

#define has_cpp17 (__cplusplus >= 201703L)

#define has_cpp20 (__cplusplus >= 202002L)

#if has_cpp20
#include <span>

#define GCE_WITH_REQUIRES(...) requires (__VA_ARGS__)
#define GCE_REQUIRES_EXPR(...) requires { __VA_ARGS__; }
#else
#include <set>
#include <boost/core/span.hpp>

#define GCE_WITH_REQUIRES(...) /*std::enable_if_t<(__VA_ARGS__), int> = 0*/
#define GCE_REQUIRES_EXPR(...) /* Cannot easily emulate inside template arguments */
#endif

namespace gce
{
#if has_cpp20
using std::span;
using std::erase_if;
#else
using boost::span;

template< class Key, class Compare, class Alloc, class Pred >
size_t erase_if(std::set<Key, Compare, Alloc> &c, Pred pred)
{
    auto old_size = c.size();
    for (auto first = c.begin(), last = c.end(); first != last;)
    {
        if (pred(*first))
            first = c.erase(first);
        else
            ++first;
    }
    return old_size - c.size();
}
template< class T, class Alloc, class Pred >
size_t erase_if(std::vector<T, Alloc> &c, Pred pred)
{
    auto it = std::remove_if(c.begin(), c.end(), pred);
    auto r = c.end() - it;
    c.erase(it, c.end());
    return r;
}
#endif
}; // namespace gce
