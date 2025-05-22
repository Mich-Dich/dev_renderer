#include "util/pch.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <meshoptimizer.h>

#include "util/util.h"
#include "application.h"
#include "layer/layer.h"
#include "layer/layer_stack.h"
#include "layer/imgui_layer.h"
#include "layer/world_layer.h"
#include "geometry/static_mesh.h"
#include "engine/platform/window.h"

#include "GL_renderer.h"


namespace GLT::render::open_GL {

    GL_renderer::GL_renderer(ref<window> window, ref<layer_stack> layer_stack)
        : renderer(window, layer_stack) {
        
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

#ifdef DEBUG
        glGenQueries(1, &m_total_render_time);
#endif
    }
    
    GL_renderer::~GL_renderer() {
        
    }
    



    void GL_renderer::draw_frame(float delta_time) {
    
#ifdef DEBUG
		m_general_performance_metrik.next_iteration();
        glBeginQuery(GL_TIME_ELAPSED, m_total_render_time);
#endif

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
        ref<GLT::geometry::static_mesh> mesh = application::get().get_world_layer()->GET_RENDER_MESH();
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, mesh->vertex_ssbo);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, mesh->index_ssbo);

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
    
        // start new ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        for (layer* layer : *renderer::m_layer_stack) 
            layer->on_imgui_render();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Swap buffers
        glfwSwapBuffers(m_window->get_window());

#ifdef DEBUG
        glEndQuery(GL_TIME_ELAPSED);
        GLint available = 0;
        while (!available)
            glGetQueryObjectiv(m_total_render_time, GL_QUERY_RESULT_AVAILABLE, &available);

        GLuint64 elapsed_time;
        glGetQueryObjectui64v(m_total_render_time, GL_QUERY_RESULT, &elapsed_time);
        m_general_performance_metrik.renderer_draw_time[m_general_performance_metrik.current_index] = elapsed_time / 1e6f;
