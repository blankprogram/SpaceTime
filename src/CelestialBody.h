#pragma once

#include "raii.h" // VertexArray, Buffer, Texture2D, RingBuffer
#include <glm/glm.hpp>
#include <vector>

struct TrailPoint {
    glm::vec3 position;
    float life;
};

class CelestialBody {
  public:
    CelestialBody(double mass, const glm::dvec3 &initialPos,
                  const glm::dvec3 &initialVel, float scale,
                  const char *texturePath, const glm::vec3 &trailColor);
    ~CelestialBody() noexcept;

    void updateTrail(float dt);
    glm::dvec3
    computeAcceleration(const std::vector<CelestialBody *> &others) const;

    const glm::dvec3 &getPosition() const noexcept { return pos_; }
    const glm::vec3 &getTrailColor() const noexcept { return trailColor_; }
    float getScale() const noexcept { return scale_; }

    void bindMeshAndTexture() const noexcept;
    void drawMesh() const noexcept;
    void drawTrail() const noexcept;

    double getMass() const noexcept { return mass_; }
    glm::dvec3 getVelocity() const noexcept { return vel_; }
    void setPosition(const glm::dvec3 &p) noexcept { pos_ = p; }
    void setVelocity(const glm::dvec3 &v) noexcept { vel_ = v; }

  private:
    double mass_;
    glm::dvec3 pos_, vel_;
    float scale_;

    VertexArray sphereVAO_;
    Buffer sphereVBO_, sphereEBO_;
    Texture2D texture_;
    GLsizei indexCount_;

    VertexArray trailVAO_;
    Buffer trailVBO_;

    static constexpr size_t MAX_TRAILS = 1000;
    RingBuffer<TrailPoint, MAX_TRAILS> trail_;
    float sampleAcc_ = 0.0f;
    glm::vec3 trailColor_;

    void initMesh();
    void initTrail();
    void rebuildTrailBuffer();
    unsigned loadTexture(const char *path) noexcept;
};
