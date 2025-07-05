#pragma once

#include "CelestialBody.h"
#include "ComputeShader.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <memory>
#include <vector>

class PhysicsEngine {
  public:
    PhysicsEngine();
    ~PhysicsEngine() = default;

    void addBody(std::unique_ptr<CelestialBody> body);
    void step(double dt);

    std::vector<CelestialBody *> getBodies() const;

  private:
    std::vector<std::unique_ptr<CelestialBody>> bodies;
    std::vector<glm::dvec3> accelerations;
    std::vector<glm::dvec3> velocities;

    std::unique_ptr<ComputeShader> gShader =
        std::make_unique<ComputeShader>("shaders/gravity.comp");

    GLuint ssboBodies = 0;
    GLuint ssboAccels = 0;

    void computeAccelerations();
};
