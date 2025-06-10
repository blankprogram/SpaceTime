
cmake_minimum_required(VERSION 3.16)
project(SpaceTimeCube)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)

# ───── GLFW ─────
FetchContent_Declare(
  glfw
  GIT_REPOSITORY https://github.com/glfw/glfw.git
  GIT_TAG latest
)
FetchContent_MakeAvailable(glfw)

# ───── GLM ─────
FetchContent_Declare(
  glm
  GIT_REPOSITORY https://github.com/g-truc/glm.git
  GIT_TAG 0.9.9.8
)
FetchContent_MakeAvailable(glm)

# ───── ImGui ─────
FetchContent_Declare(
  imgui
  GIT_REPOSITORY https://github.com/ocornut/imgui.git
  GIT_TAG master
)
FetchContent_MakeAvailable(imgui)

# ───── OpenGL ─────
find_package(OpenGL REQUIRED)

set(SRC_DIR      ${CMAKE_SOURCE_DIR}/src)
set(EXTERNAL_DIR ${CMAKE_SOURCE_DIR}/external)

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
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

add_library(imgui STATIC ${IMGUI_SOURCES})

target_include_directories(imgui PUBLIC
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)

target_link_libraries(imgui PRIVATE glfw)

add_executable(SpaceTimeCube ${SRC_FILES})

target_include_directories(SpaceTimeCube PRIVATE
    ${SRC_DIR}
    ${EXTERNAL_DIR}
    ${glm_SOURCE_DIR}
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)

target_link_libraries(SpaceTimeCube PRIVATE
    glfw
    imgui
    OpenGL::GL
    glm::glm
)

target_compile_definitions(SpaceTimeCube PRIVATE GLFW_INCLUDE_NONE)



file(GLOB_RECURSE SHADERS
     "${SRC_DIR}/shaders/*.vert"
     "${SRC_DIR}/shaders/*.frag"
     "${SRC_DIR}/shaders/*.comp"
)
file(GLOB_RECURSE TEXTURES
     "${SRC_DIR}/textures/*"
)

add_custom_target(copy-assets ALL
    DEPENDS ${SHADERS} ${TEXTURES}
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${SRC_DIR}/shaders
            ${CMAKE_CURRENT_BINARY_DIR}/shaders
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${SRC_DIR}/textures
            ${CMAKE_CURRENT_BINARY_DIR}/textures
)

add_dependencies(SpaceTimeCube copy-assets)

