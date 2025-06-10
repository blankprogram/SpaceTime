#include "GravityWell.h"
#include "CelestialBody.h"

#include <cmath>
#include <glm/gtc/type_ptr.hpp>

GravityWell::GravityWell(float size, int resolution)
    : size_{size}, resolution_{resolution} {
    setupBuffers();
}

GravityWell::~GravityWell() noexcept = default;

void GravityWell::setupBuffers() noexcept {
    int N = 2 * resolution_ + 1;
    int totalLines = 2 * (N - 1) * N;
    vertexCount_ = totalLines * 2;
    cpuBuffer_.resize(vertexCount_ * 3);

    glBindVertexArray(vao_.id);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_.id);
    glBufferData(GL_ARRAY_BUFFER, cpuBuffer_.size() * sizeof(float), nullptr,
                 GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
}

void GravityWell::updateFromBodies(const std::vector<CelestialBody *> &bodies,
                                   float G) noexcept {
    int N = 2 * resolution_ + 1;
    float step = size_ / resolution_;

    std::vector<float> yGrid(N * N);
    for (int j = 0; j < N; ++j) {
        for (int i = 0; i < N; ++i) {
            float x = (i - resolution_) * step;
            float z = (j - resolution_) * step;
            float rawY = 0.0f;
            for (auto *b : bodies) {
                auto p = b->getPosition();
                float dx = x - float(p.x);
                float dz = z - float(p.z);
                float d = std::sqrt(dx * dx + dz * dz + 0.1f);
                rawY += -G * float(b->getMass()) / d;
            }
            yGrid[j * N + i] = rawY;
        }
    }

    constexpr float K = 0.2f;
    constexpr float SCALE = 15.0f;
    auto warp = [&](float v) {
        float t = (2.0f / 3.14159265f) * std::atan(v * K);
        return t * SCALE;
    };

    float *ptr = cpuBuffer_.data();
    for (int j = 0; j < N; ++j) {
        for (int i = 0; i < N - 1; ++i) {
            float x0 = (i - resolution_) * step;
            float x1 = (i + 1 - resolution_) * step;
            float z0 = (j - resolution_) * step;
            float y0 = warp(yGrid[j * N + i]);
            float y1 = warp(yGrid[j * N + (i + 1)]);

            *ptr++ = x0;
            *ptr++ = y0;
            *ptr++ = z0;
            *ptr++ = x1;
            *ptr++ = y1;
            *ptr++ = z0;
        }
    }
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N - 1; ++j) {
            float x0 = (i - resolution_) * step;
            float z0 = (j - resolution_) * step;
            float z1 = (j + 1 - resolution_) * step;
            float y0 = warp(yGrid[j * N + i]);
            float y1 = warp(yGrid[(j + 1) * N + i]);

            *ptr++ = x0;
            *ptr++ = y0;
            *ptr++ = z0;
            *ptr++ = x0;
            *ptr++ = y1;
            *ptr++ = z1;
        }
    }

    glBindBuffer(GL_ARRAY_BUFFER, vbo_.id);
    glBufferSubData(GL_ARRAY_BUFFER, 0,
                    GLsizei(cpuBuffer_.size() * sizeof(float)),
                    cpuBuffer_.data());
}

void GravityWell::draw() const noexcept {
    glBindVertexArray(vao_.id);
    glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(vertexCount_));
}
