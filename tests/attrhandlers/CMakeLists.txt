cmake_minimum_required(VERSION 3.16)

project(test_attrhandlers LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Test)

# Create test executable for AppUuidAttr
add_executable(test_appuuidattr
    test_appuuidattr.cpp
)

target_link_libraries(test_appuuidattr
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Test
    qtlogger
)

target_include_directories(test_appuuidattr PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src
)

# Add tests to CTest
add_test(NAME AppUuidAttrTest COMMAND test_appuuidattr)
