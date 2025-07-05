#include "Simulation.h"
#include "Scene.h"

#include <glad/glad.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <stdexcept>

Simulation::Simulation(int width, int height)
    : windowWidth(width), windowHeight(height) {
    initGLFW();
    initGLAD();

    scene = std::make_unique<Scene>(windowWidth, windowHeight);
    scene->initialize(window);

    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallbackWrapper);

    lastTime = glfwGetTime();
}

Simulation::~Simulation() { shutdown(); }

void Simulation::initGLFW() {
    if (!glfwInit())
        throw std::runtime_error("GLFW init failed");

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(windowWidth, windowHeight, "SpaceTime", nullptr,
                              nullptr);
    if (!window)
        throw std::runtime_error("GLFW window creation failed");

    glfwMakeContextCurrent(window);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void Simulation::initGLAD() {
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        throw std::runtime_error("Failed to initialize GLAD");

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_PROGRAM_POINT_SIZE);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
}

void Simulation::shutdown() {
    if (window)
        glfwDestroyWindow(window);
    glfwTerminate();
}

void Simulation::run() {
    while (!glfwWindowShouldClose(window)) {
        double now = glfwGetTime();
        lastDeltaTime = now - lastTime;
        lastTime = now;

        handleInput(static_cast<float>(lastDeltaTime));
        scene->update(static_cast<float>(lastDeltaTime));
        render();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}

void Simulation::handleInput(float deltaTime) {
    scene->getCamera().updateFromInput(window, deltaTime);

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

void Simulation::render() { scene->render(static_cast<float>(lastDeltaTime)); }

void Simulation::framebufferSizeCallback(int width, int height) {
    windowWidth = width;
    windowHeight = height;
}

void Simulation::framebufferSizeCallbackWrapper(GLFWwindow *win, int w, int h) {
    static_cast<Simulation *>(glfwGetWindowUserPointer(win))
        ->framebufferSizeCallback(w, h);
}
