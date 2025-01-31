cmake_minimum_required(VERSION 3.16)

project(test_ownthreadhandler LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Test)

# Create test executable only if threading is enabled
if(NOT QTLOGGER_NO_THREAD)
    add_executable(test_ownthreadhandler
        test_ownthreadhandler.cpp
        mock_handlers.h
    )

    target_link_libraries(test_ownthreadhandler
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Test
        qtlogger
    )

    target_include_directories(test_ownthreadhandler PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../../src
    )

    # Add test to CTest
    add_test(NAME OwnThreadHandlerTest COMMAND test_ownthreadhandler)
else()
    message(STATUS "OwnThreadHandler tests skipped (QTLOGGER_NO_THREAD=ON)")
endif()
