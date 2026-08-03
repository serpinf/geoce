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

class gceDataConnection;

using gceDatasourceFactoryFn = std::unique_ptr<gceDataConnection>(*)(const std::string &connStr);

class gceDatasourceSchema final
{
public:
    gceDatasourceSchema() = default;
    ~gceDatasourceSchema() = default;

    std::string m_name;
    std::string m_svgIcon;
    gceDatasourceFactoryFn m_factory = nullptr;
    std::string m_defaultParams;
    bool canCreateTables = false;
};
