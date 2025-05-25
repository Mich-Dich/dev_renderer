#pragma once

#include "engine/render/renderer.h"

namespace GLT {

    class layer_stack;
    class file_watcher_system;
}

namespace GLT::render::open_GL {

    typedef unsigned int	GLuint;
    typedef unsigned int    GLenum;

    class GL_renderer : public GLT::render::renderer{
    public:
        GL_renderer(ref<window> window, ref<layer_stack> layer_stack);
        ~GL_renderer();
    
        void draw_frame(float delta_time);
        void set_size(const u32 width, const u32 height);

        void upload_static_mesh(ref<GLT::geometry::static_mesh> mesh) override;
        void remove_static_mesh(ref<GLT::geometry::static_mesh> mesh) override;
        bool reload_fragment_shader(const std::filesystem::path& frag_file, std::string& output) override;

        // -------- ImGui --------
        void imgui_init();
        void imgui_shutdown();
        void imgui_create_fonts();

    private:
        GLuint                              m_shader_program;
        GLuint                              m_vao;
        GLuint                              m_vbo;
        glm::vec2                           mouse_pos{};
        GLuint                              m_total_render_time{};
        
        void create_shader_program();
        void create_fullscreen_quad();
        bool compile_shader(GLuint& shader_handle, GLenum type, const char* source, std::string& output);
        
        void init_file_watcher();
        void auto_reload_file(const std::filesystem::path& file);
    };

}
