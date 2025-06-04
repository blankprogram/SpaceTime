#include "PhysicsEngine.h"
#include <cmath>

// Suzuki–Yoshida 4th-order symplectic integrator coefficients
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

std::vector<glm::dvec3>
getVelocities(const std::vector<CelestialBody *> &bodies) {
    std::vector<glm::dvec3> velocities(bodies.size());
    for (size_t i = 0; i < bodies.size(); ++i)
        velocities[i] = bodies[i]->getVelocity();
    return velocities;
}

void doDrift(std::vector<CelestialBody *> &bodies,
             const std::vector<glm::dvec3> &vels, double h) {
    for (size_t i = 0; i < bodies.size(); ++i) {
        glm::dvec3 r_old = bodies[i]->getPosition();
        bodies[i]->setPosition(r_old + vels[i] * h);
    }
}

void doKick(std::vector<CelestialBody *> &bodies,
            const std::vector<glm::dvec3> &accs, double h) {
    for (size_t i = 0; i < bodies.size(); ++i) {
        glm::dvec3 v_old = bodies[i]->getVelocity();
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
    size_t n = bodies.size();
    accelerations.resize(n);

    for (size_t i = 0; i < n; ++i) {
        glm::dvec3 a_i(0.0);
        glm::dvec3 r_i = bodies[i]->getPosition();

        for (size_t j = 0; j < n; ++j) {
            if (j == i)
                continue;

            glm::dvec3 r_j = bodies[j]->getPosition();
            double m_j = bodies[j]->getMass();

            glm::dvec3 diff = r_j - r_i;
            double dist2 = glm::dot(diff, diff) + 1e-15;
            double invDist3 = 1.0 / (dist2 * std::sqrt(dist2));

            a_i += (0.5 * m_j) * (diff * invDist3);
        }
        accelerations[i] = a_i;
    }
}

void PhysicsEngine::step(double dt) {
    if (bodies.empty())
        return;

    const size_t n = bodies.size();

    auto vel0 = getVelocities(bodies);
    doDrift(bodies, vel0, d1 * dt);

    computeAccelerations();
    auto a0 = accelerations;
    doKick(bodies, a0, k1 * dt);

    auto vel1 = getVelocities(bodies);
    doDrift(bodies, vel1, d2 * dt);

    computeAccelerations();
    auto a1 = accelerations;
    doKick(bodies, a1, k2 * dt);

    auto vel2 = getVelocities(bodies);
    doDrift(bodies, vel2, d3 * dt);

    computeAccelerations();
    auto a2 = accelerations;
    doKick(bodies, a2, k3 * dt);

    auto vel3 = getVelocities(bodies);
    doDrift(bodies, vel3, d4 * dt);

    for (auto *b : bodies)
        b->updateTrail(static_cast<float>(dt));
}
