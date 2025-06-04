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
  CelestialBody(float mass, const glm::vec3 &initialPos,
                const glm::vec3 &initialVel, float scale,
                const char *texturePath);
  ~CelestialBody();

  void updateTrail(float dt);

  glm::vec3
  computeAcceleration(const std::vector<CelestialBody *> &others) const;

  const glm::vec3 &getPosition() const { return position; }
  const glm::vec3 &getVelocity() const { return velocity; }
  float getMass() const { return mass; }
  float getScale() const { return scale; }

  void setPosition(const glm::vec3 &p) { position = p; }
  void setVelocity(const glm::vec3 &v) { velocity = v; }

  void bindMeshAndTexture() const;

  void drawMesh() const;

  void drawTrail() const;

private:
  float mass;
  glm::vec3 position;
  glm::vec3 velocity;
  float scale;

  GLuint sphereVAO = 0, sphereVBO = 0, sphereEBO = 0;
  GLuint textureID = 0;
  GLsizei indexCount = 0;

  std::vector<TrailPoint> trail;
  GLuint trailVAO = 0, trailVBO = 0;
  float sampleAccumulator = 0.0f;

  static constexpr float POINT_LIFETIME = 5.0f;
  static constexpr float SAMPLE_INTERVAL = 0.025f;
  static constexpr size_t MAX_TRAIL_POINTS = 1000;

  void initMesh();
  GLuint loadTextureFromFile(const char *path);
  void sampleTrailPoint();
  void rebuildTrailBuffer();
};
