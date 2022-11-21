file(GLOB_RECURSE NODEEDITOR_HEADERS "*.hpp")
file(GLOB_RECURSE NODEEDITOR_SOURCES "*.cpp")

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# TODO: Generate a header dynamically from discovered operators
if(CMAKE_COMPILER_IS_GNUCXX)
    message(STATUS "GCC detected, adding compile flags")

    # Missing field initializers needed for stb_image_write, see https://github.com/nothings/stb/issues/1099
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic -Wall -Wextra -Wno-missing-field-initializers")
endif(CMAKE_COMPILER_IS_GNUCXX)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx -mavx2 -mfma")

add_executable(nodeeditor ${NODEEDITOR_HEADERS} ${NODEEDITOR_SOURCES})
add_dependencies(nodeeditor glm)
target_link_libraries(nodeeditor PRIVATE glfw GLEW GL imgui)
target_compile_features(nodeeditor PRIVATE cxx_std_17)
