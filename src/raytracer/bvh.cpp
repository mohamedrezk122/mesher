#include <cmath>
#include <numeric>
#include <cfloat>
#include <iostream>

#include "bvh.hpp"
#include "../utils.hpp"

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
std::optional<float> Ray::intersects_triangle(Mesh *mesh, Triangle &tri) {
    auto [v1, v2, v3] = mesh->get_triangle_vertices(tri);
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

float Ray::dist_to_aabb(const AABB& box){
    auto opt = intersects_aabb(box);
    if(opt.has_value()){
        return opt.value();
    }
    return INFINITY;
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
std::optional<uint32_t> check_intersection(glm::vec2 mouse, glm::vec4 viewport,
                                           BVH &bvh, glm::mat4 &view_matrix,
                                           glm::mat4 &proj) {

    glm::mat4 view_model = view_matrix * bvh.mesh->model_matrix;
    Ray ray = mouse_to_object_space(mouse, viewport, view_model, proj);
    return ray.intersects_bvh(bvh);
}


BVH::BVH(Mesh &_mesh){
    mesh = &_mesh;
    tris = std::vector<uint32_t>(mesh->triangles.size());
    std::iota(tris.begin(), tris.end(), 0);
    nodes.reserve(2 * mesh->triangles.size() - 1);
    std::fill(nodes.begin(), nodes.end(), BVHNode());
    nodes[0].left = 0;
    nodes[0].first_prim_idx = 0;
    nodes[0].prim_count = tris.size();
    nodes[0].box = mesh->bounding_box;
    subdivide_primitives(0);
}

void BVH::update_bounds(uint32_t node_idx){
    BVHNode& node = nodes[node_idx];
    for(uint32_t i = 0; i < node.prim_count; i++){
        for(auto& v: mesh->get_triangle_vertices(get_triangle(i + node.first_prim_idx))){
            node.box.max.x = glm::max(v.x, node.box.max.x);
            node.box.max.y = glm::max(v.y, node.box.max.y);
            node.box.max.z = glm::max(v.z, node.box.max.z);

            node.box.min.x = glm::min(v.x, node.box.min.x);
            node.box.min.y = glm::min(v.y, node.box.min.y);
            node.box.min.z = glm::min(v.z, node.box.min.z);
        }
    }
}

void BVH::subdivide_primitives(uint32_t node_idx){
    BVHNode& node = nodes[node_idx];
    if(node.prim_count <= 2)
        return;
    // compute the longest axis to split at
    glm::vec3 diff = node.box.max - node.box.min;
    int axis = 0; // x-axis
    if(diff.y > diff.x)
        axis = 1; // y-axis
    if(diff.z > diff[axis])
        axis = 2; // z-axis
    float split =  node.box.min[axis] + 0.5f * diff[axis];
    // we move all the triangles the are left to split pos to left of the array
    // and triangles that are right to the split pos to the right of the array
    int i = node.first_prim_idx;
    int j = i + node.prim_count - 1;
    while(i <= j){
        auto centroid = mesh->triangles[tris[i]].centroid[axis];
        if( centroid < split){
            i++;
        } else {
            std::swap(tris[i], tris[j--]);
        }
    }

    uint32_t left_count = i - node.first_prim_idx;
    // one of the splits is empty
    if (left_count == node.prim_count || left_count == 0){
        LOG(node.prim_count);
        return; 
    }

    uint32_t left_idx = counter++;
    uint32_t right_idx = counter++;

    nodes[left_idx].first_prim_idx = node.first_prim_idx;
    nodes[left_idx].prim_count = left_count;

    nodes[right_idx].first_prim_idx = i;
    nodes[right_idx].prim_count = node.prim_count - left_count;

    node.prim_count = 0;
    node.left = left_idx;

    update_bounds(left_idx);
    update_bounds(right_idx);

    subdivide_primitives(left_idx);
    subdivide_primitives(right_idx);
}


std::optional<uint32_t> Ray::intersects_bvh(BVH &bvh){
    return intersects_bvh_internal(bvh, 0);
}

std::optional<uint32_t> Ray::intersects_bvh_internal(BVH &bvh, uint32_t idx){
    static int count = 0;
    count ++;
    BVHNode& node = bvh.get_node(idx);
    if(!intersects_aabb(node.box).has_value())
        return std::nullopt;

    if(node.isleaf()){
        for(uint32_t i = 0; i < node.prim_count ; i++){
            LOG(node.prim_count);
            Triangle & tri =  bvh.get_triangle(node.first_prim_idx + i);
            auto opt = intersects_triangle(bvh.mesh, tri);
            if(opt.has_value()){
                LOG(count);
                count = 0;
                return tri.id;
            }
        }
        // LOG(count);
        count = 0;
        return std::nullopt;
    }
    float left_dist = dist_to_aabb(bvh.get_node(node.left).box);
    float right_dist = dist_to_aabb(bvh.get_node(node.left + 1).box);
    // if (left_dist < right_dist){
        auto opt = intersects_bvh_internal(bvh, node.left);
        if(opt.has_value()){
            return opt;
        }
    // }
    // fallback to right node if no intersection found in the left one
    return intersects_bvh_internal(bvh, node.left + 1);
}