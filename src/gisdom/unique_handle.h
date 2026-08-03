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

#include <utility>
#include <type_traits>

namespace gce
{
// A move-only RAII wrapper for handles/resources released with a stateless Deleter.
// Deleter must be default-constructible, stateless and callable with a single T argument.
template <class T, class Deleter>
class unique_handle
{
    static_assert(std::is_invocable_v<Deleter, T>, "Deleter must be callable with handle type T");
    static_assert(std::is_default_constructible_v<Deleter>, "Deleter must be default-constructible");
    static_assert(std::is_empty_v<Deleter>, "Only stateless deleters are supported");

public:
    using handle_type = T;
    using deleter_type = Deleter;

    explicit constexpr unique_handle(handle_type h = handle_type{}) noexcept
        : m_handle(h)
    {}

    unique_handle(const unique_handle &) = delete;
    unique_handle &operator=(const unique_handle &) = delete;

    unique_handle(unique_handle &&other) noexcept
        : m_handle(std::exchange(other.m_handle, handle_type{}))
    {}

    unique_handle &operator=(unique_handle &&other) noexcept
    {
        if (this != &other)
        {
            reset(std::exchange(other.m_handle, handle_type{}));
        }
        return *this;
    }

    ~unique_handle() noexcept
    {
        if (!is_empty(m_handle))
            Deleter()(m_handle);
    }

    // Observers
    constexpr handle_type get() const noexcept { return m_handle; }
    constexpr handle_type name() const noexcept { return get(); } // compatibility
    explicit constexpr operator bool() const noexcept { return !is_empty(m_handle); }

    // Modifiers
    constexpr handle_type release() noexcept { return std::exchange(m_handle, handle_type{}); }

    void reset(handle_type h = handle_type{}) noexcept
    {
        if (!is_empty(m_handle) && m_handle != h)
            Deleter()(m_handle);
        m_handle = h;
    }

    void swap(unique_handle &other) noexcept { std::swap(m_handle, other.m_handle); }

private:
    static constexpr bool is_empty(const handle_type &h) noexcept
    {
        return h == handle_type{};
    }

    handle_type m_handle{};
};

template <class T, class D>
inline void swap(unique_handle<T, D> &a, unique_handle<T, D> &b) noexcept
{
    a.swap(b);
}

} // namespace gce
