#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>

class ComputeShader {
  public:
    ComputeShader(const char *path);
    void dispatch(int count);
    void bind() const;
    GLuint id() const { return program_; }

  private:
    GLuint program_{};
};
