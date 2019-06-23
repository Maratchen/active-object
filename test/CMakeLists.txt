project(tests)
find_package(catch2 REQUIRED)

add_executable(testrunner
    thread_executor_tests.cpp
    main.cpp
)

target_link_libraries(testrunner PRIVATE
    catch2::catch2
    active
)

add_test(all_tests testrunner)