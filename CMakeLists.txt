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

include_directories(
    external                  # For external headers
    external/glad             # For glad.h
    external/KHR              # For khrplatform.h
    ${glm_SOURCE_DIR}         # For glm::mat4 etc
)

add_executable(SpaceTimeCube
    main.cpp
    external/glad/glad.c
)

target_link_libraries(SpaceTimeCube PRIVATE glfw)

add_custom_command(TARGET SpaceTimeCube POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/shaders $<TARGET_FILE_DIR:SpaceTimeCube>/shaders
)


set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
