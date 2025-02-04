cmake_minimum_required(VERSION 3.16)

project(test_filters LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Test)

# Create test executable for CategoryFilter
add_executable(test_categoryfilter
    test_categoryfilter.cpp
    ../logmessage/mock_context.h
)

target_link_libraries(test_categoryfilter
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Test
    qtlogger
)

target_include_directories(test_categoryfilter PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src
    ${CMAKE_CURRENT_SOURCE_DIR}/../logmessage
)

# Create test executable for DuplicateFilter
add_executable(test_duplicatefilter
    test_duplicatefilter.cpp
    ../logmessage/mock_context.h
)

target_link_libraries(test_duplicatefilter
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Test
    qtlogger
)

target_include_directories(test_duplicatefilter PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src
    ${CMAKE_CURRENT_SOURCE_DIR}/../logmessage
)

# Create test executable for FunctionFilter
add_executable(test_functionfilter
    test_functionfilter.cpp
    ../logmessage/mock_context.h
)

target_link_libraries(test_functionfilter
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Test
    qtlogger
)

target_include_directories(test_functionfilter PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src
    ${CMAKE_CURRENT_SOURCE_DIR}/../logmessage
)

# Create test executable for RegExpFilter
add_executable(test_regexpfilter
    test_regexpfilter.cpp
    ../logmessage/mock_context.h
)

target_link_libraries(test_regexpfilter
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Test
    qtlogger
)

target_include_directories(test_regexpfilter PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src
    ${CMAKE_CURRENT_SOURCE_DIR}/../logmessage
)

# Add tests to CTest
add_test(NAME CategoryFilterTest COMMAND test_categoryfilter)
add_test(NAME DuplicateFilterTest COMMAND test_duplicatefilter)
add_test(NAME FunctionFilterTest COMMAND test_functionfilter)
add_test(NAME RegExpFilterTest COMMAND test_regexpfilter)
