cmake_minimum_required(VERSION 3.8)

project(active_object)

add_subdirectory(src)
add_subdirectory(test)

enable_testing()
add_test(smoke_test Test)
