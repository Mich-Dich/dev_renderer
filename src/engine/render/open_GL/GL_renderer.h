#pragma once

#include "engine/render/renderer.h"


namespace GLT::render::open_GL {

    typedef unsigned int	GLuint;

    class GL_renderer : public GLT::render::renderer{
    public:
        GL_renderer(ref<window> window);
        ~GL_renderer();
    
        void draw_frame(float delta_time);
        void set_size(const u32 width, const u32 height);

        // -------- ImGui --------
        void imgui_init();
        void imgui_shutdown();
        void imgui_create_fonts();
    
    private:
        GLuint          m_shader_program;
        GLuint          m_vao;
        GLuint          m_vbo;
        glm::vec2       mouse_pos{};
        
        void create_shader_program();
        void create_fullscreen_quad();
    };

}