#endif
    }
    

    GLuint GL_renderer::compile_shader(GLenum type, const char* source) {

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
        

    void GL_renderer::set_size(const u32 width, const u32 height) { glViewport(0, 0, width, height); }
    

    void GL_renderer::reload_fragment_shader(const std::filesystem::path& frag_file) {

        // Read new fragment shader source
        std::string frag_src = io::read_file(frag_file.string());
        if (frag_src.empty()) {
            LOG(Error, "Failed to read frag shader: " << frag_file);
            return;
        }

        // --- Recreate shader program from scratch to avoid duplicate definitions ---
        // Read vertex shader again (to use same entry point)
        std::string vert_src = io::read_file("shaders/fullscreen_quad.vert");
        if (vert_src.empty()) {
            LOG(Error, "Failed to re-read vert shader");
            return;
        }

        GLuint vertShader = compile_shader(GL_VERTEX_SHADER, vert_src.c_str());
        GLuint fragShader = compile_shader(GL_FRAGMENT_SHADER, frag_src.c_str());

        GLuint newProgram = glCreateProgram();
        glAttachShader(newProgram, vertShader);
        glAttachShader(newProgram, fragShader);
        glLinkProgram(newProgram);

        // Check link status
        GLint success;
        glGetProgramiv(newProgram, GL_LINK_STATUS, &success);
        if (!success) {
            char infoLog[512];
            glGetProgramInfoLog(newProgram, 512, nullptr, infoLog);
            LOG(Error, "Shader relink failed: " << infoLog);
            // Cleanup
            glDeleteShader(vertShader);
            glDeleteShader(fragShader);
            glDeleteProgram(newProgram);
            return;
        }

        // If successful, swap programs
        glDeleteProgram(m_shader_program);
        m_shader_program = newProgram;

        // Cleanup shaders (they can be deleted after linking)
        glDeleteShader(vertShader);
        glDeleteShader(fragShader);

        LOG(Info, "Shader program reloaded from " << frag_file);
    }


    void GL_renderer::prepare_mesh(ref<GLT::geometry::static_mesh> mesh) {

        // 1. Generate vertex remap
        std::vector<unsigned int> remap(mesh->vertices.size());
        size_t vertex_count = meshopt_generateVertexRemap(
            remap.data(),
            mesh->indices.data(),
            mesh->indices.size(),
            mesh->vertices.data(),
            mesh->vertices.size(),
            sizeof(GLT::geometry::vertex)
        );

        // 2. Allocate remapped data
        std::vector<GLT::geometry::vertex> vertices(vertex_count);
        std::vector<unsigned int> indices(mesh->indices.size());
        
        // 3. Remap indices and vertices
        meshopt_remapIndexBuffer(indices.data(), mesh->indices.data(), mesh->indices.size(), remap.data());
        meshopt_remapVertexBuffer(
            vertices.data(),
            mesh->vertices.data(),
            mesh->vertices.size(),
            sizeof(GLT::geometry::vertex),
            remap.data()
        );

        // 4. Optimize vertex cache
        meshopt_optimizeVertexCache(indices.data(), indices.data(), indices.size(), vertices.size());

        // 5. Optimize overdraw
        meshopt_optimizeOverdraw(
            indices.data(), 
            indices.data(), 
            indices.size(),
            &vertices[0].position.x,
            vertices.size(),
            sizeof(GLT::geometry::vertex),
            1.05f
        );

        // 6. Optimize vertex fetch
        meshopt_optimizeVertexFetch(
            vertices.data(),
            indices.data(),
            indices.size(),
            vertices.data(),
            vertices.size(),
            sizeof(GLT::geometry::vertex)
        );

        // Replace with optimized data
        mesh->vertices = std::move(vertices);
        mesh->indices = std::move(indices);
    }


    void GL_renderer::upload_mesh(ref<GLT::geometry::static_mesh> mesh) {

        prepare_mesh(mesh);

        mesh->vertex_buffer.create(mesh->vertices.data(), mesh->vertices.size() * sizeof(GLT::geometry::vertex));
        mesh->index_buffer.create(mesh->indices.data(), mesh->indices.size() * sizeof(u32));

        // Create buffers if they don't exist
        if(mesh->vertex_buffer.get_ID() == 0)
            mesh->vertex_buffer.create(mesh->vertices.data(), mesh->vertices.size() * sizeof(GLT::geometry::vertex));
        
        if(mesh->index_buffer.get_ID() == 0)
            mesh->index_buffer.create(mesh->indices.data(), mesh->indices.size() * sizeof(u32));

        // Create VAO
        if(mesh->vao == 0)
            glGenVertexArrays(1, &mesh->vao);
        
        glBindVertexArray(mesh->vao);
        
        // Bind buffers
        mesh->vertex_buffer.bind();
        mesh->index_buffer.bind();

        // Vertex attributes
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GLT::geometry::vertex), (void*)offsetof(GLT::geometry::vertex, position));
        
        // Normal attribute - now at offset 16 (position(12) + uv_x(4))
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(GLT::geometry::vertex), (void*)offsetof(GLT::geometry::vertex, normal));
        
        // UV attributes - split into two separate floats
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(GLT::geometry::vertex), (void*)offsetof(GLT::geometry::vertex, uv_x));
        
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(GLT::geometry::vertex), (void*)offsetof(GLT::geometry::vertex, uv_y));

        // Create SSBOs for ray tracing
        if (mesh->vertex_ssbo == 0) {
            glGenBuffers(1, &mesh->vertex_ssbo);
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, mesh->vertex_ssbo);
            glBufferData(GL_SHADER_STORAGE_BUFFER, mesh->vertices.size() * sizeof(GLT::geometry::vertex), mesh->vertices.data(), GL_STATIC_DRAW);
        }

        if (mesh->index_ssbo == 0) {
            glGenBuffers(1, &mesh->index_ssbo);
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, mesh->index_ssbo);
            glBufferData(GL_SHADER_STORAGE_BUFFER, mesh->indices.size() * sizeof(u32), mesh->indices.data(), GL_STATIC_DRAW);
        }

        glBindVertexArray(0);
    }


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


    void GL_renderer::create_shader_program() {
    
        std::string vert_content = io::read_file("shaders/fullscreen_quad.vert");
        ASSERT(!vert_content.empty(), "", "Failed to read vert shader");
    
        std::string frag_content = io::read_file("shaders/ray_tracer_intor.frag");
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
