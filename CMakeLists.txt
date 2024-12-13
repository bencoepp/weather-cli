cmake_minimum_required(VERSION 3.30)
project(weather_cli)

set(CMAKE_CXX_STANDARD 23)

set(SQLITECPP_RUN_CPPCHECK OFF CACHE BOOL "" FORCE)
set(SQLITECPP_RUN_CPPLINT OFF CACHE BOOL "" FORCE)
add_subdirectory(SQLiteCpp)
add_subdirectory(Catch2)

add_executable(weather_cli main.cpp
        Station.cpp
        Station.h
        Measurement.cpp
        Measurement.h
        barkeep.h
        SQLiteHandler.cpp
        SQLiteHandler.h
        WeatherHandler.cpp
        WeatherHandler.h
        QueryHandler.cpp
        QueryHandler.h
        tabulate.h)

target_link_libraries(weather_cli SQLiteCpp
        Catch2::Catch2)

add_executable(tests simple-test.cpp)
target_link_libraries(tests PRIVATE Catch2::Catch2WithMain)

include(CTest)
include(Catch)
catch_discover_tests(tests)
