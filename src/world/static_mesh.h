#pragma once

#include "engine/render/buffer.h"


namespace GLT::mesh {

    struct vertex {
        float position[3];
        float normal[3];
        float texcoord[2];
    };

    struct static_mesh {
        std::vector<vertex>         vertices{};
        std::vector<u32>            indices{};
        glm::mat4                   transform{1.0f};

        GLT::render::buffer         vertex_buffer{GLT::render::buffer::type::VERTEX, GLT::render::buffer::usage::STATIC};
        GLT::render::buffer         index_buffer{GLT::render::buffer::type::INDEX, GLT::render::buffer::usage::STATIC};
        u32 vao = 0;

        void create_buffers() {

            vertex_buffer.create(vertices.data(), vertices.size() * sizeof(GLT::mesh::vertex));
            index_buffer.create(indices.data(), indices.size() * sizeof(u32));
        }
    };

}
