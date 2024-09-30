#include "../include/bvh.hpp"
#include <cfloat>
#include <chrono>

constexpr float EPSILON = std::numeric_limits<float>::epsilon();

Ray::Ray(glm::vec3 &pos, glm::vec3 &_origin) {
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
std::optional<float> Ray::intersects_triangle(const Mesh &mesh, Triangle &tri) {
    auto [v1, v2, v3] = mesh.get_triangle_vertices(tri);
    glm::vec3 edge1 = v2 - v1;
    glm::vec3 edge2 = v3 - v1;
    // the following two operation is the scalar triple product
    // which equals the determinant
    glm::vec3 d_cross_edge2 = cross(dir, edge2);
    float det = dot(edge1, d_cross_edge2);
    // the det is nearly zero, which means that the ray
    // is parallel to the triangle (prependicular to the triangle normal)
    if (glm::abs(det) < EPSILON)
        return std::nullopt;
    // at this point the ray hits the plane of the triangle
    // we want to make sure whether it hits the triangle itself and where
    float inv_det = 1.0f / det;
    glm::vec3 s = origin - v1;
    float u = inv_det * dot(s, d_cross_edge2);
    // outside the triangle
    if (u < 0 || u > 1)
        return std::nullopt;
    glm::vec3 s_cross_edge1 = cross(s, edge1);
    float v = inv_det * dot(dir, s_cross_edge1);
    // not valid barycentric coordinates range
    if (v < 0 || u + v > 1)
        return std::nullopt;
    float t = inv_det * dot(s_cross_edge1, edge2);
    if (t > EPSILON)
        return t;
    // no intersection otherwise
    return std::nullopt;
}

glm::vec3 rcp(const glm::vec3 &vec) {
    glm::vec3 res;
    res[0] = glm::abs(vec[0]) < EPSILON ? FLT_MAX : 1.0f / vec[0];
    res[1] = glm::abs(vec[1]) < EPSILON ? FLT_MAX : 1.0f / vec[1];
    res[2] = glm::abs(vec[2]) < EPSILON ? FLT_MAX : 1.0f / vec[2];
    return res;
}

float min_component(const glm::vec3 &vec) {
    return glm::min(vec[0], vec[1], vec[2]);
}

float max_component(const glm::vec3 &vec) {
    return glm::max(vec[0], vec[1], vec[2]);
}

std::optional<float> Ray::intersects_aabb(const AABB &box) {
    /*
        Testing if the ray intersects the bounding box defined by
        the overlap of three slaps enclosed by the planes of the bounding
        box.
    */

    float t_near = 0.0f;
    float t_far = FLT_MAX;
    for (int i = 0; i < 3; i++) {
        // if this component is parallel to the plane, also to avoid dividing by
        // zero
        if (glm::abs(dir[i]) < EPSILON) {
            // if the ray origin is outside the slap then intersection cannot
            // happen
            if (origin[i] < box.min[i] || origin[i] > box.max[i])
                return std::nullopt;
            continue;
        }
        // since normal vector of each box has 2 zero valued components that
        // are parallel to the plane, the third one has value of 1.0f since
        // it is a unit vector

        // here we deal with perpendicular component

        float denum = 1.0f / dir[i];
        float near = denum * (box.min[i] - origin[i]);
        float far = denum * (box.max[i] - origin[i]);

        if (near > far)
            std::swap(near, far);

        // update hit distances
        t_near = glm::max(near, t_near);
        t_far = glm::min(far, t_far);

        // no intersection
        if (t_far <= t_near)
            return std::nullopt;
    }
    return t_near;
}

// vectorized version takes around 2-5 microseconds, slower than the previous
// version, maybe the rcp function is slower
std::optional<float> Ray::intersects_aabb_vectorized(const AABB &box) {
    glm::vec3 inv_dir = rcp(dir);
    glm::vec3 near = (box.min - origin) * inv_dir;
    glm::vec3 far = (box.max - origin) * inv_dir;
    glm::vec3 near_vec = glm::min(near, far);
    glm::vec3 far_vec = glm::max(near, far);
    float t_near = max_component(near_vec);
    float t_far = min_component(far_vec);
    if (t_near >= t_far)
        return std::nullopt;
    return t_near;
}

/*
    ->> iteration #1:
    bruteforce checking
    takes around 5-20 ms for 46474 trimesh
    even if the ray misses the bounding box
    complexity: linear in number of triangles

    ->> iteration #2:
    first check if the ray intersects the bounding box
    takes 0ms for rays outside the box
    complexity: still linear in number of triangles

*/
std::optional<uint64_t> check_intersection(glm::vec2 mouse, glm::vec4 viewport,
                                           Mesh &mesh, glm::mat4 &view_matrix,
                                           glm::mat4 &proj) {

    glm::mat4 view_model = view_matrix * mesh.model_matrix;
    Ray ray = mouse_to_object_space(mouse, viewport, view_model, proj);
    // check if the ray hits the mesh bounding box, if not do an early exit
    if (!ray.intersects_aabb(mesh.bounding_box).has_value()) {
        return std::nullopt;
    }
    for (auto &triangle : mesh.triangles) {
        if (ray.intersects_triangle(mesh, triangle).has_value()) {
            return triangle.id;
        }
    }
    return std::nullopt;
}
