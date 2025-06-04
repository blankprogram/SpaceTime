cmake_minimum_required(VERSION 3.16)
project(SpaceTimeCube)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
  glfw
  GIT_REPOSITORY https://github.com/glfw/glfw.git
  GIT_TAG        latest
)
FetchContent_MakeAvailable(glfw)

FetchContent_Declare(
  glm
  GIT_REPOSITORY https://github.com/g-truc/glm.git
  GIT_TAG        0.9.9.8
)
FetchContent_MakeAvailable(glm)

find_package(OpenGL REQUIRED)

include_directories(
  ${CMAKE_SOURCE_DIR}/external/glad
  ${CMAKE_SOURCE_DIR}/external/KHR
  ${CMAKE_SOURCE_DIR}/external
)

set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)

set(APP_SOURCES
    ${SRC_DIR}/main.cpp
    ${SRC_DIR}/CelestialBody.cpp
    ${SRC_DIR}/PhysicsEngine.cpp
    ${SRC_DIR}/Renderer.cpp

    ${CMAKE_SOURCE_DIR}/external/glad/glad.c
)

add_executable(SpaceTimeCube
    ${APP_SOURCES}
)

target_include_directories(SpaceTimeCube PRIVATE
    ${SRC_DIR}                        ${CMAKE_SOURCE_DIR}/external/glad
    ${CMAKE_SOURCE_DIR}/external/KHR
    ${glm_SOURCE_DIR}             )

target_link_libraries(SpaceTimeCube
    PRIVATE
      glfw
      OpenGL::GL
      glm::glm        
)

target_compile_definitions(SpaceTimeCube PRIVATE GLFW_INCLUDE_NONE)

add_custom_command(TARGET SpaceTimeCube POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/src/shaders
            $<TARGET_FILE_DIR:SpaceTimeCube>/shaders
)

add_custom_command(TARGET SpaceTimeCube POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/src/textures
            $<TARGET_FILE_DIR:SpaceTimeCube>/textures
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
