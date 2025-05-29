add_executable(one_line_config
    main.cpp
)

target_compile_features(one_line_config PRIVATE cxx_std_17)

target_link_libraries(one_line_config
    PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Concurrent
        qtlogger
)

set_target_properties(one_line_config PROPERTIES
    FOLDER "examples"
)
