#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>

class GravityWell {
  public:
    GravityWell(float size, int resolution);
    ~GravityWell();

    void updateFromBodies(const std::vector<class CelestialBody *> &bodies,
                          float G);
    void draw() const;

  private:
    GLuint vao = 0, vbo = 0;
    GLsizei vertexCount = 0;
    float size;
    int resolution;
};
