#pragma once

#include <GLFW/glfw3.h>
#include <memory>

class Scene;

class Simulation {
  public:
    Simulation(int width, int height);
    ~Simulation();

    void run();

  private:
    void initGLFW();
    void initGLAD();
    void shutdown();

    void handleInput(float deltaTime);
    void render();

    void framebufferSizeCallback(int width, int height);
    static void framebufferSizeCallbackWrapper(GLFWwindow *, int, int);

    GLFWwindow *window = nullptr;
    int windowWidth, windowHeight;
    double lastTime = 0.0;
    double lastDeltaTime = 0.0;
    std::unique_ptr<Scene> scene;
};
