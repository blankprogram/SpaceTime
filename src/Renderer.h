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

  private:
    int width_, height_;

    Program bodyProg_;
    Program trailProg_;
    Program wellProg_;
    GravityWell gravityWell_;

    static std::string loadFile(const std::filesystem::path &path);
};
