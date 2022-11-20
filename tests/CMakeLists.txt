# Find dependencies
find_package(benchmark REQUIRED)
find_package(GTest REQUIRED)

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
    gtest
    gtest_main
    pthread
    )

add_executable(logtests
    logtest.cpp
    )

target_link_libraries(logtests
    mlafw
    gtest
    gtest_main
    pthread
    )

add_executable(eventthreadtest
    eventthreadtest.cpp
    )

target_link_libraries(eventthreadtest
    mlafw
    gtest
    gtest_main
    pthread
    )

add_executable(timertest
    timertest.cpp
    )

target_link_libraries(timertest
    mlafw
    gtest
    gtest_main
    pthread
    )

add_executable(benchmark
    benchmark.cpp
    )

target_link_libraries(benchmark
    mlafw
    benchmark::benchmark
    )

# Add unit test files here
add_executable(eventtest
    eventtest.cpp
    )

target_link_libraries(eventtest
    mlafw
    gtest
    gtest_main
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
  NAME timestamp
  COMMAND $<TARGET_FILE:timertest>
  )

