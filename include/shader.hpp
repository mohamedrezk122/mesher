#pragma once

#include <fstream>
#include <iostream>
#include <string>

#include <glm/mat4x4.hpp>

#include "glad.h"

class Shader {
  public:
    Shader() = default;
    Shader(const char *vertex_shader_path, const char *fragment_shader_path);

    std::string read_shader_from_file(const char *filepath);
    GLuint setup_shader(const char *filepath, const GLuint type);
    void use();

    void set_uniform(const char *var_name, float value);
    void set_uniform(const char *var_name, int value);
    void set_uniform(const char *var_name, bool value);
    void set_uniform(const char *var_name, glm::mat4 &matrix);
    void set_uniform(const char *var_name, glm::vec3 &vec);

  private:
    GLuint shader_program;  // shader program ID
    GLuint vertex_shader;   // vertex shader ID
    GLuint fragment_shader; // fragment shader ID
};
