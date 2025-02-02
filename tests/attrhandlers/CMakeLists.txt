cmake_minimum_required(VERSION 3.16)

project(test_attrhandlers LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Test)

# Add network component if enabled
if(QTLOGGER_NETWORK)
    find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Network)
endif()

# Create test executable for AppInfoAttrs
add_executable(test_appinfoattrs
    test_appinfoattrs.cpp
    mock_qcoreapplication.h
)

target_link_libraries(test_appinfoattrs
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Test
    qtlogger
)

target_include_directories(test_appinfoattrs PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src
)

# Create test executable for SeqNumberAttr
add_executable(test_seqnumberattr
    test_seqnumberattr.cpp
)

target_link_libraries(test_seqnumberattr
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Test
    qtlogger
)

target_include_directories(test_seqnumberattr PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src
)

# Create test executable for HostInfoAttrs (only if network is enabled)
if(QTLOGGER_NETWORK)
    add_executable(test_hostinfoattrs
        test_hostinfoattrs.cpp
        mock_qhostinfo.h
    )

    target_link_libraries(test_hostinfoattrs
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Test
        Qt${QT_VERSION_MAJOR}::Network
        qtlogger
    )

    target_include_directories(test_hostinfoattrs PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../../src
    )

    target_compile_definitions(test_hostinfoattrs PRIVATE QTLOGGER_NETWORK)
endif()

# Add tests to CTest
add_test(NAME AppInfoAttrsTest COMMAND test_appinfoattrs)
add_test(NAME SeqNumberAttrTest COMMAND test_seqnumberattr)

if(QTLOGGER_NETWORK)
    add_test(NAME HostInfoAttrsTest COMMAND test_hostinfoattrs)
endif()

# Create test executable for base AttrHandler
add_executable(test_attrhandler_base
    test_attrhandler_base.cpp
)

target_link_libraries(test_attrhandler_base
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Test
    qtlogger
)

target_include_directories(test_attrhandler_base PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src
)

# Add base AttrHandler test to CTest
add_test(NAME AttrHandlerBaseTest COMMAND test_attrhandler_base)
