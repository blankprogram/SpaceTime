#pragma once

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

class Camera {
  public:
    Camera();
    void updateFromInput(GLFWwindow *window, float deltaTime);
    glm::mat4 getViewMatrix() const;
    void setPosition(const glm::vec3 &pos);

    float getYaw() const { return yaw; }
    float getPitch() const { return pitch; }
    glm::vec3 getPosition() const { return position; }
    glm::vec3 getFront() const { return front; }
    glm::vec3 getUp() const { return up; }

    void setYaw(float y);
    void setPitch(float p);
    void moveForward(float amount);
    void moveRight(float amount);
    void moveUp(float amount);
    void updateMouse(double xpos, double ypos);

  private:
    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 up;

    float yaw;
    float pitch;
    float lastX, lastY;
    bool firstMouse;

    void updateVectors();
};
