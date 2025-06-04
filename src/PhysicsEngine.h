#pragma once
#include "CelestialBody.h"
#include <glm/gtc/type_ptr.hpp>
#include <vector>

class PhysicsEngine {
  public:
    PhysicsEngine();
    ~PhysicsEngine();

    void addBody(CelestialBody *body);

    void step(double dt);

    const std::vector<CelestialBody *> &getBodies() const;

  private:
    std::vector<CelestialBody *> bodies;

    std::vector<glm::dvec3> accelerations;

    void computeAccelerations();
};
