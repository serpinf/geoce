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
#include "typeschema.hpp"
#include "entity.h"
#include "entitypck.h"

void gceTypeSchema::parseColumns()
{
    for (auto &col : m_columns)
    {
        // TODO: support compound pkeys ?
        if (col.isPKEY())
        {
            m_keyIndex = col.getIndex();
        }
        // assume geometry index is not 0 and it is the only geometry
        if (!m_geometryIndex && isGeometry(col.getType()))
        {
            m_geometryIndex = col.getIndex();
        }
    }
}

struct copy_value2
{
    const gceColumnValue &src;

    template <class T> gceColumnValue operator()()
    {
        if constexpr (gce::is_any_v<T, int16_t, int32_t, int64_t, float, double, std::string>)
        {
            if (std::holds_alternative<T>(src))
            {
                return std::get<T>(src);
            }
        }
        return {};
    }
};
gceEntityVar gceTypeSchema::createDefaultEntityVar() const
{
    gceEntityVar rec{this};
    for (auto &col : m_columns)
    {
        rec[col.getIndex()] = gce::visit_column(copy_value2{col.getDefault()}, col.getType());
    }
    return rec;
}

gceEntityPacked gceTypeSchema::createDefaultEntityPacked() const
{
    return gceEntityPacked{createDefaultEntityVar()};
}
static constexpr char int_svg[] = R"rawsvg(<svg xmlns="http://www.w3.org/2000/svg" width="512" height="512" viewBox="0 0 512 512"><title>Data-type-integer SVG Icon</title><path fill="currentColor" fill-rule="evenodd" d="M166.521 298.666v-30.781q43.906-40.312 66.875-65.781q21.094-23.281 29.062-37.344q8.125-14.063 8.125-27.656q0-14.376-9.843-22.969q-11.094-9.531-27.969-9.531q-23.125 0-53.125 16.718l-9.375-32.187q32.969-15.47 66.719-15.469q32.812 0 51.875 14.844q20.781 16.25 20.781 46.406q0 20.313-9.531 38.75t-33.907 45q-21.406 23.593-48.593 48.75h93.75v31.25zm-61.75 85.333V205.562l-40.469 27.187l-16.562-30.468l61.25-39.688h33.437v221.406zm217.177-18.354l-8.125 31.719q30.156 11.719 63.437 11.719q37.657 0 57.969-16.563q22.656-18.436 22.656-50.469q0-45-47.656-52.031q22.5-4.218 33.906-17.344q11.407-13.28 11.407-35.156q0-27.812-19.844-42.812q-18.75-14.375-52.188-14.375q-32.5 0-62.968 11.875l7.5 31.25q29.375-12.188 50.625-12.188q17.812 0 27.812 7.969q10 7.812 10 22.031q0 16.563-15.469 26.094q-13.437 8.437-42.5 8.437q-6.405 0-14.687-.781v30.156q11.406-.624 18.437-.625q56.563 0 56.563 36.25q0 15.157-9.688 25.469q-11.093 11.875-34.531 11.875q-25.938 0-52.656-12.5" clip-rule="evenodd"/></svg>)rawsvg";
static constexpr char float_svg[] = R"rawsvg(<svg xmlns="http://www.w3.org/2000/svg" width="512" height="512" viewBox="0 0 512 512"><title>Data-type-double SVG Icon</title><path fill="currentColor" fill-rule="evenodd" d="M276.625 384v-36.938q52.688-48.375 80.25-78.937q25.313-27.938 34.875-44.813q9.75-16.875 9.75-33.187q0-17.25-11.813-27.563q-13.312-11.436-33.562-11.437q-27.75 0-63.75 20.063l-11.25-38.626q39.562-18.561 80.062-18.562q39.375 0 62.25 17.812q24.938 19.5 24.938 55.688q0 24.375-11.438 46.5q-11.437 22.125-40.687 54q-25.688 28.313-58.313 58.5h112.5V384zm-155.167 0V169.875L72.896 202.5L53.02 165.938l73.5-47.626h40.125V384zm117.209-41.667q-7.5-7.666-18.834-7.666q-11.166 0-18.833 7.5q-7.5 7.5-7.5 18.666q0 11.834 7.333 19.5q7.5 7.5 19.167 7.5q11.166 0 18.667-7.5q7.5-7.667 7.5-19.166q0-11.334-7.5-18.834" clip-rule="evenodd"/></svg>)rawsvg";
static constexpr char string_svg[] = R"rawsvg(<svg xmlns="http://www.w3.org/2000/svg" width="512" height="512" viewBox="0 0 512 512"><title>Data-type-string SVG Icon</title><path fill="currentColor" d="M431.261 380.864q-11.778 13.44-23.716 18.956q-11.775 5.513-28.232 5.513q-26.134 0-40.169-14.818q-14.198-14.649-14.198-39.637q0-33.774 25.653-49.628q16.131-9.996 50.978-9.996q9.195 0 27.587 1.035v-3.274q0-40.842-37.266-40.841q-21.136 0-45.494 14.993l-10.324-29.641q29.36-15.853 60.817-15.853q40.816 0 58.4 23.435q7.099 9.653 9.68 22.402q2.743 12.752 2.743 38.256v53.766c0 19.07.538 38.313 1.613 49.801h-38.072zm-2.097-62.727q-9.519-.862-17.424-.862q-23.392 0-33.716 6.893q-13.07 8.79-13.068 25.849q0 12.237 7.098 19.817q7.098 7.411 19.037 7.411q22.101 0 38.073-25.159zm-206.202-1.205H119.38L96 405.333H42.667L144.69 106.666h53.738l110.904 298.667H256zm-13.24-40.349l-23.17-69.88a1199 1199 0 0 1-15.381-50.746q-5.841 21.837-15.577 50.746l-23.169 69.88z"/></svg>)rawsvg";
static constexpr char geometry_svg[] = R"rawsvg(<svg xmlns="http://www.w3.org/2000/svg" width="48" height="48" viewBox="0 0 48 48"><title>Geogebra-geometry SVG Icon</title><circle cx="17.069" cy="30.672" r="12.98" fill="none" stroke="currentColor" stroke-linecap="round" stroke-linejoin="round"/><path fill="none" stroke="currentColor" stroke-linecap="round" stroke-linejoin="round" d="M29.123 4.348L14.335 29.962h29.576z"/></svg>)rawsvg";

const char *gceColumnSchema::getTypeIcon() const
{
    switch (this->m_type)
    {
    case gceColumnType::int16:
    case gceColumnType::int32:
    case gceColumnType::int64:
        return int_svg;
    case gceColumnType::float32:
    case gceColumnType::float64:
        return float_svg;
    case gceColumnType::string:
        return string_svg;
    case gceColumnType::geometry:
        return geometry_svg;
    default:
        break;
    }
    return "";
}
