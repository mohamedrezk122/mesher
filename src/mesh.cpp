#include <iostream>

#include "mesh.hpp"

Mesh Mesh::highlight_triangle(uint32_t tri_idx) {
    Triangle &tri = triangles[tri_idx];
    glm::vec4 color{1.0f, 0.0f, 0.0f, 1.0f};
    std::vector<Mesh::Vertex> vertices;
    auto [v1, v2, v3] = get_triangle_vertices(tri); 
    // glm::vec3 normal = glm::normalize(glm::cross(v2-v1, v3-v1)); 
    glm::vec3 normal = glm::cross(v2-v1, v3-v1); 
    for (auto &v : {v1, v2, v3}) {
        // TODO: handle normals that point inside the mesh
        // offseting the trinagle vertices (z-buffer)
        // float x = glm::dot(tri.centroid, normal);
        // if(x < 0)
        //     normal = -1.0f * normal;
        glm::vec3 vv = v + normal * 0.5f;
        vertices.push_back(Mesh::Vertex{vv, color, normal});
    }
    std::vector<GLuint> indices{0, 2, 1};
    return Mesh(vertices, indices);
}

// depth -> x | width -> y |  height -> z
Mesh Mesh::construct_bounding_box() {
    AABB box = bounding_box;
    std::vector<GLfloat> vertices_{
        1.0f,  1.0f,  1.0f, 1.0f,  1.0f,  -1.0f,
        -1.0f, 1.0f,  1.0f, -1.0f, 1.0f,  -1.0f,

        1.0f,  -1.0f, 1.0f, 1.0f,  -1.0f, -1.0f,
        -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, -1.0f,
    };

    std::vector<GLuint> indices{0, 1, 2, 1, 3, 2, 0, 5, 1, 0, 4, 5,
                                4, 7, 5, 4, 6, 7, 6, 2, 3, 6, 3, 7,
                                6, 4, 2, 2, 4, 0, 3, 7, 1, 1, 7, 5};

    std::vector<Mesh::Vertex> vertices;
    for (uint32_t i = 0; i < vertices_.size(); i += 3) {
        vertices.push_back(Mesh::Vertex{
            glm::vec3(vertices_[i + 0], vertices_[i + 1], vertices_[i + 2]),
            glm::vec4(1.0f, 0.0f, 0.0f, 0.3f),
            glm::vec3(1.0f),
        });
    }
    glm::vec3 vec = glm::abs(box.max - box.min) * 0.5f;
    for (uint32_t i = 0; i < vertices.size(); i++) {
        // vertices[i].position = vertices[i].position + 1.5f * center;
        vertices[i].position = vertices[i].position * vec;
    }
    // box_mesh = box_mesh.scale(glm::abs(box.max-box.min));
    // box_mesh.rotate(-45, glm::vec3(1.0f, 0.0f, 0.0f));
    Mesh box_mesh(vertices, indices);
    return box_mesh;
}

static void process_mesh(aiMesh *mesh, Mesh &mymesh) {
    mymesh.vertices.reserve(mesh->mNumVertices + 1);
    for (uint32_t i = 0; i < mesh->mNumVertices; i++) {
        const auto vec = mesh->mVertices[i];

        mymesh.bounding_box.max.x = glm::max(vec.x, mymesh.bounding_box.max.x);
        mymesh.bounding_box.max.y = glm::max(vec.y, mymesh.bounding_box.max.y);
        mymesh.bounding_box.max.z = glm::max(vec.z, mymesh.bounding_box.max.z);

        mymesh.bounding_box.min.x = glm::min(vec.x, mymesh.bounding_box.min.x);
        mymesh.bounding_box.min.y = glm::min(vec.y, mymesh.bounding_box.min.y);
        mymesh.bounding_box.min.z = glm::min(vec.z, mymesh.bounding_box.min.z);

        Mesh::Vertex vertex{
            glm::vec3(vec.x, vec.y, vec.z),          // position
            glm::vec4(0.753f, 0.753f, 0.753f, 1.0f), // color
            glm::vec3(0.0f),                         // normal
        };
        mymesh.center += vertex.position;

        mymesh.vertices.push_back(vertex);
    }
    mymesh.center /= (float)mymesh.vertices.size();

    mymesh.faces.reserve(mesh->mNumFaces * 3 + 1);
    for (uint32_t i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        assert(face.mNumIndices == 3);
        mymesh.faces.push_back(face.mIndices[0]);
        mymesh.faces.push_back(face.mIndices[1]);
        mymesh.faces.push_back(face.mIndices[2]);

        // std::cout << face.mIndices[0] << " "
        //           << face.mIndices[1] << " "
        //           << face.mIndices[2] << " "
        //           << std::endl;
    }

    uint32_t idx = 0;
    // calculating vertex normals
    glm::vec3 A, B, C, normal;
    for (uint32_t i = 0; i < mymesh.faces.size(); i += 3) {
        A = mymesh.vertices[mymesh.faces[i + 0]].position;
        B = mymesh.vertices[mymesh.faces[i + 1]].position;
        C = mymesh.vertices[mymesh.faces[i + 2]].position;
        normal = glm::cross(B - A, C - A);

        glm::vec3 centroid = 0.3333f * (A + B + C);
        mymesh.triangles.push_back(Triangle{idx++, mymesh.faces[i], centroid});

        mymesh.vertices[mymesh.faces[i + 0]].normal += normal;
        mymesh.vertices[mymesh.faces[i + 1]].normal += normal;
        mymesh.vertices[mymesh.faces[i + 2]].normal += normal;
    }
}

