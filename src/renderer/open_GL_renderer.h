#pragma once

#include "platform/window.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>

class openGL_renderer {
public:
    openGL_renderer(ref<window> window);
    ~openGL_renderer();

    void init();
    void shutdown();
    void draw_frame(float delta_time);
    void set_size(const u32 width, const u32 height);

private:
    GLuint          m_shader_program;
    GLuint          m_vao;
    GLuint          m_vbo;
    ref<window>     m_window;
    glm::vec2       mouse_pos{};
    
    void create_shader_program();
    void create_fullscreen_quad();
};
