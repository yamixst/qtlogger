add_executable(file_rotation
    main.cpp
)

target_compile_features(file_rotation PRIVATE cxx_std_17)

target_link_libraries(file_rotation
    PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Concurrent
        qtlogger
)

set_target_properties(file_rotation PROPERTIES
    FOLDER "examples"
)
