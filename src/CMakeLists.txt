file(GLOB_RECURSE MAPGEN_HEADERS "*.hpp")
file(GLOB_RECURSE MAPGEN_SOURCES "*.cpp")

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

if(CMAKE_COMPILER_IS_GNUCXX)
    message(STATUS "GCC detected, adding compile flags")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic -Wall -Wextra")
endif(CMAKE_COMPILER_IS_GNUCXX)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx -mavx2 -mfma")

add_executable(mapgen ${MAPGEN_HEADERS} ${MAPGEN_SOURCES})
add_dependencies(mapgen glm)
