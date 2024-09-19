# Fetch googletest
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        58d77fa8070e8cec2dc1ed015d66b454c8d78850
)
FetchContent_MakeAvailable(googletest)

# Fetch google benchmark
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

add_executable(tests
    tests.cpp
    )

target_link_libraries(tests
    mlafw
    GTest::gtest_main
    pthread
    )

add_executable(utiltest
    attributetupletest.cpp
    )

target_link_libraries(utiltest
    mlafw
    GTest::gtest_main
    )

add_executable(logtests
    logtest.cpp
    )

target_link_libraries(logtests
    mlafw
    GTest::gtest_main
    pthread
    )

add_executable(eventthreadtest
    eventthreadtest.cpp
    )

target_link_libraries(eventthreadtest
    mlafw
    GTest::gtest_main
    pthread
    )

add_executable(timertest
    timertest.cpp
    )

target_link_libraries(timertest
    mlafw
    GTest::gtest_main
    pthread
    )

add_executable(benchmarks
    benchmark.cpp
    )

target_link_libraries(benchmarks
    mlafw
    benchmark::benchmark
    )

# Add unit test files here
add_executable(eventtest
    eventtest.cpp
    )

target_link_libraries(eventtest
    mlafw
    GTest::gtest_main
    pthread
    )

add_test(
  NAME eventtest
  COMMAND $<TARGET_FILE:eventtest>
  )

add_test(
  NAME eventthreadtest
  COMMAND $<TARGET_FILE:eventthreadtest>
  )

add_test(
  NAME timertest
  COMMAND $<TARGET_FILE:timertest>
  )

