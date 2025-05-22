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
    };

}