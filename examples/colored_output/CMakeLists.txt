add_executable(colored_output
    main.cpp
)

target_compile_features(colored_output PRIVATE cxx_std_17)

target_link_libraries(colored_output
    PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Concurrent
        qtlogger
)

set_target_properties(colored_output PROPERTIES
    FOLDER "examples"
)
