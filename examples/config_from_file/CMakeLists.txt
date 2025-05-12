add_executable(config_from_file
    main.cpp
)

target_compile_features(config_from_file PRIVATE cxx_std_17)

target_link_libraries(config_from_file
    PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Concurrent
        qtlogger
)

set_target_properties(config_from_file PROPERTIES
    FOLDER "examples"
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.ini ${CMAKE_CURRENT_BINARY_DIR}/config.ini COPYONLY)
