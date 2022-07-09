cmake_minimum_required(VERSION 3.11)

include(FetchContent)

project(MapGenerator)

set(CMAKE_CXX_STANDARD 11)

# SFML
set(SFML_BUILD_AUDIO FALSE)
set(SFML_BUILD_NETWORK FALSE)
set(SFML_VERSION "2.5.1")
FetchContent_Declare(
    sfml
    GIT_REPOSITORY "https://github.com/SFML/SFML.git"
    GIT_TAG "${SFML_VERSION}"
)
FetchContent_MakeAvailable(sfml)

# glm
add_subdirectory(external/glm)
include_directories(external/glm)

# mapgen
add_subdirectory(src)
include_directories(src)
