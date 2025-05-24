#pragma once


namespace GLT::geometry {

    #pragma pack(push, 1)
    struct BVH_node {
        glm::vec3   AABB_min;
        u32         left_node;          // Index of left child; right child is left_node + 1
        glm::vec3   AABB_max;
        u32         first_tri_index;    // Index into triIdx array for leaves
        glm::vec3   buffer;
        u32         tri_count;          // 0 for internal nodes, >0 for leaves

        bool is_leaf() const { return tri_count > 0; }
    };
    #pragma pack(pop)

}
