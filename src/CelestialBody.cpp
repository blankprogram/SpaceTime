#include "CelestialBody.h"
#include <algorithm>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

static constexpr double G_CONST = 0.5;

CelestialBody::CelestialBody(double mass, const glm::dvec3 &initialPos,
                             const glm::dvec3 &initialVel, float scale,
                             const char *texturePath)
    : mass(mass), position(initialPos), velocity(initialVel), scale(scale) {
    initMesh();
    textureID = loadTextureFromFile(texturePath);

    glGenVertexArrays(1, &trailVAO);
    glGenBuffers(1, &trailVBO);
    glBindVertexArray(trailVAO);
    glBindBuffer(GL_ARRAY_BUFFER, trailVBO);
    glBufferData(GL_ARRAY_BUFFER,
                 MAX_TRAIL_POINTS * (sizeof(glm::vec3) + sizeof(float)),
                 nullptr, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                          sizeof(glm::vec3) + sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE,
                          sizeof(glm::vec3) + sizeof(float),
                          (void *)sizeof(glm::vec3));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);
}

CelestialBody::~CelestialBody() {
    glDeleteVertexArrays(1, &sphereVAO);
    glDeleteBuffers(1, &sphereVBO);
    glDeleteBuffers(1, &sphereEBO);
    glDeleteTextures(1, &textureID);

    glDeleteVertexArrays(1, &trailVAO);
    glDeleteBuffers(1, &trailVBO);
}

void CelestialBody::initMesh() {
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    const int sectorCount = 36;
    const int stackCount = 18;
    const float radius = 1.0f;
    const float PI = glm::pi<float>();

    for (int i = 0; i <= stackCount; ++i) {
        float stackAngle = PI / 2 - i * PI / stackCount;
        float xy = radius * cos(stackAngle);
        float z = radius * sin(stackAngle);

        for (int j = 0; j <= sectorCount; ++j) {
            float sectorAngle = j * 2 * PI / sectorCount;
            float x = xy * cos(sectorAngle);
            float y = xy * sin(sectorAngle);
            float u = float(j) / sectorCount;
            float v = float(i) / stackCount;

            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);
            vertices.push_back(u);
            vertices.push_back(v);
        }
    }

    for (int i = 0; i < stackCount; ++i) {
        int k1 = i * (sectorCount + 1);
        int k2 = k1 + sectorCount + 1;
        for (int j = 0; j < sectorCount; ++j, ++k1, ++k2) {
            if (i != 0) {
                indices.push_back(k1);
                indices.push_back(k2);
                indices.push_back(k1 + 1);
            }
            if (i != (stackCount - 1)) {
                indices.push_back(k1 + 1);
                indices.push_back(k2);
                indices.push_back(k2 + 1);
            }
        }
    }

    indexCount = static_cast<GLsizei>(indices.size());

    glGenVertexArrays(1, &sphereVAO);
    glGenBuffers(1, &sphereVBO);
    glGenBuffers(1, &sphereEBO);

    glBindVertexArray(sphereVAO);

    glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float),
                 vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int),
                 indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                          (void *)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                          (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

GLuint CelestialBody::loadTextureFromFile(const char *path) {
    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int w, h, n;
    stbi_set_flip_vertically_on_load(true);
    unsigned char *data = stbi_load(path, &w, &h, &n, 0);
    if (!data) {
        std::cerr << "Failed to load texture: " << path << "\n";
        return 0;
    }
    GLenum format = (n == 4 ? GL_RGBA : GL_RGB);
    glTexImage2D(GL_TEXTURE_2D, 0, format, w, h, 0, format, GL_UNSIGNED_BYTE,
                 data);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data);
    return tex;
}

void CelestialBody::updateTrail(float dt) {
    sampleAccumulator += dt;
    while (sampleAccumulator >= SAMPLE_INTERVAL) {
        sampleAccumulator -= SAMPLE_INTERVAL;
        sampleTrailPoint();
    }
    for (auto &tp : trail) {
        tp.life -= dt;
    }
    trail.erase(
        std::remove_if(trail.begin(), trail.end(),
                       [](const TrailPoint &tp) { return tp.life <= 0.0f; }),
        trail.end());

    rebuildTrailBuffer();
}

void CelestialBody::sampleTrailPoint() {
    glm::dvec3 Pd = position;
    glm::vec3 Pf = glm::vec3(Pd.x, Pd.y, Pd.z);

    if (trail.size() < MAX_TRAIL_POINTS) {
        trail.push_back({Pf, POINT_LIFETIME});
    } else {
        trail.erase(trail.begin());
        trail.push_back({Pf, POINT_LIFETIME});
    }
}

void CelestialBody::rebuildTrailBuffer() {
    std::vector<float> bufferData;
    bufferData.reserve(trail.size() * 4);
    for (auto &tp : trail) {
        float lifeFrac = tp.life / POINT_LIFETIME;
        lifeFrac = std::max(lifeFrac, 0.0f);
        bufferData.push_back(tp.position.x);
        bufferData.push_back(tp.position.y);
        bufferData.push_back(tp.position.z);
        bufferData.push_back(lifeFrac);
    }
    glBindBuffer(GL_ARRAY_BUFFER, trailVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0,
                    static_cast<GLsizeiptr>(bufferData.size() * sizeof(float)),
                    bufferData.data());
}

glm::dvec3 CelestialBody::computeAcceleration(
    const std::vector<CelestialBody *> &others) const {
    glm::dvec3 a(0.0);
    for (auto *o : others) {
        if (o == this)
            continue;
        glm::dvec3 diff = o->position - position; // both are dvec3
        double dist2 = glm::dot(diff, diff) + 1e-15;
        double invDist3 = 1.0 / (dist2 * std::sqrt(dist2));
        a += (G_CONST * o->mass) * (diff * invDist3);
    }
    return a;
}

void CelestialBody::bindMeshAndTexture() const {
    glBindVertexArray(sphereVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureID);
}

void CelestialBody::drawMesh() const {
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr);
}

void CelestialBody::drawTrail() const {
    glBindVertexArray(trailVAO);
    glDrawArrays(GL_POINTS, 0, static_cast<GLsizei>(trail.size()));
}
