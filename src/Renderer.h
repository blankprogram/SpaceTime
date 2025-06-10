#pragma once

#include "GravityWell.h"
#include "raii.h"
#include <filesystem>
#include <glm/glm.hpp>
#include <string>
#include <vector>

class CelestialBody;
class Renderer {
  public:
    Renderer(int width, int height);
    ~Renderer() noexcept;

    void drawAll(const std::vector<CelestialBody *> &bodies,
                 const glm::mat4 &view, const glm::mat4 &proj) noexcept;

    void setViewportSize(int width, int height) noexcept;

    int getTotalPrimitives() const {
        return meshPrimitives_ + trailPrimitives_ + wellPrimitives_;
    }

    int getMeshPrimitives() const { return meshPrimitives_; }
    int getTrailPrimitives() const { return trailPrimitives_; }
    int getWellPrimitives() const { return wellPrimitives_; }

  private:
    int width_, height_;

    Program bodyProg_;
    Program trailProg_;
    Program wellProg_;
    GravityWell gravityWell_;

    GLuint queryMeshID_ = 0;
    GLuint queryTrailID_ = 0;
    GLuint queryWellID_ = 0;

    GLuint meshPrimitives_ = 0;
    GLuint trailPrimitives_ = 0;
    GLuint wellPrimitives_ = 0;

    static std::string loadFile(const std::filesystem::path &path);
};
