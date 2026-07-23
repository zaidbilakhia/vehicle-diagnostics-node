cmake_minimum_required(VERSION 3.16)

project(vehicle-node-linux LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(vehicle-node
    src/main.cpp
    src/VehicleNode.cpp
    src/Config.cpp
    src/Logger.cpp
    src/SystemInfo.cpp
    src/CanCheck.cpp
)

target_include_directories(vehicle-node PRIVATE include)

install(TARGETS vehicle-node RUNTIME DESTINATION bin)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(vehicle-node PRIVATE -Wall -Wextra -Wpedantic)
endif()
