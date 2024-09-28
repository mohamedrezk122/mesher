#include "../include/bvh.hpp"

constexpr float EPSILON = std::numeric_limits<float>::epsilon();

using namespace glm;

Ray::Ray(vec3 &pos, vec3 &_origin) {
    origin = _origin;
    dir = normalize(pos - _origin);
}

Ray mouse_to_object_space(glm::vec2 mouse, glm::vec4 viewport,
                          glm::mat4 &view_model, glm::mat4 &proj) {
    Ray ray;
    mouse.y = viewport[3] - mouse.y;
    glm::vec3 A(mouse, 0.0f);
    glm::vec3 B(mouse, 1.0f);
    ray.origin = glm::unProject(A, view_model, proj, viewport);
    ray.dir = glm::unProject(B, view_model, proj, viewport) - ray.origin;
    return ray;
}

// Möller–Trumbore intersection algorithm
std::optional<float> Ray::intersects_triangle(const Triangle &tri) {
    vec3 edge1 = tri.v2 - tri.v1;
    vec3 edge2 = tri.v3 - tri.v1;
    // the following two operation is the scalar triple product
    // which equals the determinant
    vec3 d_cross_edge2 = cross(dir, edge2);
    float det = dot(edge1, d_cross_edge2);
    // the det is nearly zero, which means that the ray
    // is parallel to the triangle (prependicular to the triangle normal)
    if (det < EPSILON && det > -EPSILON)
        return std::nullopt;
    // at this point the ray hits the plane of the triangle
    // we want to make sure whether it hits the triangle itself and where
    float inv_det = 1.0f / det;
    vec3 s = origin - tri.v1;
    float u = inv_det * dot(s, d_cross_edge2);
    // outside the triangle
    if (u < 0 || u > 1)
        return std::nullopt;
    vec3 s_cross_edge1 = cross(s, edge1);
    float v = inv_det * dot(dir, s_cross_edge1);
    if (v < 0 || u + v > 1)
        return std::nullopt;
    float t = inv_det * dot(s_cross_edge1, edge2);
    if (t > EPSILON)
        return t;
    // no intersection otherwise
    return std::nullopt;
}

// bruteforce checking
// takes around 5-10 ms for 46474 trimesh
// complexity: linear in number of triangles
std::optional<Triangle> check_intersection(glm::vec2 mouse, glm::vec4 viewport,
                                           Mesh &mesh, glm::mat4 &view_matrix,
                                           glm::mat4 &proj) {
    glm::mat4 view_model = view_matrix * mesh.model_matrix;
    Ray ray = mouse_to_object_space(mouse, viewport, view_model, proj);
    for (const auto &triangle : mesh.triangles) {
        if (auto t = ray.intersects_triangle(triangle)) {
            std::cout << "done" << std::endl;
            return triangle;
        }
    }
    return std::nullopt;
}