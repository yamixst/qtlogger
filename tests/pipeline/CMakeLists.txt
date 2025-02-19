cmake_minimum_required(VERSION 3.16)

project(test_pipeline LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Test)

# Create test executable
add_executable(test_pipeline
    test_pipeline.cpp
    mock_handler.h
)

target_link_libraries(test_pipeline
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Test
    qtlogger
)

target_include_directories(test_pipeline PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src
)

# Add test to CTest
add_test(NAME PipelineTest COMMAND test_pipeline)
