#include "CelestialBody.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <algorithm>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <numbers>
#include <span>
#include <vector>
static constexpr float POINT_LIFE = 30.0f;
static constexpr float SAMPLE_INTV = 0.05f;
static constexpr double G_CONST = 0.5;

CelestialBody::CelestialBody(double mass, const glm::dvec3 &initialPos,
                             const glm::dvec3 &initialVel, float scale,
                             const char *texturePath,
                             const glm::vec3 &trailColor)
    : mass_{mass}, pos_{initialPos}, vel_{initialVel}, scale_{scale},
      trailColor_{trailColor} {
    initMesh();
    initTrail();
    texture_.id = loadTexture(texturePath);
}

CelestialBody::~CelestialBody() noexcept = default;

void CelestialBody::updateTrail(float dt) {
    sampleAcc_ += dt;
    while (sampleAcc_ >= SAMPLE_INTV) {
        sampleAcc_ -= SAMPLE_INTV;
        trail_.push({glm::vec3(pos_), POINT_LIFE});
    }
    trail_.for_each([&](TrailPoint &tp) { tp.life -= dt; });
    rebuildTrailBuffer();
}

void CelestialBody::initMesh() {
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    constexpr int sectorCount = 36, stackCount = 18;
    constexpr float radius = 1.0f;

    for (int i = 0; i <= stackCount; ++i) {
        float stackAng = std::numbers::pi_v<float> / 2 -
                         i * std::numbers::pi_v<float> / stackCount;
        float xy = radius * std::cos(stackAng);
        float z = radius * std::sin(stackAng);
        for (int j = 0; j <= sectorCount; ++j) {
            float sectorAng = j * 2 * std::numbers::pi_v<float> / sectorCount;
            float x = xy * std::cos(sectorAng);
            float y = xy * std::sin(sectorAng);
            float u = float(j) / sectorCount;
            float v = float(i) / stackCount;
            vertices.insert(vertices.end(), {x, y, z, u, v});
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
            if (i != stackCount - 1) {
                indices.push_back(k1 + 1);
                indices.push_back(k2);
                indices.push_back(k2 + 1);
            }
        }
    }

    indexCount_ = static_cast<GLsizei>(indices.size());

    glBindVertexArray(sphereVAO_.id);
    glBindBuffer(GL_ARRAY_BUFFER, sphereVBO_.id);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float),
                 vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereEBO_.id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int),
                 indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                          (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);
}

void CelestialBody::initTrail() {
    glBindVertexArray(trailVAO_.id);
    glBindBuffer(GL_ARRAY_BUFFER, trailVBO_.id);
    glBufferData(GL_ARRAY_BUFFER,
                 MAX_TRAILS * (sizeof(glm::vec3) + sizeof(float)), nullptr,
                 GL_DYNAMIC_DRAW);

    GLsizei stride = sizeof(glm::vec3) + sizeof(float);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, nullptr);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, stride,
                          (void *)sizeof(glm::vec3));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);
}

void CelestialBody::rebuildTrailBuffer() {
    std::vector<float> data;
    data.reserve(trail_.size() * 4);
    trail_.for_each([&](const TrailPoint &tp) {
        float lf = std::max(tp.life / POINT_LIFE, 0.0f);
        data.push_back(tp.position.x);
        data.push_back(tp.position.y);
        data.push_back(tp.position.z);
        data.push_back(lf);
    });

    std::span<const float> span{data};

    glBindBuffer(GL_ARRAY_BUFFER, trailVBO_.id);
    glBufferSubData(GL_ARRAY_BUFFER, 0, span.size_bytes(), span.data());
}

unsigned CelestialBody::loadTexture(const char *path) noexcept {
    int w, h, n;
    stbi_set_flip_vertically_on_load(true);
    auto *img = stbi_load(path, &w, &h, &n, 0);
    if (!img) {
        std::cerr << "Failed to load texture: " << path << "\n";
        return 0u;
    }
    GLenum fmt = (n == 4 ? GL_RGBA : GL_RGB);
    glBindTexture(GL_TEXTURE_2D, texture_.id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, fmt, w, h, 0, fmt, GL_UNSIGNED_BYTE, img);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(img);
    return texture_.id;
}

void CelestialBody::bindMeshAndTexture() const noexcept {
    glBindVertexArray(sphereVAO_.id);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture_.id);
}

void CelestialBody::drawMesh() const noexcept {
    glDrawElements(GL_TRIANGLES, indexCount_, GL_UNSIGNED_INT, nullptr);
}

void CelestialBody::drawTrail() const noexcept {
    glBindVertexArray(trailVAO_.id);
    glDrawArrays(GL_POINTS, 0, static_cast<GLsizei>(trail_.size()));
}
