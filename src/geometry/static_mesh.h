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

    struct static_mesh {
        std::vector<vertex>         vertices{};
        std::vector<u32>            indices{};
        glm::mat4                   transform{1.0f};

        GLT::render::buffer         vertex_buffer{GLT::render::buffer::type::VERTEX, GLT::render::buffer::usage::STATIC};
        GLT::render::buffer         index_buffer{GLT::render::buffer::type::INDEX, GLT::render::buffer::usage::STATIC};
        u32 vao = 0;

        u32 vertex_ssbo = 0;
        u32 index_ssbo = 0;

        glm::vec3 model_center{0.f};
        f32 model_radius = 0.f;
        glm::vec3 world_center{0.f};
        f32 world_radius = 0.f;

        void compute_bounds() {
           
            if (vertices.empty())
                return;
            
            glm::vec3 min = vertices[0].position;
            glm::vec3 max = vertices[0].position;
            for (const auto& v : vertices) {
                min = glm::min(min, v.position);
                max = glm::max(max, v.position);
            }
            
            model_center = (min + max) * 0.5f;
            model_radius = 0.f;
            for (const auto& v : vertices) {
                float dist = glm::length(v.position - model_center);
                model_radius = glm::max(model_radius, dist);
            }
        }

        void update_world_bounds(const glm::mat4& transform) {

            world_center = glm::vec3(transform * glm::vec4(model_center, 1.0f));
            glm::vec3 scale(
                glm::length(transform[0]),
                glm::length(transform[1]),
                glm::length(transform[2])
            );
            float max_scale = glm::max(scale.x, glm::max(scale.y, scale.z));
            world_radius = model_radius * max_scale;
        }
    };

}