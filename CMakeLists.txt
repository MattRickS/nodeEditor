cmake_minimum_required(VERSION 3.11)

include(FetchContent)

project(MapGenerator LANGUAGES CXX)

set(BUILD_SHARED_LIBS OFF)
set(CMAKE_CXX_STANDARD 11)

find_package(OpenGL REQUIRED)

find_program(CCACHE_FOUND ccache)

if(CCACHE_FOUND)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
endif(CCACHE_FOUND)

# GLM
message("Fetching GLM...")
set(GLM_VERSION "0.9.9.8")
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG "${GLM_VERSION}"
)
FetchContent_MakeAvailable(glm)

# GLEW
message("Fetching GLEW...")
set(GLEW_VERSION "2.2.0")
FetchContent_Declare(
    glew
    GIT_REPOSITORY "https://github.com/nigels-com/glew.git"
    GIT_TAG "glew-${GLEW_VERSION}"
)
FetchContent_MakeAvailable(glew)

# GLFW
message("Fetching GLFW...")
set(GLFW_VERSION "3.3.7")
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
    glfw
    GIT_REPOSITORY "https://github.com/glfw/glfw.git"
    GIT_TAG "${GLFW_VERSION}"
)
FetchContent_MakeAvailable(glfw)

# ImGui
message("Fetching ImGui...")
set(IMGUI_VERSION "1.88")
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui
    GIT_TAG "v${IMGUI_VERSION}"
)
FetchContent_MakeAvailable(imgui)
add_library(imgui STATIC)
target_sources(imgui
    PRIVATE
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/imgui.cpp

    PRIVATE
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
)
target_include_directories(imgui
    PUBLIC ${imgui_SOURCE_DIR}
    PUBLIC ${imgui_SOURCE_DIR}/backends
    PUBLIC ${SDL2_DIR}/include
)
target_link_libraries(imgui GL)

# mapgen
add_subdirectory(src)
include_directories(src)

# tests
add_subdirectory(tests)
include_directories(tests)
