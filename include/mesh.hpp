#pragma once

#include <array>
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

// axis-aligned bounding box
struct AABB {
    glm::vec3 max{-1e30f};
    glm::vec3 min{1e30f};
};

struct Triangle {
    // index to Vertex in vertices
    uint64_t id;
    uint64_t first_vertex_idx;
    glm::vec3 centroid;
};

class Mesh {
  public:
    struct Vertex {
        glm::vec3 position;
        glm::vec4 color;
        glm::vec3 normal;
    };

    glm::vec3 center;
    std::vector<Triangle> triangles;
    std::vector<Vertex> vertices;
    std::vector<GLuint> faces;
    // initialize with identity
    glm::mat4 model_matrix{1.0f};
    AABB bounding_box;

    // constructors
    Mesh(std::vector<Vertex> _vertices, std::vector<GLuint> faces);
    Mesh(std::string filepath);
    Mesh() = default;

    void draw(Shader &shader);

    glm::mat4 get_model_matrix();
    Mesh scale(float s);
    Mesh scale(glm::vec3 s);

    Mesh translate(float t);
    Mesh translate(glm::vec3 t);
    Mesh rotate(float angle, glm::vec3 axis);

    Mesh highlight_triangle(uint32_t tri_idx);
    Mesh construct_bounding_box();

    std::array<glm::vec3, 3> get_triangle_vertices(Triangle &tri) const;

  private:
    GLuint VAO, VBO, EBO;
    void setup_mesh();
};
