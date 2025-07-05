#include "CelestialBody.h"
#include "ComputeShader.h"
#include <glad/glad.h> // ← Make sure this is included for GLuint
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
    std::vector<glm::dvec3> velocities;

    ComputeShader *gShader = nullptr;

    GLuint ssboBodies = 0;
    GLuint ssboAccels = 0;

    void computeAccelerations();
    void computeAccelerationsGPU();
};
