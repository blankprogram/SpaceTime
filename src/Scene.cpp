#include "Scene.h"
#include "CelestialBody.h"
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <random>

Scene::Scene(int width, int height)
    : width(width), height(height), renderer(width, height) {}

Scene::~Scene() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    if (window)
        glfwDestroyWindow(window);
    glfwTerminate();
}

void Scene::initialize(GLFWwindow *win) {
    window = win;
    glfwSetWindowUserPointer(window, &renderer);

    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 450");

    addInitialBodies();
}

void Scene::addInitialBodies() {
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

    physics.addBody(std::move(body1));
    physics.addBody(std::move(body2));
    physics.addBody(std::move(body3));
}

void Scene::addRandomBodies(int n, double mass, double space) {
    std::default_random_engine rng{std::random_device{}()};
    std::uniform_real_distribution<double> posDist(-space, space);
    std::uniform_real_distribution<double> velDist(-1.0, 1.0);
    std::uniform_real_distribution<float> colorDist(0.2f, 1.0f);

    for (int i = 0; i < n; ++i) {
        glm::dvec3 pos(posDist(rng), posDist(rng), posDist(rng));
        glm::dvec3 vel(velDist(rng), velDist(rng), velDist(rng));
        glm::vec3 color(colorDist(rng), colorDist(rng), colorDist(rng));

        std::string tex = (i % 3 == 0)   ? "textures/dirt.jpg"
                          : (i % 3 == 1) ? "textures/lava.png"
                                         : "textures/stone.jpg";

        auto body = std::make_unique<CelestialBody>(mass, pos, vel, 0.5f,
                                                    tex.c_str(), color);
        physics.addBody(std::move(body));
    }
}

void Scene::update(float deltaTime) {
    constexpr double maxSubstep = 0.01;
    double remaining = deltaTime;
    while (remaining > 0.0) {
        double step = std::min(remaining, maxSubstep);
        physics.step(step);
        remaining -= step;
    }
}

void Scene::render(float dt) {
    int w, h;
    glfwGetFramebufferSize(window, &w, &h);

    renderer.setViewportSize(w, h);

    float aspect = static_cast<float>(w) / h;
    glm::mat4 proj =
        glm::perspective(glm::radians(45.0f), aspect, 0.1f, 5000.0f);
    glm::mat4 view = camera.getViewMatrix();

    renderer.drawAll(physics.getBodies(), view, proj);

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::SetNextWindowSize(ImVec2(200, 200), ImGuiCond_Always);

    ImGui::Begin("Performance");
    ImGui::Text("FPS: %.1f", 1.0f / dt);
    ImGui::Text("Frame time: %.2f ms", dt * 1000.0f);
    ImGui::Text("Primitives: %d", renderer.getTotalPrimitives());
    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

Camera &Scene::getCamera() { return camera; }
GLFWwindow *Scene::getWindow() const { return window; }
