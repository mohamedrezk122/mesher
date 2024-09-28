#pragma once

#include <glm/detail/func_geometric.hpp>
#include <glm/ext.hpp>
#include <glm/vec3.hpp>
#include <optional>

#include "mesh.hpp"

struct Ray {
    glm::vec3 origin;
    // unit vector
    glm::vec3 dir;
    // at which distance the ray hit an object along `dir`
    float t = 1e30;
    // constructors
    Ray(glm::vec3 &pos, glm::vec3 &target);
    Ray() = default;

    // intersection tests
    std::optional<float> intersects_triangle(const Triangle &tri);
    std::optional<float> intersects_aabb_vectorized(const AABB &box);
    std::optional<float> intersects_aabb(const AABB &box);
};

Ray mouse_to_object_space(glm::vec2 mouse, glm::vec4 viewport,
                          glm::mat4 &view_model, glm::mat4 &proj);

std::optional<Triangle> check_intersection(glm::vec2 mouse, glm::vec4 viewport,
                                           Mesh &mesh, glm::mat4 &view_matrix,
                                           glm::mat4 &proj);
