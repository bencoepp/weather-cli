cmake_minimum_required(VERSION 3.30)
project(weather_cli)

set(CMAKE_CXX_STANDARD 23)

add_executable(weather_cli main.cpp
        Station.cpp
        Station.h
        Measurement.cpp
        Measurement.h)