#include "../include/mesh.hpp"

Mesh::Mesh(std::vector<Vertex> _vertices, std::vector<GLuint> _indices) {
    vertices = _vertices;
    indices = _indices;
    // initialize with identity
    model_matrix = glm::mat4(1.0f);
    setup_mesh();
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
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * indices.size(),
                 indices.data(), GL_STATIC_DRAW);
}

void Mesh::draw(Shader &shader) {
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
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