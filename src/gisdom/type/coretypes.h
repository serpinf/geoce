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
#include "typeschema.hpp"

namespace gce
{
constexpr inline auto raster_schema_id = gce::make_uuid("{10DA8234-00CC-45E4-AC60-044C671C8F6D}");

constexpr inline auto geometry_schema_id = gce::make_uuid("{1FCDD773-E6BF-4517-8D3A-F844D251834C}");

std::unique_ptr<const gceTypeSchema> fenceSchema();

std::unique_ptr<const gceTypeSchema> buildingSchema();

std::unique_ptr<const gceTypeSchema> treeSchema();

std::unique_ptr<const gceTypeSchema> rasterSchema();

std::unique_ptr<const gceTypeSchema> geometrySchema();
}
