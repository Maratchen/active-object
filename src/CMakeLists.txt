add_library(active_object INTERFACE)

target_include_directories(active_object INTERFACE
    "${PROJECT_SOURCE_DIR}/src/"
)

target_compile_features(active_object INTERFACE
    cxx_std_11
)