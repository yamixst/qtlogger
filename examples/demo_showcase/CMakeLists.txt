add_executable(demo_showcase
    main.cpp
)

target_compile_features(demo_showcase PRIVATE cxx_std_17)

target_link_libraries(demo_showcase
    PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Concurrent
        qtlogger
)

set_target_properties(demo_showcase PROPERTIES
    FOLDER "examples"
)
