#pragma once

#include <glm/gtc/matrix_transform.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <vector>

#include "glad.h"
#include "shader.hpp"

class Mesh {
  public:

    struct Vertex {
        glm::vec3 position;
        glm::vec3 color;
        glm::vec3 normal;
    };

    std::vector<Vertex> vertices;
    std::vector<GLuint> faces;
    glm::mat4 model_matrix;

    // constructor
    Mesh(std::vector<Vertex> _vertices, std::vector<GLuint> faces);

    void draw(Shader &shader);

    glm::mat4 get_model_matrix();
    Mesh scale(float s);
    Mesh scale(glm::vec3 s);

    Mesh translate(float t);
    Mesh translate(glm::vec3 t);
    Mesh rotate(float angle, glm::vec3 axis);

  private:
    GLuint VAO, VBO, EBO;
    void setup_mesh();
};