cmake_minimum_required(VERSION 3.30)
project(weather_cli)

set(CMAKE_CXX_STANDARD 23)

set(SQLITECPP_RUN_CPPCHECK OFF CACHE BOOL "" FORCE)
set(SQLITECPP_RUN_CPPLINT OFF CACHE BOOL "" FORCE)
add_subdirectory(SQLiteCpp)

add_executable(weather_cli main.cpp
        Station.cpp
        Station.h
        Measurement.cpp
        Measurement.h
        barkeep.h)

target_link_libraries(weather_cli SQLiteCpp)