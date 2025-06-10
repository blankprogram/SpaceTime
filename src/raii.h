#pragma once
#include <array>
#include <cstddef>
#include <glad/glad.h>
#include <stdexcept>
#include <string>
#include <utility>

struct GlObject {
    GlObject() = default;
    explicit GlObject(GLuint id) : id{id} {}
    ~GlObject() {
        if (id)
            glDeleteBuffers(1, &id);
    }
    GlObject(GlObject &&o) noexcept : id(o.id) { o.id = 0; }
    GlObject &operator=(GlObject &&o) noexcept {
        std::swap(id, o.id);
        return *this;
    }
    GLuint id = 0;
};

struct VertexArray : GlObject {
    VertexArray() { glGenVertexArrays(1, &id); }
    ~VertexArray() {
        if (id)
            glDeleteVertexArrays(1, &id);
    }
};
struct Buffer : GlObject {
    Buffer() { glGenBuffers(1, &id); }
    ~Buffer() {
        if (id)
            glDeleteBuffers(1, &id);
    }
};
struct Texture2D : GlObject {
    Texture2D() { glGenTextures(1, &id); }
    ~Texture2D() {
        if (id)
            glDeleteTextures(1, &id);
    }
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
    }

    Program(const Program &) = delete;
    Program &operator=(const Program &) = delete;

    Program(Program &&o) noexcept : id(o.id) { o.id = 0; }
    Program &operator=(Program &&o) noexcept {
        std::swap(id, o.id);
        return *this;
    }

    void use() const noexcept { glUseProgram(id); }
    [[nodiscard]] GLint uniform(const char *name) const noexcept {
        return glGetUniformLocation(id, name);
    }

    GLuint id = 0;

    static Program fromSources(const std::string &vertSrc,
                               const std::string &fragSrc) {
        auto compile = [&](GLenum type, const std::string &src) {
            GLuint s = glCreateShader(type);
            const char *c = src.c_str();
            glShaderSource(s, 1, &c, nullptr);
            glCompileShader(s);
            GLint ok = 0;
            glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
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
        glAttachShader(p, vs);
        glAttachShader(p, fs);
        glLinkProgram(p);
        GLint ok = 0;
        glGetProgramiv(p, GL_LINK_STATUS, &ok);
        if (!ok) {
            char buf[512];
            glGetProgramInfoLog(p, sizeof(buf), nullptr, buf);
            glDeleteProgram(p);
            throw std::runtime_error(buf);
        }
        glDeleteShader(vs);
        glDeleteShader(fs);
        return Program{p};
    }
};
