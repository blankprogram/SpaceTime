
#pragma once
#include "CelestialBody.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>
class Renderer {
public:
  Renderer(int width, int height);
  ~Renderer();

  void drawAll(const std::vector<CelestialBody *> &bodies,
               const glm::mat4 &view, const glm::mat4 &proj);

  // Call when window is resized
  void setViewportSize(int width, int height);

private:
  int width, height;

  // Shader program IDs
  GLuint bodyShader;
  GLuint trailShader;

  // Uniform locations
  GLint bodyMvpLoc;
  GLint bodyTexLoc;
  GLint trailMvpLoc;

  // Helpers to load/compile/link shaders
  std::string loadFileToString(const char *path);
  GLuint compileSingleShader(GLenum type, const std::string &src);
  GLuint linkProgram(GLuint vs, GLuint fs);

  void initShaders();
  void cleanupShaders();
};
