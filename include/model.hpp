#pragma once

#include <vector>

#include <assimp/Importer.hpp>
#include <assimp/mesh.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

#include "mesh.hpp"
#include "shader.hpp"

class Model {
  public:
    Model(){}
    Model(std::string filepath);
    void draw(Shader &shader);

    glm::mat4 model_matrix;
    glm::vec3 trans_vector;

    glm::mat4 get_model_matrix();
    Model scale(float s);
    Model scale(glm::vec3 s);

    Model translate(float t);
    Model translate(glm::vec3 t);

    Model translate_x(float dx);
    Model translate_y(float dy);
    Model translate_z(float dz);

    Model rotate(float angle, glm::vec3 axis);

  private:
    std::vector<Mesh> meshes;
    std::string directory;

    void process_node(aiNode *node, const aiScene *scene);
    void process_mesh(aiMesh *mesh, const aiScene *scene);
};