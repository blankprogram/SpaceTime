
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
    cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * camSpeed;
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * camSpeed;
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
      glfwCreateWindow(800, 600, "SpaceTimeCube", nullptr, nullptr);
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

  constexpr float SUN_MASS = 10000.0f;
  constexpr float EARTH_MASS = 100.0f;
  constexpr float MOON_MASS = 1.0f;
  constexpr float EARTH_DIST = 20.0f;
  constexpr float EM_SEP = 1.5f;
  const float totalEM = EARTH_MASS + MOON_MASS;
  const float muVal = MOON_MASS / totalEM;

  glm::vec3 baryPos(EARTH_DIST, 0.0f, 0.0f);
  glm::vec3 earthPos = baryPos - glm::vec3(muVal * EM_SEP, 0.0f, 0.0f);
  glm::vec3 moonPos = baryPos + glm::vec3((1.0f - muVal) * EM_SEP, 0.0f, 0.0f);

  float vBary = sqrt(0.5f * (SUN_MASS + totalEM) / EARTH_DIST);
  float vRel = sqrt(0.5f * totalEM / EM_SEP);

  glm::vec3 earthVel = glm::vec3(0, 0, vBary) - muVal * glm::vec3(0, 0, vRel);
  glm::vec3 moonVel =
      glm::vec3(0, 0, vBary) + (1.0f - muVal) * glm::vec3(0, 0, vRel);

  CelestialBody *sun = new CelestialBody(
      SUN_MASS, glm::vec3(0.0f), glm::vec3(0.0f), 1.5f, "textures/lava.png");
  CelestialBody *earth = new CelestialBody(EARTH_MASS, earthPos, earthVel, 0.5f,
                                           "textures/dirt.jpg");
  CelestialBody *moon = new CelestialBody(MOON_MASS, moonPos, moonVel, 0.2f,
                                          "textures/stone.jpg");

  physics.addBody(sun);
  physics.addBody(earth);
  physics.addBody(moon);

  // 6) Main loop
  while (!glfwWindowShouldClose(window)) {
    float currentFrame = static_cast<float>(glfwGetTime());
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    processInput(window);
    physics.step(deltaTime);

    glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
    int w, h;
    glfwGetFramebufferSize(window, &w, &h);
    glm::mat4 proj = glm::perspective(glm::radians(45.0f), (float)w / (float)h,
                                      0.1f, 100.0f);

    renderer.drawAll(physics.getBodies(), view, proj);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  delete sun;
  delete earth;
  delete moon;

  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}
