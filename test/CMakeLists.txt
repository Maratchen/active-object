project(tests)
find_package(catch2 REQUIRED)

add_executable(tests
    active_object_tests.cpp
    main.cpp
)
target_link_libraries(tests PRIVATE
    catch2::catch2
    active_object
)
add_test(smoke_test tests)