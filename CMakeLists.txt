cmake_minimum_required(VERSION 3.8)

project(MlaFw
    VERSION 0.1
    DESCRIPTION "Simple event framework implementation"
    LANGUAGES CXX
    )

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)

add_subdirectory(src)
add_subdirectory(tests)
