cmake_minimum_required(VERSION 3.16)

project(test_package LANGUAGES CXX)

find_package(qtlogger CONFIG REQUIRED)

add_executable(test_package test_package.cpp)
target_link_libraries(test_package PRIVATE qtlogger::qtlogger)

set_target_properties(test_package PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)
