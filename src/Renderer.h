#pragma once

#include "GravityWell.h"
#include "ShaderProgram.h"
#include <filesystem>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>

class Renderer {
  public:
    Renderer(int width, int height);
    ~Renderer() noexcept;

    void drawAll(const std::vector<class CelestialBody *> &bodies,
                 const glm::mat4 &view, const glm::mat4 &proj) noexcept;

    void setViewportSize(int width, int height) noexcept;

  private:
    int width_, height_;

    ShaderProgram bodyProg_;
    ShaderProgram trailProg_;
    ShaderProgram wellProg_;

    GravityWell gravityWell_;

    static std::string loadFile(const std::filesystem::path &path);
};
