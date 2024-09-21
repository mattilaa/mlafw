# Fetch googletest

# Disable GoogleTest unit tests
set(BUILD_GMOCK OFF CACHE BOOL "Builds the googlemock subproject" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "Enable installation of googletest" FORCE)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        58d77fa8070e8cec2dc1ed015d66b454c8d78850
)
FetchContent_MakeAvailable(googletest)

# Fetch google benchmark

# Disable Google Benchmark tests
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "Disable benchmark testing" FORCE)
set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "Disable benchmark gtest testing" FORCE)

FetchContent_Declare(
    benchmark
    GIT_REPOSITORY https://github.com/google/benchmark.git
    GIT_TAG        d572f4777349d43653b21d6c2fc63020ab326db2
)
FetchContent_MakeAvailable(benchmark)

# Make 'release' as default build type
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O2")
set(CMAKE_CXX_FLAGS_RELEASE "-O2")

# Define test executables
set(TEST_EXECUTABLES
    attributetupletest
    logtest
    eventthreadtest
    timertest
)

# Create test targets
foreach(test_name ${TEST_EXECUTABLES})
    add_executable(${test_name} ${test_name}.cpp)
    target_link_libraries(${test_name}
        mlafw
        GTest::gtest_main
        pthread
    )
    add_test(NAME ${test_name} COMMAND ${test_name})
endforeach()

# Benchmarks (not a test, but keeping it here for consistency)
add_executable(benchmarks benchmark.cpp)
target_link_libraries(benchmarks
    mlafw
    benchmark::benchmark
)
