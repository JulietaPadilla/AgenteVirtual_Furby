cmake_minimum_required(VERSION 3.16)
project(AgenteVirtualFurby LANGUAGES CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)

add_executable(AgenteVirtualFurby
    src/main.cpp
)

target_include_directories(AgenteVirtualFurby PRIVATE include)
target_link_libraries(AgenteVirtualFurby PRIVATE sfml-graphics sfml-window sfml-system)