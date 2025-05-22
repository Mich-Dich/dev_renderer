#pragma once

#include "engine/render/renderer.h"

namespace GLT { class layer_stack; }

namespace GLT::render::open_GL {

    typedef unsigned int	GLuint;
    typedef unsigned int    GLenum;

    class GL_renderer : public GLT::render::renderer{
    public:
        GL_renderer(ref<window> window, ref<layer_stack> layer_stack);
        ~GL_renderer();
    
        void draw_frame(float delta_time);
        void set_size(const u32 width, const u32 height);

        void upload_mesh(ref<GLT::mesh::static_mesh> mesh);
        void create_geometry_shader();

        // -------- ImGui --------
        void imgui_init();
        void imgui_shutdown();
        void imgui_create_fonts();

        void reload_fragment_shader(const std::filesystem::path& frag_file);

    private:
        GLuint          m_vao{};
        GLuint          m_vbo{};
        glm::vec2       mouse_pos{};
        f32             m_time_total = 0.f;
        
    #ifdef DEBUG
        GLuint  m_query_total;
        GLuint  m_query_geometry;
        GLuint  m_query_lighting;
    #endif

        void create_shader_program();
        void create_fullscreen_quad();
        GLuint compile_shader(GLenum type, const char* source);

        // TODO: create camera struct
        glm::mat4 m_viewMatrix;
        glm::vec3 m_cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
        glm::vec3 m_cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
        glm::vec3 m_cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
        
        // Uniform locations
        GLuint m_geometry_shader_model_loc;
        GLuint m_geometry_shader_view_loc;
        GLuint m_geometry_shader_proj_loc;
    };

}
