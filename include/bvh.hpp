#pragma once

#include <glm/detail/func_geometric.hpp>
#include <glm/ext.hpp>
#include <glm/vec3.hpp>
#include <optional>

#include "mesh.hpp"

// namespace bvh {

struct Ray {
    glm::vec3 origin;
    // unit vector
    glm::vec3 dir;
    // at which distance the ray hit an object along `dir`
    float t = 1e30;
    std::optional<float> intersects_triangle(const Triangle &tri);
    Ray(glm::vec3 &pos, glm::vec3 &target);
    Ray() = default;
};

Ray mouse_to_object_space(glm::vec2 mouse, glm::vec4 viewport,
                          glm::mat4 &view_model, glm::mat4 &proj);

std::optional<Triangle> check_intersection(glm::vec2 mouse, glm::vec4 viewport,
                                           Mesh &mesh, glm::mat4 &view_matrix,
                                           glm::mat4 &proj);

// } // bvh namespace