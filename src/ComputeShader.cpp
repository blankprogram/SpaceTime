#include "ComputeShader.h"
#include <fstream>
#include <sstream>
#include <stdexcept>

ComputeShader::ComputeShader(const char *path) {
    std::ifstream file(path);
    std::stringstream ss;
    ss << file.rdbuf();
    std::string src = ss.str();
    const char *csrc = src.c_str();

    GLuint shader = glCreateShader(GL_COMPUTE_SHADER);
    glShaderSource(shader, 1, &csrc, nullptr);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char log[512];
        glGetShaderInfoLog(shader, 512, nullptr, log);
        throw std::runtime_error("Compute shader compilation failed: " +
                                 std::string(log));
    }

    program_ = glCreateProgram();
    glAttachShader(program_, shader);
    glLinkProgram(program_);
    glDeleteShader(shader);
}

void ComputeShader::bind() const { glUseProgram(program_); }

void ComputeShader::dispatch(int count) {
    glDispatchCompute((count + 127) / 128, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}
