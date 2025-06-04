cmake_minimum_required(VERSION 3.16)
project(SpaceTimeCube)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)

FetchContent_Declare(
  glfw
  GIT_REPOSITORY https://github.com/glfw/glfw.git
  GIT_TAG latest
)
FetchContent_MakeAvailable(glfw)

FetchContent_Declare(
  glm
  GIT_REPOSITORY https://github.com/g-truc/glm.git
  GIT_TAG 0.9.9.8
)
FetchContent_MakeAvailable(glm)

find_package(OpenGL REQUIRED)

set(SRC_DIR      ${CMAKE_SOURCE_DIR}/src)
set(EXTERNAL_DIR ${CMAKE_SOURCE_DIR}/external)

set(SRC_FILES
    ${SRC_DIR}/main.cpp
    ${SRC_DIR}/CelestialBody.cpp
    ${SRC_DIR}/PhysicsEngine.cpp
    ${SRC_DIR}/Renderer.cpp
    ${EXTERNAL_DIR}/glad/glad.c
)

add_executable(SpaceTimeCube ${SRC_FILES})

target_include_directories(SpaceTimeCube PRIVATE
    ${SRC_DIR}
    ${EXTERNAL_DIR}
    ${glm_SOURCE_DIR}
)

target_link_libraries(SpaceTimeCube PRIVATE
    glfw
    OpenGL::GL
    glm::glm
)

target_compile_definitions(SpaceTimeCube PRIVATE GLFW_INCLUDE_NONE)

add_custom_command(TARGET SpaceTimeCube POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${SRC_DIR}/shaders
            $<TARGET_FILE_DIR:SpaceTimeCube>/shaders
)

add_custom_command(TARGET SpaceTimeCube POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${SRC_DIR}/textures
            $<TARGET_FILE_DIR:SpaceTimeCube>/textures
)
