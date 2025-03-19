add_executable(simple_config
    main.cpp
)

target_compile_features(simple_config PRIVATE cxx_std_17)

target_link_libraries(simple_config
    PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Concurrent
        qtlogger
)

set_target_properties(simple_config PROPERTIES
    FOLDER "examples"
)
