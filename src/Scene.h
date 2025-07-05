#pragma once

#include "Camera.h"
#include "PhysicsEngine.h"
#include "Renderer.h"

#include <GLFW/glfw3.h>

class Scene {
  public:
    Scene(int width, int height);
    ~Scene();

    void initialize(GLFWwindow *window);
    void update(float deltaTime);
    void render(float dt);

    Camera &getCamera();
    GLFWwindow *getWindow() const;

  private:
    GLFWwindow *window = nullptr;
    int width, height;

    Camera camera;
    PhysicsEngine physics;
    Renderer renderer;

    void addInitialBodies();
    void addRandomBodies(int n = 100, double mass = 100.0, double space = 50.0);
};
