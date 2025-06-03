#include <glm/fwd.hpp>
#include <glm/trigonometric.hpp>
#include <vector>
#define STB_IMAGE_IMPLEMENTATION
#include <GLFW/glfw3.h>
#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <stb_image.h>

#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

float yaw = -90.0f;
float pitch = 0.0f;
float lastX = 400.0f;
float lastY = 300.0f;
bool firstMouse = true;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

void processInput(GLFWwindow *window) {
  float baseSpeed = 2.5f;
  float speedMultiplier = 1.0f;

  if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
    speedMultiplier = 3.0f;
  float cameraSpeed = baseSpeed * speedMultiplier * deltaTime;

  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    cameraPos += cameraSpeed * cameraFront;
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    cameraPos -= cameraSpeed * cameraFront;
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    cameraPos -=
        glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    cameraPos +=
        glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
  if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
    cameraPos += cameraSpeed * cameraUp;
  if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
    cameraPos -= cameraSpeed * cameraUp;
}

void mouse_callback(GLFWwindow *window, double xpos, double ypos) {
  if (firstMouse) {
    lastX = xpos;
    lastY = ypos;
    firstMouse = false;
  }

  float xoffset = xpos - lastX;
  float yoffset = lastY - ypos;
  lastX = xpos;
  lastY = ypos;

  float sensitivity = 0.1f;
  xoffset *= sensitivity;
  yoffset *= sensitivity;

  yaw += xoffset;
  pitch += yoffset;

  pitch = glm::clamp(pitch, -89.0f, 89.0f);

  glm::vec3 direction;
  direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
  direction.y = sin(glm::radians(pitch));
  direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
  cameraFront = glm::normalize(direction);
}

// -------------------------
// Utility: Load shader file
// -------------------------
std::string load_shader(const char *path) {
  std::ifstream file(path);
  if (!file.is_open())
    throw std::runtime_error("Failed to open shader file.");
  std::stringstream ss;
  ss << file.rdbuf();
  return ss.str();
}

// -------------------------
// Utility: Compile shader
// -------------------------
GLuint compile_shader(GLenum type, const std::string &source) {
  GLuint shader = glCreateShader(type);
  const char *src = source.c_str();
  glShaderSource(shader, 1, &src, nullptr);
  glCompileShader(shader);

  int success;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    char log[512];
    glGetShaderInfoLog(shader, 512, nullptr, log);
    std::cerr << "Shader Compile Error: " << log << "\n";
  }

  return shader;
}

// -------------------------
// Utility: Link shader program
// -------------------------
GLuint create_shader_program(const char *vs_path, const char *fs_path) {
  auto vs_src = load_shader(vs_path);
  auto fs_src = load_shader(fs_path);
  GLuint vs = compile_shader(GL_VERTEX_SHADER, vs_src);
  GLuint fs = compile_shader(GL_FRAGMENT_SHADER, fs_src);

  GLuint program = glCreateProgram();
  glAttachShader(program, vs);
  glAttachShader(program, fs);
  glLinkProgram(program);

  glDeleteShader(vs);
  glDeleteShader(fs);

  return program;
}

// -------------------------
// Vertex Data (Sphere)
// -------------------------
void generateSphereMesh(std::vector<float> &vertices,
                        std::vector<unsigned int> &indices, float radius = 1.0f,
                        int sectorCount = 36, int stackCount = 18) {
  for (int i = 0; i <= stackCount; ++i) {
    float stackAngle = glm::pi<float>() / 2 - i * glm::pi<float>() / stackCount;
    float xy = radius * cos(stackAngle);
    float z = radius * sin(stackAngle);

    for (int j = 0; j <= sectorCount; ++j) {
      float sectorAngle = j * 2 * glm::pi<float>() / sectorCount;

      float x = xy * cos(sectorAngle);
      float y = xy * sin(sectorAngle);
      float u = (float)j / sectorCount;
      float v = (float)i / stackCount;

      vertices.insert(vertices.end(), {x, y, z, u, v});
    }
  }

  for (int i = 0; i < stackCount; ++i) {
    int k1 = i * (sectorCount + 1);
    int k2 = k1 + sectorCount + 1;

    for (int j = 0; j < sectorCount; ++j, ++k1, ++k2) {
      if (i != 0) {
        indices.insert(indices.end(), {static_cast<unsigned int>(k1),
                                       static_cast<unsigned int>(k2),
                                       static_cast<unsigned int>(k1 + 1)});
      }
      if (i != (stackCount - 1)) {
        indices.insert(indices.end(), {static_cast<unsigned int>(k1 + 1),
                                       static_cast<unsigned int>(k2),
                                       static_cast<unsigned int>(k2 + 1)});
      }
    }
  }
}
// -------------------------
// Main
// -------------------------
int main() {
  if (!glfwInit())
    return -1;

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow *window =
      glfwCreateWindow(800, 600, "OpenGL Cube", nullptr, nullptr);
  if (!window)
    return -1;

  glfwSetCursorPosCallback(window, mouse_callback);
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  glfwMakeContextCurrent(window);
  gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

  int width, height;
  glfwGetFramebufferSize(window, &width, &height);
  glViewport(0, 0, width, height);

  GLuint shaderProgram =
      create_shader_program("shaders/vertex.glsl", "shaders/fragment.glsl");

  std::vector<float> sphereVertices;
  std::vector<unsigned int> sphereIndices;
  generateSphereMesh(sphereVertices, sphereIndices);

  GLuint VAO, VBO, EBO;
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  glGenBuffers(1, &EBO);

  glBindVertexArray(VAO);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sphereVertices.size() * sizeof(float),
               sphereVertices.data(), GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
               sphereIndices.size() * sizeof(unsigned int),
               sphereIndices.data(), GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                        (void *)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  GLuint texture;
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  int texWidth, texHeight, texChannels;
  stbi_set_flip_vertically_on_load(true);
  unsigned char *data =
      stbi_load("textures/earth.jpeg", &texWidth, &texHeight, &texChannels, 0);
  if (data) {
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texWidth, texHeight, 0, GL_RGB,
                 GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
  } else {
    std::cerr << "Failed to load texture\n";
  }
  stbi_image_free(data);

  glEnable(GL_DEPTH_TEST);
  GLint mvpLoc = glGetUniformLocation(shaderProgram, "u_MVP");
  GLint texLoc = glGetUniformLocation(shaderProgram, "u_Texture");

  while (!glfwWindowShouldClose(window)) {
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;
    processInput(window);

    float time = glfwGetTime();
    glm::mat4 model =
        glm::rotate(glm::mat4(1.0f), time, glm::vec3(0.0f, 1.0f, 0.0f));

    model =
        glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

    glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

    glm::mat4 proj = glm::perspective(glm::radians(45.0f),
                                      (float)width / height, 0.1f, 100.0f);
    glm::mat4 mvp = proj * view * model;

    glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(shaderProgram);
    glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, glm::value_ptr(mvp));
    glUniform1i(texLoc, 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glBindVertexArray(VAO);

    glDrawElements(GL_TRIANGLES, sphereIndices.size(), GL_UNSIGNED_INT, 0);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(1, &VBO);
  glDeleteTextures(1, &texture);
  glDeleteProgram(shaderProgram);

  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}
