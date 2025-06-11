#pragma once

#include "raii.h"
#include <glm/glm.hpp>
#include <vector>

class CelestialBody;

class GravityWell {
  public:
    GravityWell(float size, int resolution);
    ~GravityWell() noexcept;

    void updateFromBodies(const std::vector<CelestialBody *> &bodies,
                          float G) noexcept;
    void draw() const noexcept;

  private:
    VertexArray vao_;
    Buffer vbo_;
    size_t vertexCount_;

    float size_;
    int resolution_;

    std::vector<float> cpuBuffer_;
    std::vector<float> yGrid_;

    void setupBuffers() noexcept;
};
