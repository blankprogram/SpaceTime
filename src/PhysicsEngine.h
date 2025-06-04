#pragma once

#include "CelestialBody.h"
#include <vector>

class PhysicsEngine {
public:
  PhysicsEngine();
  ~PhysicsEngine();

  void addBody(CelestialBody *body);

  void step(float dt);

  const std::vector<CelestialBody *> &getBodies() const;

private:
  std::vector<CelestialBody *> bodies;
  std::vector<glm::vec3> accelerations;

  void computeAccelerations();
};
