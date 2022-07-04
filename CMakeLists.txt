cmake_minimum_required(VERSION 3.10)

project(MapGenerator)

set(CMAKE_CXX_STANDARD 11)

# glm
add_subdirectory(external/glm)
include_directories(external/glm)

# mapgen
add_subdirectory(src)
include_directories(src)
