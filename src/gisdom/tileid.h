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

#include <cstdint>
#include <string>
#include <functional>

namespace gce
{
// storage layout (bits): [63..61]=zeros(3) | [60..56]=level(5) | [55..28]=ty(28) | [27..0]=tx(28)
// Note: tx and ty have 28 bits each. For a zoom `z` the valid tx/ty range is [0 .. 2^z - 1].
// `max_level()` returns 28 to reflect the maximum logical level where 2^level tiles fit into tx/ty.
class tileid
{
public:
    using value_type = uint64_t;

    // bit sizes
    static constexpr unsigned X_BITS = 28u;
    static constexpr unsigned Y_BITS = 28u;
    static constexpr unsigned Z_BITS = 5u; // storage for level (0..28)

    // shifts
    static constexpr unsigned X_SHIFT = 0u;
    static constexpr unsigned Y_SHIFT = X_SHIFT + X_BITS; // 28
    static constexpr unsigned Z_SHIFT = Y_SHIFT + Y_BITS; // 56


    // maximum logical level addressable by tx/ty
    static constexpr std::size_t max_level() noexcept { return X_BITS; } // 28

    // Constructors
    explicit constexpr tileid(value_type key) noexcept : m_key(key) {}
    constexpr tileid() noexcept = default;

    // Unchecked component constructor: components are masked into storage.
    constexpr tileid(uint32_t x, uint32_t y, uint8_t z) noexcept
        : m_key((value_type(z &((1u << Z_BITS) - 1)) << Z_SHIFT)
              | (value_type(y & ((1u << Y_BITS) - 1)) << Y_SHIFT)
              | (value_type(x & ((1u << X_BITS) - 1)) << X_SHIFT))
    {}

    // Accessors
    constexpr uint8_t get_z() const noexcept
    {
        return static_cast<uint8_t>((m_key >> Z_SHIFT) & ((1llu << Z_BITS) - 1));
    }
    constexpr uint32_t get_y() const noexcept
    {
        return static_cast<uint32_t>((m_key >> Y_SHIFT) & ((1llu << Y_BITS) - 1));
    }
    constexpr uint32_t get_x() const noexcept
    {
        return static_cast<uint32_t>((m_key >> X_SHIFT) & ((1llu << X_BITS) - 1));
    }
    constexpr value_type get_value() const noexcept { return m_key; }
    constexpr bool empty() const noexcept { return m_key == 0; }

#if has_cpp20
    // C++20 defaulted comparisons (total order on m_key)
    constexpr auto operator<=>(const tileid &other) const noexcept = default;
    constexpr bool operator==(const tileid &other) const noexcept = default;
#else
    constexpr bool operator==(const tileid &other) const noexcept { return m_key == other.m_key; }
    constexpr bool operator!=(const tileid &other) const noexcept { return m_key != other.m_key; }
    constexpr bool operator<(const tileid &other) const noexcept { return m_key < other.m_key; }
    constexpr bool operator<=(const tileid &other) const noexcept { return m_key <= other.m_key; }
    constexpr bool operator>(const tileid &other) const noexcept { return m_key > other.m_key; }
    constexpr bool operator>=(const tileid &other) const noexcept { return m_key >= other.m_key; }
#endif

    // Parent tile (z-1). Returns empty tileid if already at z==0.
    constexpr tileid parent() const noexcept
    {
        const uint8_t z = get_z();
        if (z == 0) return tileid{};
        return tileid(get_x() >> 1u, get_y() >> 1u, static_cast<uint8_t>(z - 1u));
    }

    //  1 | 2
    //  3 | 4
    // Returns a child tile at z+1. If z==max_level() returns this tileid.
    constexpr tileid get_part(int part) const noexcept
    {
        const uint8_t clev = get_z();
        if (clev == max_level())
        {
            return tileid(m_key);
        }
        const uint8_t lev = static_cast<uint8_t>(clev + 1u);
        const uint32_t x = get_x() * 2u;
        const uint32_t y = get_y() * 2u;
        switch (part)
        {
        case 1: return tileid(x, y + 1u, lev);
        case 2: return tileid(x + 1u, y + 1u, lev);
        case 3: return tileid(x, y, lev);
        case 4: return tileid(x + 1u, y, lev);
        default: return tileid{};
        }
    }

    // String for debugging: "z/x/y"
    std::string to_string() const
    {
        return std::to_string(get_z()) + "/" + std::to_string(get_x()) + "/" + std::to_string(get_y());
    }

private:
    value_type m_key = 0;
};

} // namespace gce

// Allow tileid in unordered containers
namespace std
{
template<>
struct hash<gce::tileid>
{
    size_t operator()(gce::tileid const &t) const noexcept
    {
        return std::hash<uint64_t>{}(t.get_value());
    }
};
}
