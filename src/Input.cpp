#include "Input.h"
#include "Camera.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

namespace Input {

static float lastX = 400.0f;
static float lastY = 300.0f;
static bool firstMouse = true;

void mouseCallback(GLFWwindow *window, double xpos, double ypos, Camera &cam) {
    if (firstMouse) {
        lastX = (float)xpos;
        lastY = (float)ypos;
        firstMouse = false;
    }

    float xoffset = (float)xpos - lastX;
    float yoffset = lastY - (float)ypos;
    lastX = (float)xpos;
    lastY = (float)ypos;

    xoffset *= 0.1f;
    yoffset *= 0.1f;

    cam.setYaw(cam.getYaw() + xoffset);
    cam.setPitch(cam.getPitch() + yoffset);
}

void processInput(GLFWwindow *window, Camera &cam, float deltaTime) {
    float speed = 5.0f * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        speed *= 5.0f;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cam.moveForward(speed);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cam.moveForward(-speed);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cam.moveRight(-speed);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cam.moveRight(speed);
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        cam.moveUp(speed);
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
        cam.moveUp(-speed);

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}
} // namespace Input
