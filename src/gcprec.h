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
#ifndef GVEPREC_H_INLUDED
#define GVEPREC_H_INLUDED


#define wxNO_UNSAFE_WXSTRING_CONV
// wxWidgets built in setup
#include <wx/wx.h>

#include <boost/core/noncopyable.hpp>
#include <fmt/format.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>

#if TARGET_OS_MAC
#define GL_SILENCE_DEPRECATION
#include <openGL/gl3.h>
#include <openGL/gl3ext.h>
#define __gl_h_ // suppress inclugion of old gl.h

#define glewInit()
const int GLEW_VERSION_3_1 = 1;
const int GLEW_VERSION_1_4 = 1;
#else
#include "GL/glew.h"
#endif

#define gceDEBUG

#include <boost/uuid/uuid.hpp>

#include "cxxcompat.h"

namespace gce
{
using boost::uuids::uuid;

namespace make_uuid_details
{
constexpr const size_t short_guid_form_length = 36;	// XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX
constexpr const size_t long_guid_form_length = 38;	// {XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX}

//
constexpr uint8_t parse_hex_digit(const char c)
{
    if ('0' <= c && c <= '9')
    {
        return c - '0';
    }
    else if ('a' <= c && c <= 'f')
    {
        return 10 + c - 'a';
    }
    else if ('A' <= c && c <= 'F')
    {
        return 10 + c - 'A';
    }
    else
    {
        throw std::domain_error{"invalid character in GUID"};
    }
}

constexpr uint8_t parse_hex_byte(const char *ptr)
{
    return 16 * parse_hex_digit(ptr[0]) + parse_hex_digit(ptr[1]);
}

constexpr gce::uuid make_uuid_helper(const char *begin)
{
    gce::uuid result;
    size_t offsets[16] = {0, 2, 4, 6, 9, 11, 14, 16, 19, 21, 24, 26, 28, 30, 32, 34};
    for (int i = 0; i < 16; i++)
    {
        result.data[i] = parse_hex_byte(begin + offsets[i]);
    }
    return result;
}

template<size_t N>
constexpr gce::uuid make_uuid(const char(&str)[N])
{
    static_assert(N == (long_guid_form_length + 1) || N == (short_guid_form_length + 1), "String GUID of the form {XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX} or XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX is expected");

    if constexpr (N == (long_guid_form_length + 1))
    {
        if (str[0] != '{' || str[long_guid_form_length - 1] != '}')
        {
            throw std::domain_error{"Missing opening or closing brace"};
        }
        return make_uuid_helper(str + 1);
    }
    else
    {
        return make_uuid_helper(str);
    }
}
}
using make_uuid_details::make_uuid;

template <class T, class...X>
struct is_any
{
    static const bool value = (... || std::is_same_v<T, X>);
};
template< class T, class...X >
constexpr bool is_any_v = is_any<T, X...>::value;

}

template <typename T, typename F>
T set_flag(T data, const F flag, bool val)
{
    return val ? data | flag : data & (~flag);
}

template <typename T, typename F>
bool has_flag(T data, const F flag)
{
    return (data & static_cast<T>(flag)) != 0;
}

inline std::string to_string(const wxString &s)
{
    return s.utf8_string();
}
inline wxString to_wxstring(const std::string &s)
{
    return wxString::FromUTF8(s);
}

inline wxString to_wxstring(const std::string_view &s)
{
    return wxString::FromUTF8(s.data(), s.size());
}

inline wxString to_wxstring(const char *s)
{
    return wxString::FromUTF8(s);
}

#endif
