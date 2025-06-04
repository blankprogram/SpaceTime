#include "PhysicsEngine.h"

PhysicsEngine::PhysicsEngine() {}

PhysicsEngine::~PhysicsEngine() {}

void PhysicsEngine::addBody(CelestialBody *body) { bodies.push_back(body); }

const std::vector<CelestialBody *> &PhysicsEngine::getBodies() const {
  return bodies;
}

void PhysicsEngine::computeAccelerations() {
  size_t n = bodies.size();
  accelerations.resize(n);

  for (size_t i = 0; i < n; ++i) {
    accelerations[i] = bodies[i]->computeAcceleration(bodies);
  }
}

void PhysicsEngine::step(float dt) {
  size_t n = bodies.size();
  if (n == 0)
    return;

  computeAccelerations();

  std::vector<glm::vec3> newPositions(n);
  std::vector<glm::vec3> oldVelocities(n);
  for (size_t i = 0; i < n; ++i) {
    CelestialBody *b = bodies[i];
    oldVelocities[i] = b->getVelocity();
    glm::vec3 a = accelerations[i];
    glm::vec3 r = b->getPosition();
    glm::vec3 v = oldVelocities[i];
    newPositions[i] = r + v * dt + 0.5f * a * dt * dt;
  }

  for (size_t i = 0; i < n; ++i) {
    bodies[i]->setPosition(newPositions[i]);
  }

  computeAccelerations();
  std::vector<glm::vec3> newAccels = accelerations;

  for (size_t i = 0; i < n; ++i) {
    CelestialBody *b = bodies[i];
    glm::vec3 vOld = oldVelocities[i];
    glm::vec3 aOld = accelerations[i];
    glm::vec3 aNew = newAccels[i];
    glm::vec3 vNew = vOld + 0.5f * (accelerations[i] + aNew) * dt;
    b->setVelocity(vNew);
  }

  for (auto *b : bodies) {
    b->updateTrail(dt);
  }
}
