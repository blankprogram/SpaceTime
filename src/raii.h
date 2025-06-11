#pragma once

#include <array>
#include <cstddef>
#include <glad/glad.h>
#include <iostream>
#include <string>
#include <unordered_map>
#include <utility>

#define CHECK_GL()                                                             \
    do {                                                                       \
        GLenum err;                                                            \
        while ((err = glGetError()) != GL_NO_ERROR) {                          \
            std::cerr << "OpenGL error 0x" << std::hex << err << " at "        \
                      << __FILE__ << ":" << __LINE__ << std::dec << std::endl; \
        }                                                                      \
    } while (0)

struct GlObject {
    GlObject() = default;
    explicit GlObject(GLuint id) : id{id} {}
    GlObject(GlObject &&o) noexcept : id(o.id) { o.id = 0; }
    GlObject &operator=(GlObject &&o) noexcept {
        std::swap(id, o.id);
        return *this;
    }
    GLuint id = 0;
};

struct VertexArray : GlObject {
    VertexArray() {
        glGenVertexArrays(1, &id);
        CHECK_GL();
    }
    ~VertexArray() {
        if (id)
            glDeleteVertexArrays(1, &id);
        CHECK_GL();
    }
    VertexArray(const VertexArray &) = delete;
    VertexArray &operator=(const VertexArray &) = delete;
    VertexArray(VertexArray &&) = default;
    VertexArray &operator=(VertexArray &&) = default;
};

struct Buffer : GlObject {
    Buffer() {
        glGenBuffers(1, &id);
        CHECK_GL();
    }
    ~Buffer() {
        if (id)
            glDeleteBuffers(1, &id);
        CHECK_GL();
    }
    Buffer(const Buffer &) = delete;
    Buffer &operator=(const Buffer &) = delete;
    Buffer(Buffer &&) = default;
    Buffer &operator=(Buffer &&) = default;
};

struct Texture2D : GlObject {
    Texture2D() {
        glGenTextures(1, &id);
        CHECK_GL();
    }
    ~Texture2D() {
        if (id)
            glDeleteTextures(1, &id);
        CHECK_GL();
    }
    Texture2D(const Texture2D &) = delete;
    Texture2D &operator=(const Texture2D &) = delete;
    Texture2D(Texture2D &&) = default;
    Texture2D &operator=(Texture2D &&) = default;
};

template <class T, size_t N> class RingBuffer {
    std::array<T, N> buf_;
    size_t head_ = 0, count_ = 0;

  public:
    template <class F> void for_each(F f) {
        for (size_t i = 0; i < count_; ++i) {
            f(buf_[(head_ + i) % N]);
        }
    }

    template <class F> void for_each(F f) const {
        for (size_t i = 0; i < count_; ++i) {
            f(buf_[(head_ + i) % N]);
        }
    }

    void push(const T &item) {
        buf_[(head_ + count_) % N] = item;
        if (count_ < N)
            ++count_;
        else
            head_ = (head_ + 1) % N;
    }

    size_t size() const { return count_; }
    void clear() { head_ = count_ = 0; }
};

struct Program {
    Program() = default;
    explicit Program(GLuint id) : id{id} {}
    ~Program() noexcept {
        if (id)
            glDeleteProgram(id);
        CHECK_GL();
    }

    Program(const Program &) = delete;
    Program &operator=(const Program &) = delete;

    Program(Program &&o) noexcept
        : id(o.id), uniformLocations_(std::move(o.uniformLocations_)) {
        o.id = 0;
    }
    Program &operator=(Program &&o) noexcept {
        std::swap(id, o.id);
        uniformLocations_ = std::move(o.uniformLocations_);
        return *this;
    }

    void use() const noexcept {
        glUseProgram(id);
        CHECK_GL();
    }

    [[nodiscard]] GLint uniform(const char *name) const noexcept {
        auto it = uniformLocations_.find(name);
        return it != uniformLocations_.end() ? it->second : -1;
    }

    GLuint id = 0;
    std::unordered_map<std::string, GLint> uniformLocations_;

    static Program fromSources(const std::string &vertSrc,
                               const std::string &fragSrc) {
        auto compile = [&](GLenum type, const std::string &src) {
            GLuint s = glCreateShader(type);
            CHECK_GL();
            const char *c = src.c_str();
            glShaderSource(s, 1, &c, nullptr);
            CHECK_GL();
            glCompileShader(s);
            CHECK_GL();

            GLint ok = 0;
            glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
            CHECK_GL();
            if (!ok) {
                char buf[512];
                glGetShaderInfoLog(s, sizeof(buf), nullptr, buf);
                glDeleteShader(s);
                throw std::runtime_error(buf);
            }
            return s;
        };

        GLuint vs = compile(GL_VERTEX_SHADER, vertSrc);
        GLuint fs = compile(GL_FRAGMENT_SHADER, fragSrc);

        GLuint p = glCreateProgram();
        CHECK_GL();
        glAttachShader(p, vs);
        CHECK_GL();
        glAttachShader(p, fs);
        CHECK_GL();
        glLinkProgram(p);
        CHECK_GL();

        GLint ok = 0;
        glGetProgramiv(p, GL_LINK_STATUS, &ok);
        CHECK_GL();
        if (!ok) {
            char buf[512];
            glGetProgramInfoLog(p, sizeof(buf), nullptr, buf);
            glDeleteProgram(p);
            throw std::runtime_error(buf);
        }

        glDeleteShader(vs);
        glDeleteShader(fs);
        CHECK_GL();

        Program prg{p};
        prg.uniformLocations_["u_MVP"] = glGetUniformLocation(p, "u_MVP");
        CHECK_GL();
        prg.uniformLocations_["u_Texture"] =
            glGetUniformLocation(p, "u_Texture");
        CHECK_GL();
        prg.uniformLocations_["u_TrailColor"] =
            glGetUniformLocation(p, "u_TrailColor");
        CHECK_GL();

        return prg;
    }
};
