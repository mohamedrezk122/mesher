#pragma once

#include <glm/detail/func_geometric.hpp>
#include <glm/ext.hpp>
#include <glm/vec3.hpp>
#include <optional>

#include "../mesh.hpp"


struct BVHNode {
    AABB box;
    uint32_t left;
    uint32_t first_prim_idx, prim_count;
    bool isleaf(){
        return prim_count > 0;
    }
    BVHNode() = default;
};

class BVH {
public:
    BVH() = default;
    BVH(Mesh &mesh);

    BVHNode& get_node(uint32_t idx){
        return nodes[idx];
    }

    Triangle& get_triangle(uint32_t idx){
        return mesh->triangles[tris[idx]];
    }

    Mesh *mesh;
private:
    std::vector<BVHNode> nodes;
    std::vector<uint32_t> tris;
    uint32_t counter = 1;
    void update_bounds(uint32_t node_idx);
    void subdivide_primitives(uint32_t node_idx);
};

struct Ray {
    glm::vec3 origin;
    // unit vector
    glm::vec3 dir;
    // constructors
    Ray(glm::vec3 &pos, glm::vec3 &target);
    Ray() = default;

    // intersection tests
    std::optional<float> intersects_triangle(Mesh *mesh, Triangle &tri);
    std::optional<float> intersects_aabb_vectorized(const AABB &box);
    std::optional<float> intersects_aabb(const AABB &box);
    std::optional<uint32_t> intersects_bvh(BVH &bvh);
    // distance computation
    float dist_to_aabb(const AABB &box);
    
private:
    std::optional<uint32_t> intersects_bvh_internal(BVH &bvh, uint32_t idx);
};

Ray mouse_to_object_space(glm::vec2 mouse, glm::vec4 viewport,
                          glm::mat4 &view_model, glm::mat4 &proj);


// std::optional<uint32_t> check_intersection(glm::vec2 mouse, glm::vec4 viewport,
//                                            Mesh &mesh, glm::mat4 &view_matrix,
//                                            glm::mat4 &proj);

std::optional<uint32_t> check_intersection(glm::vec2 mouse, glm::vec4 viewport,
                                           BVH &bvh, glm::mat4 &view_matrix,
                                           glm::mat4 &proj);
