#pragma once

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/extended_min_max.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <vector>

#include <assimp/Importer.hpp>
#include <assimp/mesh.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

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
    // initialize with identity
    glm::mat4 model_matrix{1.0f};
    glm::vec3 bounding_box{0.0f};

    // constructors
    Mesh(std::vector<Vertex> _vertices, std::vector<GLuint> faces);
    Mesh(std::string filepath);
    Mesh(){}

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