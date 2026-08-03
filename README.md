###geoce
Geographic Client Engine
An open source virtual Globe map editor/viewer with 3D and projection modes, designed to provide high performance and extensibility.

###About
This project's intension is to provide a native open source solution for highly efficient creation and rendering of cartographic data.
allowing to combine data from multiple sources: web maps, remote or local spatial database(s).

Open architecture is designed to split data source, processsing logic and redering abstractions:
- database table: data schema
- model: buisness logic implementation and 2D/3D models
- tech: renndering technique level

###Plans
- provide sample project
- support sqlite3 database for geometry data
- map objects rendering/editing in Globe mode
- linux support

###Dependencies
You will need MS Visual Studio 2022 Community or later with vcpkg installed
Use vcpkg to install the following libraries:
wxWidgets, boost, glew, GEOS, GDAL, PostgreSQL (for libpq), sqlite3, glm, GeographicLib, fmt, gtest
Building (Windows only, other platforms build files are work in progress)
Clone the repository, use FreeGVE.sln to buil sample project and tests
Usage
Run the App, create a new workspace, add datasource, models and scenes.
As of now you need a PostgreSQL server with PostGIS installed to work with vector data and a web map access to view 3D Globe

###Contributing
Guidelines for how others can contribute to your C++ project.
1. Fork the repository.
2. Create a new branch for your feature or bug fix.
3. Make your changes and ensure they compile and pass tests.
4. Commit your changes with clear messages.
5. Push your branch to your fork.
6. Open a pull request against the main branch of the original repository.

###License
This project is licensed under the Apache 2.0 License.
