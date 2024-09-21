#include <cstdlib>

#include "../include/shader.hpp"

std::string Shader::read_shader_from_file(const char *filepath) {
    std::fstream file(filepath, std::fstream::in);

    if (!file.is_open()) {
        std::cerr << "Cannot open file" << std::endl;
        return "";
    }
    std::string contents = "";
    std::string line = "";
    while (!file.eof()) {
        std::getline(file, line);
        contents.append(line + "\n");
    }
    file.close();
    return contents;
}

GLuint Shader::setup_shader(const char *filepath, const GLuint shader_type) {
    int success;
    char infoLog[512];
    this->use();
    GLuint shader = glCreateShader(shader_type);
    std::string shader_source = read_shader_from_file(filepath);
    glShaderSource(shader, 1, (GLchar **)&shader_source, nullptr);
    glCompileShader(shader);
    // print compile errors if any
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n"
                  << infoLog << std::endl;
    };

    glAttachShader(shader_program, shader);
    return shader;
}

Shader::Shader() {}

Shader::Shader(const char *v_shader_path, const char *f_shader_path) {
    int success;
    char infoLog[512];
    shader_program = glCreateProgram();
    vertex_shader = setup_shader(v_shader_path, GL_VERTEX_SHADER);
    fragment_shader = setup_shader(f_shader_path, GL_FRAGMENT_SHADER);

    glLinkProgram(shader_program);

    glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shader_program, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n"
                  << infoLog << std::endl;
    }

    glValidateProgram(shader_program);

    // some cleanup
    glDetachShader(shader_program, vertex_shader);
    glDetachShader(shader_program, fragment_shader);
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
}

void Shader::set_uniform(const char *var_name, glm::mat4 &matrix) {
    GLint location = glGetUniformLocation(shader_program, var_name);
    if (location >= 0)
        glUniformMatrix4fv(location, 1, GL_FALSE, &matrix[0][0]);
    else {
        std::cout << "Cannot find uniform varible" << std::endl;
        exit(EXIT_FAILURE);
    }
}

void Shader::set_uniform(const char *var_name, glm::vec3 &vec) {
    GLint location = glGetUniformLocation(shader_program, var_name);
    if (location >= 0)
        glUniform3fv(location, 1, &vec[0]);
    else {
        std::cout << "Cannot find uniform varible" << std::endl;
        exit(EXIT_FAILURE);
    }
}

void Shader::use() { glUseProgram(shader_program); }