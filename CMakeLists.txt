cmake_minimum_required(VERSION 3.11)

include(FetchContent)

project(MlaFw
    VERSION 0.1
    DESCRIPTION "Simple event framework implementation (header-only)"
    LANGUAGES CXX
    )

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)

# Enable testing
enable_testing()

FetchContent_Declare(
    concurrentqueue
    GIT_REPOSITORY https://github.com/cameron314/concurrentqueue.git
    GIT_TAG        3747268264d0fa113e981658a99ceeae4dad05b7
)

FetchContent_MakeAvailable(concurrentqueue)

add_subdirectory(include)
add_subdirectory(tests)

# Add this line to generate test configuration
include(CTest)
