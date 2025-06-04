#include "CelestialBody.h"
#include "PhysicsEngine.h"
#include "Renderer.h"

#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

void framebufferSizeCallback(GLFWwindow *window, int width, int height) {
    Renderer *renderer =
        reinterpret_cast<Renderer *>(glfwGetWindowUserPointer(window));
    if (renderer) {
        renderer->setViewportSize(width, height);
    }
}

static glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 30.0f);
static glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
static glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

static float yaw = -90.0f;
static float pitch = 0.0f;
static float lastX = 800.0f / 2.0f;
static float lastY = 600.0f / 2.0f;
static bool firstMouse = true;

static float deltaTime = 0.0f;
static float lastFrame = 0.0f;

void mouseCallback(GLFWwindow *window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = (float)xpos;
        lastY = (float)ypos;
        firstMouse = false;
    }
    float xoffset = (float)xpos - lastX;
    float yoffset = lastY - (float)ypos;
    lastX = (float)xpos;
    lastY = (float)ypos;

    const float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;
    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(direction);
}

void processInput(GLFWwindow *window) {
    float baseSpeed = 5.0f;
    float sprint =
        (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) ? 3.0f : 1.0f;
    float camSpeed = baseSpeed * sprint * deltaTime;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += camSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= camSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -=
            glm::normalize(glm::cross(cameraFront, cameraUp)) * camSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos +=
            glm::normalize(glm::cross(cameraFront, cameraUp)) * camSpeed;
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        cameraPos += camSpeed * cameraUp;
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
        cameraPos -= camSpeed * cameraUp;
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
        glfwCreateWindow(800, 600, "Three-Body Figure-Eight", nullptr, nullptr);
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

    PhysicsEngine physics;
    Renderer renderer(800, 600);

    glfwSetWindowUserPointer(window, &renderer);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

    constexpr double posScale = 20.0;
    constexpr double G_factor = 0.5;
    constexpr double mass = 100.0;
    double GM_over_L = (G_factor * mass) / posScale;
    double vScale = std::sqrt(GM_over_L);

    // "Flower in Circle" initial positions
    glm::dvec2 r1_orig = {-0.602885898116520, 0.059162128863347};
    glm::dvec2 r2_orig = {0.252709795391000, 0.058254872224370};
    glm::dvec2 r3_orig = {-0.355389016941814, 0.038323764315145};

    // Initial velocities
    glm::dvec2 v1_orig = {0.122913546623784, 0.747443868604908};
    glm::dvec2 v2_orig = {-0.019325586404545, 1.369241993562101};
    glm::dvec2 v3_orig = {-0.103587960218793, -2.116685862168820};

    glm::dvec3 pos1 = glm::dvec3(r1_orig.x, r1_orig.y, 0.0) * posScale;
    glm::dvec3 pos2 = glm::dvec3(r2_orig.x, r2_orig.y, 0.0) * posScale;
    glm::dvec3 pos3 = glm::dvec3(r3_orig.x, r3_orig.y, 0.0) * posScale;

    glm::dvec3 vel1 = glm::dvec3(v1_orig.x, v1_orig.y, 0.0) * vScale;
    glm::dvec3 vel2 = glm::dvec3(v2_orig.x, v2_orig.y, 0.0) * vScale;
    glm::dvec3 vel3 = glm::dvec3(v3_orig.x, v3_orig.y, 0.0) * vScale;

    CelestialBody *body1 =
        new CelestialBody(mass, pos1, vel1, 0.5f, "textures/dirt.jpg",
                          glm::vec3(1.0f, 0.0f, 0.0f)); // red

    CelestialBody *body2 =
        new CelestialBody(mass, pos2, vel2, 0.5f, "textures/lava.png",
                          glm::vec3(0.0f, 1.0f, 0.0f)); // green

    CelestialBody *body3 =
        new CelestialBody(mass, pos3, vel3, 0.5f, "textures/stone.jpg",
                          glm::vec3(0.0f, 0.4f, 1.0f)); // blue

    physics.addBody(body1);
    physics.addBody(body2);
    physics.addBody(body3);

    lastFrame = glfwGetTime();
    const double maxSubstep = 0.001;
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

        glm::mat4 view =
            glm::lookAt(glm::dvec3(cameraPos),
                        glm::dvec3(cameraPos) + glm::dvec3(cameraFront),
                        glm::dvec3(cameraUp));
        int w, h;
        glfwGetFramebufferSize(window, &w, &h);
        glm::mat4 proj = glm::perspective(glm::radians(45.0f),
                                          (float)w / (float)h, 1.0f, 1000.0f);

        renderer.drawAll(physics.getBodies(), view, proj);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    delete body1;
    delete body2;
    delete body3;
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
