file(GLOB_RECURSE TESTS_HEADERS "../src/mapgen/*.hpp")
file(GLOB_RECURSE TESTS_SOURCES "../src/mapgen/*.cpp")
list(APPEND TESTS_SOURCES "test_nodegraph.cpp")

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# TODO: Generate a header dynamically from discovered operators
if(CMAKE_COMPILER_IS_GNUCXX)
    message(STATUS "GCC detected, adding compile flags")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic -Wall -Wextra")
endif(CMAKE_COMPILER_IS_GNUCXX)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx -mavx2 -mfma")

add_executable(tests ${TESTS_HEADERS} ${TESTS_SOURCES})
add_dependencies(tests glm)
target_link_libraries(tests PRIVATE glfw GLEW GL imgui)
target_compile_features(tests PRIVATE cxx_std_17)
