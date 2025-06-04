#pragma once

#include <glad/glad.h>
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
    ~CelestialBody();

    void updateTrail(float dt);
    glm::dvec3
    computeAcceleration(const std::vector<CelestialBody *> &others) const;

    const glm::dvec3 &getPosition() const { return position; }
    const glm::dvec3 &getVelocity() const { return velocity; }
    const glm::vec3 &getTrailColor() const { return trailColor; }
    float getMass() const { return mass; }
    float getScale() const { return scale; }

    void setPosition(const glm::dvec3 &p) { position = p; }
    void setVelocity(const glm::dvec3 &v) { velocity = v; }

    void bindMeshAndTexture() const;
    void drawMesh() const;
    void drawTrail() const;

  private:
    double mass;
    glm::dvec3 position;
    glm::dvec3 velocity;
    double scale;

    GLuint sphereVAO = 0, sphereVBO = 0, sphereEBO = 0;
    GLuint textureID = 0;
    GLsizei indexCount = 0;

    std::vector<TrailPoint> trail;
    glm::vec3 trailColor;
    GLuint trailVAO = 0, trailVBO = 0;
    float sampleAccumulator = 0.0f;

    static constexpr float POINT_LIFETIME = 1000.0f;
    static constexpr float SAMPLE_INTERVAL = 0.025f;
    static constexpr size_t MAX_TRAIL_POINTS = 1000;

    void initMesh();
    void initTrailBuffer();
    GLuint loadTextureFromFile(const char *path);
    void sampleTrailPoint();
    void rebuildTrailBuffer();
};
