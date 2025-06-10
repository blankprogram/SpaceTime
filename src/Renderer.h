#pragma once
#include "CelestialBody.h"
#include "GravityWell.h"
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

    void setViewportSize(int width, int height);

  private:
    int width, height;

    GLuint bodyShader;
    GLuint trailShader;

    GLint bodyMvpLoc;
    GLint bodyTexLoc;
    GLint trailMvpLoc;

    GLuint wellShader;
    GLint wellMvpLoc;

    GravityWell gravityWell = GravityWell(40.0f, 100);

    std::string loadFileToString(const char *path);
    GLuint compileSingleShader(GLenum type, const std::string &src);
    GLuint linkProgram(GLuint vs, GLuint fs);

    void initShaders();
    void cleanupShaders();
};
