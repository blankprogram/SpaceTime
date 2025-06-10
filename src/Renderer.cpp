// Renderer.cpp
#include "Renderer.h"
#include "CelestialBody.h"
#include <format>
#include <fstream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <sstream>
#include <stdexcept>

std::string Renderer::loadFile(const std::filesystem::path &path) {
    std::ifstream in{path, std::ios::binary};
    if (!in) {
        throw std::runtime_error(
            std::format("Failed to open shader file '{}'", path.string()));
    }
    std::ostringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

Renderer::Renderer(int w, int h)
    : width_{w}, height_{h}, bodyProg_{loadFile("shaders/vertex.glsl"),
                                       loadFile("shaders/fragment.glsl")},
      trailProg_{loadFile("shaders/trail.vert"),
                 loadFile("shaders/trail.frag")},
      wellProg_{loadFile("shaders/gravitywell.vert"),
                loadFile("shaders/gravitywell.frag")},
      gravityWell_{40.0f, 100} {
    glViewport(0, 0, width_, height_);
}

Renderer::~Renderer() noexcept = default;

void Renderer::setViewportSize(int w, int h) noexcept {
    width_ = w;
    height_ = h;
    glViewport(0, 0, width_, height_);
}

void Renderer::drawAll(const std::vector<CelestialBody *> &bodies,
                       const glm::mat4 &view, const glm::mat4 &proj) noexcept {
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // -- gravity well --
    gravityWell_.updateFromBodies(bodies, 0.5f);
    wellProg_.use();
    glm::mat4 mvp = proj * view;
    glUniformMatrix4fv(wellProg_.uniformLocation("u_MVP"), 1, GL_FALSE,
                       reinterpret_cast<const GLfloat *>(&mvp));
    gravityWell_.draw();

    bodyProg_.use();
    for (auto *b : bodies) {
        glm::vec3 posF = glm::vec3(b->getPosition());
        glm::mat4 model = glm::translate(glm::mat4{1.0f}, posF) *
                          glm::scale(glm::mat4{1.0f}, glm::vec3(b->getScale()));

        glm::mat4 bodyMvp = proj * view * model;
        glUniformMatrix4fv(bodyProg_.uniformLocation("u_MVP"), 1, GL_FALSE,
                           reinterpret_cast<const GLfloat *>(&bodyMvp));
        glUniform1i(bodyProg_.uniformLocation("u_Texture"), 0);

        b->bindMeshAndTexture();
        b->drawMesh();
    }

    trailProg_.use();
    GLint trailMvpLoc = trailProg_.uniformLocation("u_MVP");
    GLint colorLoc = trailProg_.uniformLocation("u_TrailColor");
    for (auto *b : bodies) {
        glm::mat4 tMvp = proj * view;
        glUniformMatrix4fv(trailMvpLoc, 1, GL_FALSE,
                           reinterpret_cast<const GLfloat *>(&tMvp));
        glUniform3fv(colorLoc, 1,
                     reinterpret_cast<const GLfloat *>(&b->getTrailColor()));
        b->drawTrail();
    }
}
