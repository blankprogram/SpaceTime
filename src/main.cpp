#include "CelestialBody.h"
#include "PhysicsEngine.h"
#include "Renderer.h"

#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <iostream>
#include <memory>

constexpr int WINDOW_WIDTH = 800;
constexpr int WINDOW_HEIGHT = 600;
constexpr float MOUSE_SENSITIVITY = 0.1f;
constexpr float CAMERA_SPEED = 5.0f;
constexpr float CAMERA_SPRINT_MULT = 5.0f;
constexpr float MAX_PITCH = 89.0f;

struct Camera {
    glm::vec3 position{0.0f, 0.0f, 30.0f};
    glm::vec3 front{0.0f, 0.0f, -1.0f};
    glm::vec3 up{0.0f, 1.0f, 0.0f};
    float yaw = -90.0f;
    float pitch = 0.0f;
    float lastX = WINDOW_WIDTH / 2.0f;
    float lastY = WINDOW_HEIGHT / 2.0f;
    bool firstMouse = true;
} camera;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

void framebufferSizeCallback(GLFWwindow *window, int width, int height) {
    auto *renderer =
        reinterpret_cast<Renderer *>(glfwGetWindowUserPointer(window));
    if (renderer)
        renderer->setViewportSize(width, height);
}

void mouseCallback(GLFWwindow *window, double xpos, double ypos) {
    if (camera.firstMouse) {
        camera.lastX = (float)xpos;
        camera.lastY = (float)ypos;
        camera.firstMouse = false;
    }
    float xoffset = (float)xpos - camera.lastX;
    float yoffset = camera.lastY - (float)ypos;
    camera.lastX = (float)xpos;
    camera.lastY = (float)ypos;

    xoffset *= MOUSE_SENSITIVITY;
    yoffset *= MOUSE_SENSITIVITY;

    camera.yaw += xoffset;
    camera.pitch += yoffset;
    camera.pitch = glm::clamp(camera.pitch, -MAX_PITCH, MAX_PITCH);

    glm::vec3 direction;
    direction.x =
        cos(glm::radians(camera.yaw)) * cos(glm::radians(camera.pitch));
    direction.y = sin(glm::radians(camera.pitch));
    direction.z =
        sin(glm::radians(camera.yaw)) * cos(glm::radians(camera.pitch));
    camera.front = glm::normalize(direction);
}

void processInput(GLFWwindow *window) {
    float sprint = (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
                       ? CAMERA_SPRINT_MULT
                       : 1.0f;
    float camSpeed = CAMERA_SPEED * sprint * deltaTime;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.position += camSpeed * camera.front;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.position -= camSpeed * camera.front;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.position -=
            glm::normalize(glm::cross(camera.front, camera.up)) * camSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.position +=
            glm::normalize(glm::cross(camera.front, camera.up)) * camSpeed;
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        camera.position += camSpeed * camera.up;
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
        camera.position -= camSpeed * camera.up;
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

int main() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW.\n";
        return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window =
        glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Three-Body Figure-Eight",
                         nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window.\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetCursorPosCallback(window, mouseCallback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD.\n";
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_PROGRAM_POINT_SIZE);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    PhysicsEngine physics;
    Renderer renderer(WINDOW_WIDTH, WINDOW_HEIGHT);

    glfwSetWindowUserPointer(window, &renderer);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

    constexpr double posScale = 20.0;
    constexpr double G_factor = 0.5;
    constexpr double mass = 100.0;
    double GM_over_L = (G_factor * mass) / posScale;
    double vScale = std::sqrt(GM_over_L);

    glm::dvec2 r1_orig = {-0.602885898116520, 0.059162128863347};
    glm::dvec2 r2_orig = {0.252709795391000, 0.058254872224370};
    glm::dvec2 r3_orig = {-0.355389016941814, 0.038323764315145};
    glm::dvec2 v1_orig = {0.122913546623784, 0.747443868604908};
    glm::dvec2 v2_orig = {-0.019325586404545, 1.369241993562101};
    glm::dvec2 v3_orig = {-0.103587960218793, -2.116685862168820};

    auto body1 = std::make_unique<CelestialBody>(
        mass, glm::dvec3(r1_orig.x, 0.0, r1_orig.y) * posScale,
        glm::dvec3(v1_orig.x, 0.0, v1_orig.y) * vScale, 0.5f,
        "textures/dirt.jpg", glm::vec3(1.0f, 0.0f, 0.0f));

    auto body2 = std::make_unique<CelestialBody>(
        mass, glm::dvec3(r2_orig.x, 0.0, r2_orig.y) * posScale,
        glm::dvec3(v2_orig.x, 0.0, v2_orig.y) * vScale, 0.5f,
        "textures/lava.png", glm::vec3(0.0f, 1.0f, 0.0f));

    auto body3 = std::make_unique<CelestialBody>(
        mass, glm::dvec3(r3_orig.x, 0.0, r3_orig.y) * posScale,
        glm::dvec3(v3_orig.x, 0.0, v3_orig.y) * vScale, 0.5f,
        "textures/stone.jpg", glm::vec3(0.0f, 0.4f, 1.0f));

    physics.addBody(body1.get());
    physics.addBody(body2.get());
    physics.addBody(body3.get());

    lastFrame = glfwGetTime();
    const double maxSubstep = 0.01;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 450");
    while (!glfwWindowShouldClose(window)) {
        double now = glfwGetTime();
        double rawDt = now - lastFrame;
        lastFrame = now;
        deltaTime = (float)rawDt;

        processInput(window);

        double remaining = rawDt;
        while (remaining > 0.0) {
            double subDt = std::min(remaining, maxSubstep);
            physics.step(subDt);
            remaining -= subDt;
        }

        glm::mat4 view = glm::lookAt(camera.position,
                                     camera.position + camera.front, camera.up);
        int w, h;
        glfwGetFramebufferSize(window, &w, &h);
        glm::mat4 proj = glm::perspective(glm::radians(45.0f),
                                          (float)w / (float)h, 1.0f, 1000.0f);

        renderer.drawAll(physics.getBodies(), view, proj);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowSize(ImVec2(200, 200), ImGuiCond_Always);

        ImGui::Begin("Performance");

        ImGui::Text("FPS: %.1f", 1.0f / deltaTime);

        ImGui::Text("GPU Primitives: %d", renderer.getTotalPrimitives());
        ImGui::Text("Mesh (Triangles): %d", renderer.getMeshPrimitives());
        ImGui::Text("Trails (Lines): %d", renderer.getTrailPrimitives());
        ImGui::Text("Well Lines: %d", renderer.getWellPrimitives());
        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
    return 0;
}