static void process_node(aiNode *node, const aiScene *scene, Mesh &mymesh) {
    // TODO: working with one mesh for now
    aiNode *child = node->mChildren[0];
    uint32_t index = child->mMeshes[0];
    aiMesh *mesh = scene->mMeshes[index];
    process_mesh(mesh, mymesh);
}

Mesh::Mesh(std::vector<Vertex> _vertices, std::vector<GLuint> _faces) {
    vertices = _vertices;
    faces = _faces;
    setup_mesh();
}

Mesh::Mesh(std::string filepath) {
    Assimp::Importer importer;
    const aiScene *scene =
        importer.ReadFile(filepath, aiProcess_Triangulate | aiProcess_FlipUVs);
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE ||
        !scene->mRootNode) {
        std::cerr << "ERROR::ASSIMP::" << importer.GetErrorString()
                  << std::endl;
        return;
    }
    process_node(scene->mRootNode, scene, *this);
    setup_mesh();
    // center mesh
    glm::vec3 vec = glm::abs(bounding_box.max - bounding_box.min);
    auto ratio = 1.45f / glm::max(vec.x, vec.y, vec.z);
    // translate(ratio * bounding_box / 2.0f);
    // translate(-1.0f * center);
    scale(ratio);
    rotate(-90, glm::vec3(1.0f, 0.0f, 0.0f));
    center *= ratio;
}

void Mesh::setup_mesh() {

    // setup vertex array object
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // setup vertex buffer object
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(),
                 vertices.data(), GL_STATIC_DRAW);

    // specify the structure within VAO
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (void *)offsetof(Vertex, position));

    // colors
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (GLvoid *)offsetof(Vertex, color));

    // normals
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_TRUE, sizeof(Vertex),
                          (GLvoid *)offsetof(Vertex, normal));

    // setup element buffer object
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * faces.size(),
                 faces.data(), GL_STATIC_DRAW);
}

void Mesh::draw(Shader &shader) {
    (void)shader;
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glDrawElements(GL_TRIANGLES, faces.size(), GL_UNSIGNED_INT, 0);
}

Mesh Mesh::scale(float s) {
    model_matrix = glm::scale(model_matrix, glm::vec3(s));
    return *this;
}

Mesh Mesh::scale(glm::vec3 s) {
    model_matrix = glm::scale(model_matrix, s);
    return *this;
}

Mesh Mesh::translate(float t) {
    model_matrix = glm::translate(model_matrix, glm::vec3(t));
    return *this;
}

Mesh Mesh::translate(glm::vec3 t) {
    model_matrix = glm::translate(model_matrix, t);
    return *this;
}

Mesh Mesh::rotate(float angle, glm::vec3 axis) {
    model_matrix = glm::rotate(model_matrix, glm::radians(angle), axis);
    return *this;
}

glm::mat4 Mesh::get_model_matrix() { return model_matrix; }

std::array<glm::vec3, 3> Mesh::get_triangle_vertices(Triangle &tri) const {
    uint32_t v1_idx = tri.first_vertex_idx;
    return {vertices[v1_idx + 0].position, vertices[v1_idx + 1].position,
            vertices[v1_idx + 2].position};
}
