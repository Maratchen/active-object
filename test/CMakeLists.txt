add_executable(smoke_test main.cpp)
target_link_libraries(smoke_test active_object)
add_test(smoke_test smoke_test)