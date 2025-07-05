#include "PhysicsEngine.h"
#include "ComputeShader.h"
#include <cmath>
// Suzuki–Yoshida 4th-order coefficients
namespace {
const double alpha = 1.0 / (2.0 - std::cbrt(2.0));
const double beta = -std::cbrt(2.0) / (2.0 - std::cbrt(2.0));
const double gamma = alpha;

const double d1 = alpha * 0.5;
const double d2 = (alpha + beta) * 0.5;
const double d3 = (beta + gamma) * 0.5;
const double d4 = gamma * 0.5;

const double k1 = alpha;
const double k2 = beta;
const double k3 = gamma;

void doDrift(std::vector<CelestialBody *> &bodies,
             const std::vector<glm::dvec3> &vels, double h) {
    for (size_t i = 0; i < bodies.size(); ++i) {
        auto r_old = bodies[i]->getPosition();
        bodies[i]->setPosition(r_old + vels[i] * h);
    }
}

void doKick(std::vector<CelestialBody *> &bodies,
            const std::vector<glm::dvec3> &accs, double h) {
    for (size_t i = 0; i < bodies.size(); ++i) {
        auto v_old = bodies[i]->getVelocity();
        bodies[i]->setVelocity(v_old + accs[i] * h);
    }
}
} // namespace

PhysicsEngine::PhysicsEngine() = default;
PhysicsEngine::~PhysicsEngine() = default;

void PhysicsEngine::addBody(CelestialBody *body) { bodies.push_back(body); }

const std::vector<CelestialBody *> &PhysicsEngine::getBodies() const {
    return bodies;
}

void PhysicsEngine::computeAccelerations() {
    if (!gShader)
        gShader = new ComputeShader("shaders/gravity.comp");

    size_t n = bodies.size();
    std::vector<glm::vec4> posMass(n);

    for (size_t i = 0; i < n; ++i) {
        glm::dvec3 p = bodies[i]->getPosition();
        posMass[i] = glm::vec4((float)p.x, (float)p.y, (float)p.z,
                               (float)bodies[i]->getMass());
    }

    if (!ssboBodies)
        glGenBuffers(1, &ssboBodies);
    if (!ssboAccels)
        glGenBuffers(1, &ssboAccels);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssboBodies);
    glBufferData(GL_SHADER_STORAGE_BUFFER, n * sizeof(glm::vec4),
                 posMass.data(), GL_DYNAMIC_DRAW);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssboAccels);
    glBufferData(GL_SHADER_STORAGE_BUFFER, n * sizeof(glm::vec4), nullptr,
                 GL_DYNAMIC_DRAW);

    gShader->bind();
    gShader->dispatch((int)n);

    std::vector<glm::vec4> accels(n);
    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, n * sizeof(glm::vec4),
                       accels.data());

    accelerations.resize(n);
    for (size_t i = 0; i < n; ++i)
        accelerations[i] = glm::dvec3(accels[i]);
}

void PhysicsEngine::step(double dt) {
    if (bodies.empty())
        return;

    size_t n = bodies.size();
    velocities.resize(n);

    for (size_t i = 0; i < n; ++i)
        velocities[i] = bodies[i]->getVelocity();

    doDrift(bodies, velocities, d1 * dt);
    computeAccelerations();
    doKick(bodies, accelerations, k1 * dt);

    for (size_t i = 0; i < n; ++i)
        velocities[i] = bodies[i]->getVelocity();
    doDrift(bodies, velocities, d2 * dt);
    computeAccelerations();
    doKick(bodies, accelerations, k2 * dt);

    for (size_t i = 0; i < n; ++i)
        velocities[i] = bodies[i]->getVelocity();
    doDrift(bodies, velocities, d3 * dt);
    computeAccelerations();
    doKick(bodies, accelerations, k3 * dt);

    for (size_t i = 0; i < n; ++i)
        velocities[i] = bodies[i]->getVelocity();
    doDrift(bodies, velocities, d4 * dt);

    for (auto *b : bodies)
        b->updateTrail(static_cast<float>(dt));
}
