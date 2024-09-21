#include "../include/mesh.hpp"

static void process_mesh(aiMesh *mesh, const aiScene *scene, Mesh &mymesh) {
    mymesh.vertices.reserve(mesh->mNumVertices + 1);
    for (uint32_t i = 0; i < mesh->mNumVertices; i++) {
        const auto vec = mesh->mVertices[i];

        mymesh.bounding_box.x = std::max(std::abs(vec.x), mymesh.bounding_box.x);
        mymesh.bounding_box.y = std::max(std::abs(vec.y), mymesh.bounding_box.y);
        mymesh.bounding_box.z = std::max(std::abs(vec.z), mymesh.bounding_box.z);

        mymesh.vertices.push_back(Mesh::Vertex{
            .position = glm::vec3(vec.x, vec.y, vec.z),
            .color = glm::vec3(0.753f),
            .normal = glm::vec3(0.0f),
        });
    }

    mymesh.faces.reserve(mesh->mNumFaces * 3 + 1);
    for (uint32_t i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        assert(face.mNumIndices == 3); 
        mymesh.faces.push_back(face.mIndices[0]);
        mymesh.faces.push_back(face.mIndices[1]);
        mymesh.faces.push_back(face.mIndices[2]);
    }

    // calculating vertex normals
    glm::vec3 A, B, C, normal;
    for (uint32_t i = 0; i < mymesh.faces.size(); i += 3) {
        A = mymesh.vertices[mymesh.faces[i + 0]].position;
        B = mymesh.vertices[mymesh.faces[i + 1]].position;
        C = mymesh.vertices[mymesh.faces[i + 2]].position;

        normal = glm::cross(B - A, C - A);
        
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
    process_mesh(mesh, scene, mymesh);
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
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cerr << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
        return;
    }
    process_node(scene->mRootNode, scene, *this);
    setup_mesh();
    // center mesh
    auto ratio = 1.45f / glm::max(bounding_box.x, bounding_box.y, bounding_box.z);
    translate(ratio * bounding_box / 2.0f);
    scale(ratio);
    rotate(-90, glm::vec3(1.0f, 0.0f, 0.0f));

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
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
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