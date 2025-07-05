#pragma once

#include "Camera.h"
#include <GLFW/glfw3.h>

namespace Input {
void mouseCallback(GLFWwindow *window, double xpos, double ypos,
                   Camera &camera);
void processInput(GLFWwindow *window, Camera &camera, float deltaTime);
} // namespace Input
