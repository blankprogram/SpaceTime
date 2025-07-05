#include "Camera.h"
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>

Camera::Camera()
    : position(0.0f, 0.0f, 30.0f), front(0.0f, 0.0f, -1.0f),
      up(0.0f, 1.0f, 0.0f), yaw(-90.0f), pitch(0.0f), lastX(400), lastY(300),
      firstMouse(true) {}

void Camera::setPosition(const glm::vec3 &pos) { position = pos; }

void Camera::updateFromInput(GLFWwindow *window, float deltaTime) {
    constexpr float sensitivity = 0.1f;
    constexpr float speed = 5.0f;
    constexpr float sprintMult = 5.0f;
    constexpr float maxPitch = 89.0f;

    double xPos, yPos;
    glfwGetCursorPos(window, &xPos, &yPos);
    if (firstMouse) {
        lastX = (float)xPos;
        lastY = (float)yPos;
        firstMouse = false;
    }
    float xOffset = (float)xPos - lastX;
    float yOffset = lastY - (float)yPos;
    lastX = (float)xPos;
    lastY = (float)yPos;

    xOffset *= sensitivity;
    yOffset *= sensitivity;

    yaw += xOffset;
    pitch += yOffset;
    pitch = glm::clamp(pitch, -maxPitch, maxPitch);

    updateVectors();

    float finalSpeed =
        speed * deltaTime *
        (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ? sprintMult
                                                               : 1.0f);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        position += front * finalSpeed;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        position -= front * finalSpeed;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        position -= glm::normalize(glm::cross(front, up)) * finalSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        position += glm::normalize(glm::cross(front, up)) * finalSpeed;
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        position += up * finalSpeed;
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
        position -= up * finalSpeed;
}

void Camera::updateVectors() {
    glm::vec3 dir;
    dir.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    dir.y = sin(glm::radians(pitch));
    dir.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    front = glm::normalize(dir);
}

glm::mat4 Camera::getViewMatrix() const {
    return glm::lookAt(position, position + front, up);
}

void Camera::setYaw(float y) {
    yaw = y;
    updateVectors();
}

void Camera::setPitch(float p) {
    pitch = glm::clamp(p, -89.0f, 89.0f);
    updateVectors();
}

void Camera::moveForward(float amount) { position += front * amount; }

void Camera::moveRight(float amount) {
    position += glm::normalize(glm::cross(front, up)) * amount;
}

void Camera::moveUp(float amount) { position += up * amount; }
