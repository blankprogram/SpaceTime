#pragma once

#include <format>
#include <glad/glad.h>
#include <stdexcept>
#include <string>

[[nodiscard]]
static GLuint compileShader(GLenum type, const std::string &src) {
    GLuint shader = glCreateShader(type);
    const char *cstr = src.c_str();
    glShaderSource(shader, 1, &cstr, nullptr);
    glCompileShader(shader);

    GLint ok = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (ok != GL_TRUE) {
        char info[512];
        glGetShaderInfoLog(shader, sizeof(info), nullptr, info);
        glDeleteShader(shader);
        throw std::runtime_error(std::format(
            "Shader compile error ({}):\n{}",
            (type == GL_VERTEX_SHADER ? "VERTEX" : "FRAGMENT"), info));
    }
    return shader;
}

class ShaderProgram {
  public:
    [[nodiscard]]
    ShaderProgram(std::string_view vertSrc, std::string_view fragSrc) {
        GLuint vs = compileShader(GL_VERTEX_SHADER, std::string{vertSrc});
        GLuint fs = compileShader(GL_FRAGMENT_SHADER, std::string{fragSrc});

        id_ = glCreateProgram();
        glAttachShader(id_, vs);
        glAttachShader(id_, fs);
        glLinkProgram(id_);

        GLint linked = GL_FALSE;
        glGetProgramiv(id_, GL_LINK_STATUS, &linked);
        if (linked != GL_TRUE) {
            char info[512];
            glGetProgramInfoLog(id_, sizeof(info), nullptr, info);
            glDeleteProgram(id_);
            glDeleteShader(vs);
            glDeleteShader(fs);
            throw std::runtime_error(
                std::format("Program link error:\n{}", info));
        }

        glDeleteShader(vs);
        glDeleteShader(fs);
    }

    ~ShaderProgram() noexcept {
        if (id_ != 0)
            glDeleteProgram(id_);
    }

    void use() const noexcept { glUseProgram(id_); }

    [[nodiscard]]
    GLint uniformLocation(const char *name) const noexcept {
        return glGetUniformLocation(id_, name);
    }

  private:
    GLuint id_{0};
};
