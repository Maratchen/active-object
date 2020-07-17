# automatically enable catch2 to generate ctest targets
if(CONAN_CATCH2_ROOT_DEBUG)
  include(${CONAN_CATCH2_ROOT_DEBUG}/lib/cmake/Catch2/Catch.cmake)
else()
  include(${CONAN_CATCH2_ROOT}/lib/cmake/Catch2/Catch.cmake)
endif()

add_library(catch2_main STATIC
    main.cpp)

target_link_libraries(catch2_main PRIVATE
    CONAN_PKG::catch2
)

add_executable(tests
    thread_executor_tests.cpp
)

target_link_libraries(tests PRIVATE
    CONAN_PKG::catch2
    catch2_main
    active
)

catch_discover_tests(tests)