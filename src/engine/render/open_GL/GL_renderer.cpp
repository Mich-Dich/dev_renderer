#include "util/pch.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "util/util.h"

#include "GL_renderer.h"


namespace GLT::render::open_GL {

    GL_renderer::GL_renderer(ref<window> window) 
        : renderer(window) {
        
        GLFWwindow* glfwWindow = m_window->get_window();
        glfwMakeContextCurrent(glfwWindow);
        
        glewExperimental = GL_TRUE;
        GLenum err = glewInit();
        ASSERT(err == GLEW_OK, "", "Failed to initialize GLEW: " << glewGetErrorString(err))
        ASSERT(GLEW_VERSION_2_0, "", "OpenGL 2.0 not supported!")
    
        LOG(Trace, "OpenGL Version: " << glGetString(GL_VERSION));
        LOG(Trace, "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION));
        LOG(Trace, "Vendor: " << glGetString(GL_VENDOR));
        LOG(Trace, "Renderer: " << glGetString(GL_RENDERER));
    
        create_shader_program();
        create_fullscreen_quad();
        
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
    }
    
    GL_renderer::~GL_renderer() {
        
    }
    

    
    void GL_renderer::draw_frame(float delta_time) {
    
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
        glUseProgram(m_shader_program);
    
        // Set uniforms
        GLint loc_resolution = glGetUniformLocation(m_shader_program, "u_resolution");
        GLint loc_mouse = glGetUniformLocation(m_shader_program, "u_mouse");
        GLint loc_time = glGetUniformLocation(m_shader_program, "u_time");
    
        const int width = m_window->get_width();
        const int height = m_window->get_height();
        glUniform2f(loc_resolution, (float)width, (float)height);
    
        m_window->get_mouse_position(mouse_pos);
        glUniform2f(loc_mouse, mouse_pos.x, (height - mouse_pos.y)); // Flip Y
    
        static float totalTime = 0.0f;
        totalTime += delta_time;
        glUniform1f(loc_time, totalTime);
    
        // Draw fullscreen quad
        glBindVertexArray(m_vao);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glBindVertexArray(0);
    
        // Swap buffers
        glfwSwapBuffers(m_window->get_window());
    }
    
    void GL_renderer::set_size(const u32 width, const u32 height) { glViewport(0, 0, width, height); }
    
    
    void GL_renderer::imgui_init() {

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui_ImplGlfw_InitForOpenGL(m_window->get_window(), true);
        ImGui_ImplOpenGL3_Init("#version 330");
        ImGui::StyleColorsDark();
    }

    void GL_renderer::imgui_shutdown() {

        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        // ImGui::DestroyContext();
    }

    void GL_renderer::imgui_create_fonts() {
        
        ImGui_ImplOpenGL3_CreateFontsTexture();
    }



    
    GLuint compile_shader(GLenum type, const char* source) {
        GLuint shader = glCreateShader(type);
        glShaderSource(shader, 1, &source, nullptr);
        glCompileShader(shader);
        
        GLint success;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if(!success) {
            char infoLog[512];
            glGetShaderInfoLog(shader, 512, nullptr, infoLog);
            LOG(Error, "Shader compilation failed: " << infoLog);
        }
        return shader;
    }
    
    void GL_renderer::create_shader_program() {
    
        std::string vert_content = io::read_file("shaders/fullscreen_quad.vert");
        ASSERT(!vert_content.empty(), "", "Failed to read vert shader");
    
        std::string frag_content = io::read_file("shaders/07_polygon.frag");
        ASSERT(!frag_content.empty(), "", "Failed to read frag shader");
    
        GLuint vertexShader = compile_shader(GL_VERTEX_SHADER, vert_content.data());
        GLuint fragmentShader = compile_shader(GL_FRAGMENT_SHADER, frag_content.data());
    
        m_shader_program = glCreateProgram();
        glAttachShader(m_shader_program, vertexShader);
        glAttachShader(m_shader_program, fragmentShader);
        glLinkProgram(m_shader_program);
    
        GLint success;
        glGetProgramiv(m_shader_program, GL_LINK_STATUS, &success);
        if(!success) {
            char infoLog[512];
            glGetProgramInfoLog(m_shader_program, 512, nullptr, infoLog);
            LOG(Error, "Shader program linking failed: " << infoLog);
        }
    
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
    }
    
    void GL_renderer::create_fullscreen_quad() {
        
        float vertices[] = {
            // positions   // texCoords
            -1.0,  1.0,    0.0, 1.0,
            -1.0, -1.0,    0.0, 0.0,
             1.0,  1.0,    1.0, 1.0,
             1.0, -1.0,    1.0, 0.0
        };
    
        glGenVertexArrays(1, &m_vao);
        glGenBuffers(1, &m_vbo);
    
        glBindVertexArray(m_vao);
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
        // Position attribute
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
    
        // Texture coordinate attribute
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
        glEnableVertexAttribArray(1);
    
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

}
