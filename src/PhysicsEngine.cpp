#include "PhysicsEngine.h"

static const double α = 1.0 / (2.0 - std::cbrt(2.0));
static const double β = -std::cbrt(2.0) / (2.0 - std::cbrt(2.0));
static const double γ = α;

static const double d1 = α * 0.5;
static const double d2 = (α + β) * 0.5;
static const double d3 = (β + γ) * 0.5;
static const double d4 = γ * 0.5;

static const double k1 = α;
static const double k2 = β;
static const double k3 = γ;

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
        glm::dvec3 a_i(0.0);
        glm::dvec3 r_i = bodies[i]->getPosition();
        double m_i = bodies[i]->getMass();

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

static void doDrift(std::vector<CelestialBody *> &bodies,
                    const std::vector<glm::dvec3> &vels, double h) {
    for (size_t i = 0; i < bodies.size(); ++i) {
        glm::dvec3 r_old = bodies[i]->getPosition();
        glm::dvec3 v = vels[i];
        bodies[i]->setPosition(r_old + v * h);
    }
}

static void doKick(std::vector<CelestialBody *> &bodies,
                   const std::vector<glm::dvec3> &accs, double h) {
    for (size_t i = 0; i < bodies.size(); ++i) {
        glm::dvec3 v_old = bodies[i]->getVelocity();
        bodies[i]->setVelocity(v_old + accs[i] * h);
    }
}

void PhysicsEngine::step(double dt) {
    size_t n = bodies.size();
    if (n == 0)
        return;

    std::vector<glm::dvec3> vel0(n);
    for (size_t i = 0; i < n; ++i) {
        vel0[i] = bodies[i]->getVelocity();
    }

    doDrift(bodies, vel0, d1 * dt);

    // B) Recompute a₀ at new positions
    computeAccelerations();
    std::vector<glm::dvec3> a0 = accelerations;

    doKick(bodies, a0, k1 * dt);

    {
        std::vector<glm::dvec3> vel1(n);
        for (size_t i = 0; i < n; ++i)
            vel1[i] = bodies[i]->getVelocity();
        doDrift(bodies, vel1, d2 * dt);
    }

    computeAccelerations();
    std::vector<glm::dvec3> a1 = accelerations;

    doKick(bodies, a1, k2 * dt);

    {
        std::vector<glm::dvec3> vel2(n);
        for (size_t i = 0; i < n; ++i)
            vel2[i] = bodies[i]->getVelocity();
        doDrift(bodies, vel2, d3 * dt);
    }

    computeAccelerations();
    std::vector<glm::dvec3> a2 = accelerations;

    doKick(bodies, a2, k3 * dt);

    {
        std::vector<glm::dvec3> vel3(n);
        for (size_t i = 0; i < n; ++i)
            vel3[i] = bodies[i]->getVelocity();
        doDrift(bodies, vel3, d4 * dt);
    }

    for (auto *b : bodies) {
        b->updateTrail((float)dt);
    }
}
