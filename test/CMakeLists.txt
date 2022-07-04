find_package(Catch2)

add_library(catch2_main STATIC
    main.cpp)

target_link_libraries(catch2_main PUBLIC
    Catch2::Catch2
)

add_executable(tests
    thread_executor_tests.cpp
)

target_link_libraries(tests PRIVATE
    catch2_main
    active
)

include(CTest)
include(Catch)

# automatically enable catch2 to generate ctest targets
catch_discover_tests(tests)