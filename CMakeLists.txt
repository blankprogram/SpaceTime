cmake_minimum_required(VERSION 3.16)
project(spacetime LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
endif()

find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)
find_package(OpenGL REQUIRED)
find_package(glad REQUIRED)
find_package(imgui REQUIRED)

set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)
set(IMGUI_BACKENDS_DIR ${CMAKE_SOURCE_DIR}/external/imgui_backends)

set(SRC_FILES
    ${SRC_DIR}/CelestialBody.cpp
    ${SRC_DIR}/GravityWell.cpp
    ${SRC_DIR}/PhysicsEngine.cpp
    ${SRC_DIR}/Renderer.cpp
    ${SRC_DIR}/main.cpp
    ${IMGUI_BACKENDS_DIR}/imgui_impl_glfw.cpp
    ${IMGUI_BACKENDS_DIR}/imgui_impl_opengl3.cpp
)

add_executable(spacetime ${SRC_FILES})

target_include_directories(spacetime PRIVATE
    ${SRC_DIR}
    ${IMGUI_BACKENDS_DIR}
)

target_link_libraries(spacetime PRIVATE
    glfw
    glad::glad
    imgui::imgui
    OpenGL::GL
    glm::glm
)

target_compile_definitions(spacetime PRIVATE
    GLFW_INCLUDE_NONE
    IMGUI_IMPL_OPENGL_LOADER_GLAD
)

file(GLOB_RECURSE SHADERS "${SRC_DIR}/shaders/*")
file(GLOB_RECURSE TEXTURES "${SRC_DIR}/textures/*")

add_custom_target(copy-assets ALL
    DEPENDS ${SHADERS} ${TEXTURES}
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${SRC_DIR}/shaders
            ${CMAKE_CURRENT_BINARY_DIR}/shaders
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${SRC_DIR}/textures
            ${CMAKE_CURRENT_BINARY_DIR}/textures
)

add_dependencies(spacetime copy-assets)
