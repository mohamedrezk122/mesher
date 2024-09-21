#include "../include/model.hpp"
#include <cstdlib>

Model::Model(std::string filepath) {
    trans_vector = glm::vec3(0.0f);
    model_matrix = glm::mat4(1.0f);
    Assimp::Importer importer;
    const aiScene *scene =
        importer.ReadFile(filepath, aiProcess_Triangulate | aiProcess_FlipUVs);
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE ||
        !scene->mRootNode) {
        std::cerr << "ERROR::ASSIMP::" << importer.GetErrorString()
                  << std::endl;
        return;
    }
    process_node(scene->mRootNode, scene);
    assert(meshes.size() == 1);
    model_matrix = meshes[0].model_matrix;
    rotate(-90, glm::vec3(1.0f, 0.0f, 0.0f));
}

void Model::process_node(aiNode *node, const aiScene *scene) {
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        unsigned int index = node->mMeshes[i];
        aiMesh *mesh = scene->mMeshes[index];
        process_mesh(mesh, scene);
    }
    // recursively process nodes
    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        aiNode *child = node->mChildren[i];
        process_node(child, scene);
    }
}

void Model::process_mesh(aiMesh *mesh, const aiScene *scene) {
    glm::vec3 bounding_box(0.0f);
    std::vector<Vertex> vertices;
    vertices.reserve(mesh->mNumVertices + 1);
    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        auto vec = mesh->mVertices[i];

        bounding_box.x = std::max(std::abs(vec.x), bounding_box.x);
        bounding_box.y = std::max(std::abs(vec.y), bounding_box.y);
        bounding_box.z = std::max(std::abs(vec.z), bounding_box.z);

        vertices.push_back(Vertex{
            .position = glm::vec3(vec.x, vec.y, vec.z),
            .color = glm::vec3(0.753f),
            .normal = glm::vec3(0.0f),
        });
    }

    std::vector<unsigned int> indices;
    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        if (face.mNumIndices > 3)
            std::cout << face.mNumIndices << std::endl;
        for (unsigned int j = 0; j < face.mNumIndices; j++) {
            indices.push_back(face.mIndices[j]);
        }
    }

    // calculating vertex normals
    for (uint32_t i = 0; i < indices.size(); i += 3) {

        glm::vec3 A = vertices[indices[i + 0]].position;
        glm::vec3 B = vertices[indices[i + 1]].position;
        glm::vec3 C = vertices[indices[i + 2]].position;

        glm::vec3 normal = glm::cross(B - A, C - A);

        vertices[indices[i + 0]].normal += normal;
        vertices[indices[i + 1]].normal += normal;
        vertices[indices[i + 2]].normal += normal;
    }

    auto ratio = 1.45f / std::max(bounding_box.x,
                                  std::max(bounding_box.y, bounding_box.z));
    Mesh mymesh(vertices, indices);
    mymesh = mymesh.scale(ratio);
    // center mesh
    mymesh = mymesh.translate(ratio * bounding_box / 2.0f);
    meshes.push_back(mymesh);
}

void Model::draw(Shader &shader) {
    for (auto &mesh : meshes)
        mesh.draw(shader);
}

Model Model::scale(float s) {
    model_matrix = glm::scale(model_matrix, glm::vec3(s));
    return *this;
}

Model Model::scale(glm::vec3 s) {
    model_matrix = glm::scale(model_matrix, s);
    return *this;
}

Model Model::translate(float t) {
    model_matrix = glm::translate(model_matrix, glm::vec3(t));
    return *this;
}
Model Model::translate(glm::vec3 t) {
    model_matrix = glm::translate(model_matrix, t);
    return *this;
}

Model Model::translate_x(float dx) {
    trans_vector.x += dx;
    return translate(trans_vector);
}

Model Model::translate_y(float dy) {
    trans_vector.y += dy;
    return translate(trans_vector);
}

Model Model::translate_z(float dz) {
    trans_vector.z += dz;
    return translate(trans_vector);
}

Model Model::rotate(float angle, glm::vec3 axis) {
    model_matrix = glm::rotate(model_matrix, glm::radians(angle), axis);
    return *this;
}

glm::mat4 Model::get_model_matrix() { return model_matrix; }
