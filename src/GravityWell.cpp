#include "GravityWell.h"
#include "CelestialBody.h"

#include <cmath>

GravityWell::GravityWell(float size, int resolution)
    : size(size), resolution(resolution) {
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    const int N = 2 * resolution + 1;
    const int numLines = 2 * (N - 1) * N;
    const int floatsPerVertex = 3;
    glBufferData(GL_ARRAY_BUFFER,
                 numLines * 2 * floatsPerVertex * sizeof(float), nullptr,
                 GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
}

GravityWell::~GravityWell() {
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
}

void GravityWell::updateFromBodies(const std::vector<CelestialBody *> &bodies,
                                   float G) {
    const int N = 2 * resolution + 1;
    const float step = size / resolution;

    std::vector<float> yGrid(N * N, 0.0f);
    for (int j = 0; j < N; ++j) {
        for (int i = 0; i < N; ++i) {
            float x = (i - resolution) * step;
            float z = (j - resolution) * step;
            float y = 0.0f;
            for (auto *body : bodies) {
                glm::dvec3 p = body->getPosition();
                float dx = x - static_cast<float>(p.x);
                float dz = z - static_cast<float>(p.z);
                float dist = std::sqrt(dx * dx + dz * dz + 0.1f);
                y += -G * static_cast<float>(body->getMass()) / dist;
            }
            yGrid[j * N + i] = y;
        }
    }

    const int totalLines = 2 * (N - 1) * N;
    vertexCount = totalLines * 2;
    std::vector<float> vertices(vertexCount * 3);
    float *ptr = vertices.data();

    for (int j = 0; j < N; ++j) {
        for (int i = 0; i < N - 1; ++i) {
            float x0 = (i - resolution) * step;
            float x1 = (i + 1 - resolution) * step;
            float z = (j - resolution) * step;
            float y0 = yGrid[j * N + i];
            float y1 = yGrid[j * N + (i + 1)];

            *ptr++ = x0;
            *ptr++ = y0;
            *ptr++ = z;
            *ptr++ = x1;
            *ptr++ = y1;
            *ptr++ = z;
        }
    }

    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N - 1; ++j) {
            float x = (i - resolution) * step;
            float z0 = (j - resolution) * step;
            float z1 = (j + 1 - resolution) * step;
            float y0 = yGrid[j * N + i];
            float y1 = yGrid[(j + 1) * N + i];

            *ptr++ = x;
            *ptr++ = y0;
            *ptr++ = z0;
            *ptr++ = x;
            *ptr++ = y1;
            *ptr++ = z1;
        }
    }

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(float),
                    vertices.data());
}

void GravityWell::draw() const {
    glBindVertexArray(vao);
    glDrawArrays(GL_LINES, 0, vertexCount);
}
