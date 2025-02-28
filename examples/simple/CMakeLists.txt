add_executable(simple
    main.cpp
)

target_compile_features(simple PRIVATE cxx_std_17)

target_link_libraries(simple
    PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Concurrent
        qtlogger
)

set_target_properties(simple PROPERTIES
    FOLDER "examples"
)
