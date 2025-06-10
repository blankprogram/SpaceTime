#include "Renderer.h"
#include <fstream>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <sstream>

std::string Renderer::loadFileToString(const char *path) {
    std::ifstream in(path);
    if (!in.is_open()) {
        std::cerr << "Failed to open shader file: " << path << "\n";
        return "";
    }
    std::ostringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

GLuint Renderer::compileSingleShader(GLenum type, const std::string &src) {
    GLuint shader = glCreateShader(type);
    const char *cstr = src.c_str();
    glShaderSource(shader, 1, &cstr, nullptr);
    glCompileShader(shader);

    GLint ok = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char info[512];
        glGetShaderInfoLog(shader, 512, nullptr, info);
        std::cerr << "Shader compile error ("
                  << (type == GL_VERTEX_SHADER ? "VERTEX" : "FRAGMENT")
                  << "):\n"
                  << info << "\n";
    }
    return shader;
}

GLuint Renderer::linkProgram(GLuint vs, GLuint fs) {
    GLuint prog = glCreateProgram();
    glAttachShader(prog, vs);
    glAttachShader(prog, fs);
    glLinkProgram(prog);

    GLint ok = 0;
    glGetProgramiv(prog, GL_LINK_STATUS, &ok);
    if (!ok) {
        char info[512];
        glGetProgramInfoLog(prog, 512, nullptr, info);
        std::cerr << "Program link error:\n" << info << "\n";
    }
    return prog;
}

Renderer::Renderer(int w, int h) : width(w), height(h) {
    initShaders();
    glViewport(0, 0, width, height);
}

Renderer::~Renderer() { cleanupShaders(); }

void Renderer::initShaders() {
    std::string bodyVS = loadFileToString("shaders/vertex.glsl");
    std::string bodyFS = loadFileToString("shaders/fragment.glsl");
    GLuint vs = compileSingleShader(GL_VERTEX_SHADER, bodyVS);
    GLuint fs = compileSingleShader(GL_FRAGMENT_SHADER, bodyFS);
    bodyShader = linkProgram(vs, fs);
    glDeleteShader(vs);
    glDeleteShader(fs);

    bodyMvpLoc = glGetUniformLocation(bodyShader, "u_MVP");
    bodyTexLoc = glGetUniformLocation(bodyShader, "u_Texture");

    std::string trailVS = loadFileToString("shaders/trail.vert");
    std::string trailFS = loadFileToString("shaders/trail.frag");
    vs = compileSingleShader(GL_VERTEX_SHADER, trailVS);
    fs = compileSingleShader(GL_FRAGMENT_SHADER, trailFS);
    trailShader = linkProgram(vs, fs);
    glDeleteShader(vs);
    glDeleteShader(fs);

    trailMvpLoc = glGetUniformLocation(trailShader, "u_MVP");

    std::string wellVS = loadFileToString("shaders/gravitywell.vert");
    std::string wellFS = loadFileToString("shaders/gravitywell.frag");
    GLuint wvs = compileSingleShader(GL_VERTEX_SHADER, wellVS);
    GLuint wfs = compileSingleShader(GL_FRAGMENT_SHADER, wellFS);
    wellShader = linkProgram(wvs, wfs);
    glDeleteShader(wvs);
    glDeleteShader(wfs);

    wellMvpLoc = glGetUniformLocation(wellShader, "u_MVP");
}

void Renderer::cleanupShaders() {
    glDeleteProgram(bodyShader);
    glDeleteProgram(trailShader);
}

void Renderer::setViewportSize(int w, int h) {
    width = w;
    height = h;
    glViewport(0, 0, width, height);
}

void Renderer::drawAll(const std::vector<CelestialBody *> &bodies,
                       const glm::mat4 &view, const glm::mat4 &proj) {
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    gravityWell.updateFromBodies(bodies, 0.5f);
    glUseProgram(wellShader);
    glm::mat4 mvp = proj * view * glm::mat4(1.0f);
    glUniformMatrix4fv(wellMvpLoc, 1, GL_FALSE, glm::value_ptr(mvp));
    gravityWell.draw();

    glUseProgram(bodyShader);
    for (auto *b : bodies) {
        glm::dvec3 Pd = b->getPosition();
        glm::vec3 Pf = glm::vec3(Pd.x, Pd.y, Pd.z);

        glm::mat4 model = glm::translate(glm::mat4(1.0f), Pf);
        model = glm::scale(model, glm::vec3(b->getScale()));
        glm::mat4 mvp = proj * view * model;

        glUniformMatrix4fv(bodyMvpLoc, 1, GL_FALSE, glm::value_ptr(mvp));
        glUniform1i(bodyTexLoc, 0);

        b->bindMeshAndTexture();
        b->drawMesh();
    }

    glUseProgram(trailShader);
    for (auto *b : bodies) {
        glm::mat4 mvp = proj * view * glm::mat4(1.0f);
        glUniformMatrix4fv(trailMvpLoc, 1, GL_FALSE, glm::value_ptr(mvp));

        const glm::vec3 &color = b->getTrailColor();
        glUniform3fv(glGetUniformLocation(trailShader, "u_TrailColor"), 1,
                     glm::value_ptr(color));

        b->drawTrail();
    }
}
