find_package(Catch2 3 REQUIRED)

add_executable(tests
    thread_executor_tests.cpp
)

target_link_libraries(tests PRIVATE
    Catch2::Catch2WithMain
    active
)

include(CTest)
include(Catch)

# automatically enable catch2 to generate ctest targets
catch_discover_tests(tests)