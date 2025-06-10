cmake_minimum_required(VERSION 3.16)
project(SpaceTime)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)

set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)
set(EXTERNAL_DIR ${CMAKE_SOURCE_DIR}/external)
set(IMGUI_DIR ${EXTERNAL_DIR}/imgui)

set(SRC_FILES
    ${SRC_DIR}/main.cpp
    ${SRC_DIR}/CelestialBody.cpp
    ${SRC_DIR}/PhysicsEngine.cpp
    ${SRC_DIR}/Renderer.cpp
    ${SRC_DIR}/GravityWell.cpp
    ${EXTERNAL_DIR}/glad/glad.c
)

# ───── ImGui Sources ─────
set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)

add_library(imgui STATIC ${IMGUI_SOURCES})

target_include_directories(imgui PUBLIC
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)

target_link_libraries(imgui PRIVATE glfw)

# ───── Main Executable ─────
add_executable(SpaceTime ${SRC_FILES})

target_include_directories(SpaceTime PRIVATE
    ${SRC_DIR}
    ${EXTERNAL_DIR}
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)

target_link_libraries(SpaceTime PRIVATE
    glfw
    imgui
    OpenGL::GL
    glm::glm
)

target_compile_definitions(SpaceTime PRIVATE GLFW_INCLUDE_NONE)

# ───── Asset Copying ─────
file(GLOB_RECURSE SHADERS "${SRC_DIR}/shaders/*.vert" "${SRC_DIR}/shaders/*.frag" "${SRC_DIR}/shaders/*.comp")
file(GLOB_RECURSE TEXTURES "${SRC_DIR}/textures/*")

add_custom_target(copy-assets ALL
    DEPENDS ${SHADERS} ${TEXTURES}
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${SRC_DIR}/shaders ${CMAKE_CURRENT_BINARY_DIR}/shaders
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${SRC_DIR}/textures ${CMAKE_CURRENT_BINARY_DIR}/textures
)

add_dependencies(SpaceTime copy-assets)
