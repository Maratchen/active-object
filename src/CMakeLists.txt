add_library(active_object INTERFACE)
target_sources(active_object INTERFACE active_object.hpp)
target_compile_features(active_object INTERFACE cxx_std_11)
target_include_directories(active_object INTERFACE "${PROJECT_SOURCE_DIR}/src")