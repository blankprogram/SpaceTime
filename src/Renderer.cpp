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
    : width_{w}, height_{h},
      bodyProg_{Program::fromSources(loadFile("shaders/vertex.glsl"),
                                     loadFile("shaders/fragment.glsl"))},
      trailProg_{Program::fromSources(loadFile("shaders/trail.vert"),
                                      loadFile("shaders/trail.frag"))},
      wellProg_{Program::fromSources(loadFile("shaders/gravitywell.vert"),
                                     loadFile("shaders/gravitywell.frag"))},
      gravityWell_{40.0f, 50} {

    glGenQueries(1, &queryMeshID_);
    glGenQueries(1, &queryTrailID_);
    glGenQueries(1, &queryWellID_);
    glViewport(0, 0, width_, height_);
}

Renderer::~Renderer() noexcept {
    glDeleteQueries(1, &queryMeshID_);
    glDeleteQueries(1, &queryTrailID_);
    glDeleteQueries(1, &queryWellID_);
};

void Renderer::setViewportSize(int w, int h) noexcept {
    width_ = w;
    height_ = h;
    glViewport(0, 0, width_, height_);
}

void Renderer::drawAll(const std::vector<CelestialBody *> &bodies,
                       const glm::mat4 &view, const glm::mat4 &proj) noexcept {
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    gravityWell_.updateFromBodies(bodies, 0.5f);
    wellProg_.use();
    {
        glm::mat4 mvp = proj * view;
        glUniformMatrix4fv(wellProg_.uniform("u_MVP"), 1, GL_FALSE,
                           glm::value_ptr(mvp));
    }
    glBeginQuery(GL_PRIMITIVES_GENERATED, queryWellID_);
    gravityWell_.draw();
    glEndQuery(GL_PRIMITIVES_GENERATED);
    glGetQueryObjectuiv(queryWellID_, GL_QUERY_RESULT, &wellPrimitives_);

    bodyProg_.use();

    glBeginQuery(GL_PRIMITIVES_GENERATED, queryMeshID_);
    for (auto *b : bodies) {
        glm::mat4 model =
            glm::translate(glm::mat4{1.0f}, glm::vec3(b->getPosition())) *
            glm::scale(glm::mat4{1.0f}, glm::vec3(b->getScale()));

        glm::mat4 mvp = proj * view * model;
        glUniformMatrix4fv(bodyProg_.uniform("u_MVP"), 1, GL_FALSE,
                           glm::value_ptr(mvp));
        glUniform1i(bodyProg_.uniform("u_Texture"), 0);

        b->bindMeshAndTexture();
        b->drawMesh();
    }

    glEndQuery(GL_PRIMITIVES_GENERATED);

    glGetQueryObjectuiv(queryMeshID_, GL_QUERY_RESULT, &meshPrimitives_);

    trailProg_.use();
    glDepthMask(GL_FALSE);
    glEnable(GL_DEPTH_TEST);
    GLint locMvp = trailProg_.uniform("u_MVP");
    GLint locColor = trailProg_.uniform("u_TrailColor");

    glBeginQuery(GL_PRIMITIVES_GENERATED, queryTrailID_);
    for (auto *b : bodies) {
        glm::mat4 mvp = proj * view;
        glUniformMatrix4fv(locMvp, 1, GL_FALSE, glm::value_ptr(mvp));
        glUniform3fv(locColor, 1, glm::value_ptr(b->getTrailColor()));
        b->drawTrail();
    }

    glEndQuery(GL_PRIMITIVES_GENERATED);

    glGetQueryObjectuiv(queryTrailID_, GL_QUERY_RESULT, &trailPrimitives_);

    glDepthMask(GL_TRUE);
}
