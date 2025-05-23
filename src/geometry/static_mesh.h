#pragma once

#include "engine/render/buffer.h"


namespace GLT::geometry {

    #pragma pack(push, 1)  // No padding between members
    struct vertex {
        glm::vec3 position;  // 12 bytes
        float uv_x;          // 4 bytes
        glm::vec3 normal;    // 12 bytes
        float uv_y;          // 4 bytes
    };
    #pragma pack(pop)

    
    #pragma pack(push, 1)  // No padding between members
    struct bounding_volume {
        glm::vec3   origin;
        u32         child_0;
        glm::vec3   extend;
        u32         child_1;
    };
    #pragma pack(pop)


    struct static_mesh {
        std::vector<vertex>         vertices{};
        std::vector<u32>            indices{};

        glm::mat4                   transform{1.0f};        // TODO: move to transform_component as soon as ECS is integrated
        glm::vec3                   center{0.f};
        f32                         radius = 0.f;           // a sphere around the model_center
        glm::vec3                   size;                   // use same center as sphere bounds

        GLT::render::buffer         vertex_buffer{GLT::render::buffer::type::VERTEX, GLT::render::buffer::usage::STATIC};
        GLT::render::buffer         index_buffer{GLT::render::buffer::type::INDEX, GLT::render::buffer::usage::STATIC};
        u32                         vao = 0;
        u32                         vertex_ssbo = 0;
        u32                         index_ssbo = 0;

        void compute_bounds() {
           
            if (vertices.empty())
                return;
            
            glm::vec3 min = vertices[0].position;
            glm::vec3 max = vertices[0].position;
            for (const auto& v : vertices) {
                min = glm::min(min, v.position);
                max = glm::max(max, v.position);
            }
            
            center = (min + max) * 0.5f;
            radius = 0.f;
            for (const auto& v : vertices) {
                float dist = glm::length(v.position - center);
                radius = glm::max(radius, dist);
            }
        }
    };

}
