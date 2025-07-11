add_executable(pattern_formatter
    main.cpp
)

target_compile_features(pattern_formatter PRIVATE cxx_std_17)

target_link_libraries(pattern_formatter
    PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Concurrent
        qtlogger
)

set_target_properties(pattern_formatter PROPERTIES
    FOLDER "examples"
)
