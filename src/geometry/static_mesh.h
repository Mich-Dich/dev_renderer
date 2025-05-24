#pragma once

#include "engine/render/buffer.h"
#include "BVH.h"


namespace GLT::geometry {

    #pragma pack(push, 1)  // No padding between members
    struct vertex {
        glm::vec3 position;  // 12 bytes
        float uv_x;          // 4 bytes
        glm::vec3 normal;    // 12 bytes
        float uv_y;          // 4 bytes
    };
    #pragma pack(pop)

    struct static_mesh {
        std::vector<vertex>         vertices{};
        std::vector<u32>            indices{};

        glm::mat4                   transform{1.0f};        // TODO: move to transform_component as soon as ECS is integrated

        std::vector<BVH_node>       BVH_nodes;
        std::vector<u32>            triIdx;

        GLT::render::buffer         vertex_buffer{GLT::render::buffer::type::VERTEX, GLT::render::buffer::usage::STATIC};
        GLT::render::buffer         index_buffer{GLT::render::buffer::type::INDEX, GLT::render::buffer::usage::STATIC};
        u32                         vao = 0;
        u32                         vertex_ssbo = 0;
        u32                         index_ssbo = 0;
        u32                         bvh_ssbo = 0;
        u32                         triidx_ssbo = 0;
    
#ifdef DEBUG
        // BVH Visualization parameters
        int bvh_viz_max_depth = 3;
        bool bvh_show_leaves = true;
        glm::vec4 bvh_viz_color = glm::vec4(1.0f, 0.0f, 0.0f, 1.f); // Red by default

        u32 max_triangles_count = 0;
        u32 average_triangles_count = 0;
        int bvh_max_depth = 0;
        int bvh_leaf_count = 0;
        f32 BVH_build_time = 0.f;

        void compute_bvh_stats();
#endif

        void build_BVH();

    private:
        void update_node_bounds(u32 nodeIdx);
        void subdivide(u32 nodeIdx, const std::vector<glm::vec3>& centroids);
    };

}
