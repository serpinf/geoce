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
#include "coretypes.h"

static constexpr char fence_svg[] = R"rawsvg(<svg xmlns="http://www.w3.org/2000/svg" width="200" height="200" viewBox="0 0 1024 1024"><path fill="currentColor" d="M992 768q13 0 22.5 9.5t9.5 22.5v64q0 13-9.5 22.5T992 896h-32v96q0 13-9.5 22.5T928 1024H800q-13 0-22.5-9.5T768 992v-96H640v96q0 13-9.5 22.5T608 1024H480q-13 0-22.5-9.5T448 992v-96H320v96q0 13-9.5 22.5T288 1024H160q-13 0-22.5-9.5T128 992v-96H32q-13 0-22.5-9.5T0 864v-64q0-13 9.5-22.5T32 768h96V384H32q-13 0-22.5-9.5T0 352v-64q0-13 9.5-22.5T32 256h96v-96q0-40 5-50L211 7q5-7 12.5-7T237 7l77 103q6 8 6 50v96h128v-96q0-37 5-50L531 7q5-7 12.5-7T557 7l78 103q5 8 5 50v96h128v-96q0-43 5-50L851 7q5-7 12.5-7T877 7l77 103q6 11 6 50v96h32q13 0 22.5 9.5t9.5 22.5v64q0 13-9.5 22.5T992 384h-32v384h32zM448 384H320v384h128V384zm320 0H640v384h128V384z"/></svg>)rawsvg";
std::unique_ptr<const gceTypeSchema> gce::fenceSchema()
{
    return std::make_unique<const gceTypeSchema>(
        gce::make_uuid("{6C9D4E0B-5685-4C3E-AD3E-007196FC0313}"), fence_svg, "fence",
        gceColumnSchema(0, gceColumnType::int64, "id_fence").PKEY().Serial(),
        gceColumnSchema(1, gceColumnType::geometry, "geom").geometryDimension(1).coordinateType(geom::CoordinateType::XY),
        gceColumnSchema(2, gceColumnType::string, "name").Default(std::string("Some fence")).Property(),
        gceColumnSchema(3, gceColumnType::float32, "height").Default(2.0f).Property()
    );
}

static constexpr char building_svg[] = R"rawsvg(<svg xmlns="http://www.w3.org/2000/svg" width="200" height="200" viewBox="0 0 14 14"><path fill="none" stroke="currentColor" stroke-linecap="round" stroke-linejoin="round" d="M8.5 13.5h-8V4l4-3.5l4 3.5zm0 0h5v-7h-5m-4 7v-2M3 8.5h3m-3-3h3"/></svg>)rawsvg";
std::unique_ptr<const gceTypeSchema> gce::buildingSchema()
{
    return std::make_unique<const gceTypeSchema>(
        gce::make_uuid("{1FA4DD06-D812-497C-8665-9B5FFD1D69BA}"), building_svg, "building",
        gceColumnSchema(0, gceColumnType::int64, "id_building").PKEY().Serial(),
        gceColumnSchema(1, gceColumnType::geometry, "geom").geometryDimension(2).coordinateType(geom::CoordinateType::XY),
        gceColumnSchema(2, gceColumnType::string, "name").Default(std::string("building name")).Property(),
        gceColumnSchema(3, gceColumnType::int32, "floors").Default(1).Property()
    );
}

static constexpr char tree_svg[] = R"rawsvg(<svg xmlns="http://www.w3.org/2000/svg" width="200" height="200" viewBox="0 0 48 48"><g fill="none" stroke="#000" stroke-linecap="round" stroke-linejoin="round" stroke-width="4"><path d="M13.0448 14C13.5501 8.3935 18.262 4 24 4C29.738 4 34.4499 8.3935 34.9552 14H35C39.9706 14 44 18.0294 44 23C44 27.9706 39.9706 32 35 32H13C8.02944 32 4 27.9706 4 23C4 18.0294 8.02944 14 13 14H13.0448Z"/><path d="M24 28L29 23"/><path d="M24 25L18 19"/><path d="M24 44V18"/></g></svg>)rawsvg";
std::unique_ptr<const gceTypeSchema> gce::treeSchema()
{
    return std::make_unique<const gceTypeSchema>(
        gce::make_uuid("{3A4B5A31-B292-483C-AA83-56B4617AE036}"), tree_svg, "tree",
        gceColumnSchema(0, gceColumnType::int64, "id_tree").PKEY().Serial(),
        gceColumnSchema(1, gceColumnType::geometry, "geom").geometryDimension(0).coordinateType(geom::CoordinateType::XY),
        gceColumnSchema(2, gceColumnType::string, "type").Property(),
        gceColumnSchema(3, gceColumnType::float32, "height").Property()
    );
}

static constexpr char raster_svg[] = R"rawsvg(<svg xmlns="http://www.w3.org/2000/svg" width="200" height="200" viewBox="0 0 20 20"><path fill="currentColor" d="M0 0h9v9H0V0zm2 2v5h5V2H2zm-2 9h9v9H0v-9zm2 2v5h5v-5H2zm9-13h9v9h-9V0zm2 2v5h5V2h-5zm-2 9h9v9h-9v-9zm2 2v5h5v-5h-5z"/></svg>)rawsvg";
std::unique_ptr<const gceTypeSchema> gce::rasterSchema()
{
    return std::make_unique<const gceTypeSchema>(
        raster_schema_id, raster_svg, "raster",
        gceColumnSchema(0, gceColumnType::int64, "id_raster").PKEY(),
        gceColumnSchema(1, gceColumnType::string, "format"),
        gceColumnSchema(2, gceColumnType::string, "data")
    );
}

static constexpr char geometry_svg[] = R"rawsvg(<svg xmlns="http://www.w3.org/2000/svg" width="48" height="48" viewBox="0 0 48 48"><title>Geogebra-geometry SVG Icon</title><circle cx="17.069" cy="30.672" r="12.98" fill="none" stroke="currentColor" stroke-linecap="round" stroke-linejoin="round"/><path fill="none" stroke="currentColor" stroke-linecap="round" stroke-linejoin="round" d="M29.123 4.348L14.335 29.962h29.576z"/></svg>)rawsvg";
std::unique_ptr<const gceTypeSchema> gce::geometrySchema()
{
    return std::make_unique<const gceTypeSchema>(
        geometry_schema_id, geometry_svg, "geometry",
        gceColumnSchema(0, gceColumnType::int64, "id_geom").PKEY(),
        gceColumnSchema(1, gceColumnType::geometry, "geom").geometryDimension(-1).coordinateType(geom::CoordinateType::XY)
    );
}
