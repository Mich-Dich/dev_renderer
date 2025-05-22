#include "util/pch.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "util/util.h"
#include "application.h"
#include "layer/layer.h"
#include "layer/layer_stack.h"
#include "layer/imgui_layer.h"
#include "layer/world_layer.h"
#include "world/static_mesh.h"
#include "engine/platform/window.h"

#include "GL_renderer.h"

#ifdef DEBUG
    #define END_TIMER               glEndQuery(GL_TIME_ELAPSED);
    #define START_TIMER(variable)   glBeginQuery(GL_TIME_ELAPSED, variable);
#else
    #define END_TIMER
    #define START_TIMER(variable)
#endif


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

        m_gbuffer.create(window->get_width(), window->get_height());
        create_geometry_shader();

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);

#ifdef DEBUG
        GLuint queries[3];
        glGenQueries(3, queries);
        m_query_total     = queries[0];
        m_query_geometry  = queries[1];
        m_query_lighting  = queries[2];
#endif

    }
    
    GL_renderer::~GL_renderer() {
        
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
    
    
    void GL_renderer::draw_frame(f32 delta_time) {
    
#ifdef DEBUG
		m_general_performance_metrik.next_iteration();
        START_TIMER(m_query_total);
#endif

        // ----------------------------------------------------------
        // Geometry Pass: Render scene to G-buffer
        // ----------------------------------------------------------
        START_TIMER(m_query_geometry);

        glBindFramebuffer(GL_FRAMEBUFFER, m_gbuffer.FBO);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(m_geometry_shader);

        // Update matrices
        m_viewMatrix = glm::lookAt(m_cameraPos, m_cameraPos + m_cameraFront, m_cameraUp);
        glUniformMatrix4fv(m_geometry_shader_view_loc, 1, GL_FALSE, glm::value_ptr(m_viewMatrix));

        // Draw mesh
        ref<GLT::mesh::static_mesh> mesh = application::get().get_world_layer()->GET_RENDER_MESH();
        glBindVertexArray(mesh->vao);
        glUniformMatrix4fv(m_geometry_shader_model_loc, 1, GL_FALSE, glm::value_ptr(mesh->transform));
        glDrawElements(GL_TRIANGLES, mesh->indices.size(), GL_UNSIGNED_INT, 0);

        END_TIMER; // ends m_query_geometry

        // ----------------------------------------------------------
        // Lighting Pass: Render fullscreen quad with lighting calculation
        // ----------------------------------------------------------
        START_TIMER(m_query_lighting);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        glUseProgram(m_lighting_shader);

        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, mesh->vertex_ssbo);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, mesh->index_ssbo);
        glUniform1ui(glGetUniformLocation(m_lighting_shader, "numIndices"), mesh->indices.size());
        LOG(Trace, "Binding mesh indecies: " << mesh->indices.size() << " vertex SSBO: " << mesh->vertex_ssbo << " index SSBO: " << mesh->index_ssbo)

        // Camera setup
        glm::mat4 projection = glm::perspective(
            glm::radians(45.0f),
            static_cast<f32>(m_window->get_width()) / m_window->get_height(),
            0.01f, 1000.0f
        );
        glm::mat4 view = glm::lookAt(m_cameraPos, m_cameraPos + m_cameraFront, m_cameraUp);
        glm::mat4 invProjection = glm::inverse(projection);
        glm::mat4 invView = glm::inverse(view);

        glUniform3fv(glGetUniformLocation(m_lighting_shader, "cameraPos"), 1, glm::value_ptr(m_cameraPos));
        glUniformMatrix4fv(glGetUniformLocation(m_lighting_shader, "invProjection"), 1, GL_FALSE, glm::value_ptr(invProjection));
        glUniformMatrix4fv(glGetUniformLocation(m_lighting_shader, "invView"), 1, GL_FALSE, glm::value_ptr(invView));
        glUniformMatrix4fv(glGetUniformLocation(m_lighting_shader, "model"), 1, GL_FALSE, glm::value_ptr(mesh->transform));
        glUniform2fv(glGetUniformLocation(m_lighting_shader, "screenSize"), 1, glm::value_ptr(glm::vec2(m_window->get_width(), m_window->get_height())));

        // Draw fullscreen quad
        glBindVertexArray(m_vao);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glBindVertexArray(0);

        END_TIMER; // ends m_query_lighting

        // ----------------------------------------------------------
        // Render UI
        // ----------------------------------------------------------
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        for (layer* layer : *renderer::m_layer_stack) 
            layer->on_imgui_render();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Swap buffers
        glfwSwapBuffers(m_window->get_window());        // TODO: move to window and make renderer a friend
        glfwSwapBuffers(m_window->get_window());

#ifdef DEBUG
        END_TIMER; // ends m_query_total

        // Wait for all three results to become available
        GLuint available = 0;
        while (!available) {
            glGetQueryObjectiv(m_query_total, GL_QUERY_RESULT_AVAILABLE, (GLint*)&available);
        }

        GLuint64 time_total, time_geom, time_light;
        glGetQueryObjectui64v(m_query_total,    GL_QUERY_RESULT, &time_total);
        glGetQueryObjectui64v(m_query_geometry, GL_QUERY_RESULT, &time_geom);
        glGetQueryObjectui64v(m_query_lighting, GL_QUERY_RESULT, &time_light);

        // Convert from nanoseconds to milliseconds:
        m_general_performance_metrik.renderer_draw_time[m_general_performance_metrik.current_index]     = time_total  / 1e6;
        m_general_performance_metrik.geometry_pass_time[m_general_performance_metrik.current_index]     = time_geom   / 1e6;
        m_general_performance_metrik.lighting_pass_time[m_general_performance_metrik.current_index]     = time_light  / 1e6;
#endif
    }
    

    void GL_renderer::set_size(const u32 width, const u32 height) {

        m_gbuffer.resize(width, height);
        glViewport(0, 0, width, height);
        
        // Update projection matrix
        glUseProgram(m_geometry_shader);
        glm::mat4 projection = glm::perspective(
            glm::radians(45.0f),
            static_cast<f32>(width) / height,
            0.1f, 100.0f
        );
        glUniformMatrix4fv(m_geometry_shader_proj_loc, 1, GL_FALSE, glm::value_ptr(projection));
        glUseProgram(0);
    }


    void GL_renderer::reload_fragment_shader(const std::filesystem::path& frag_file) {

        std::string frag_src = io::read_file(frag_file.string());
        VALIDATE(!frag_src.empty(), return, "", "Failed to read frag shader: " << frag_file)

        // --- Recreate shader program from scratch to avoid duplicate definitions ---
        // Read vertex shader again (to use same entry point)
        std::string vert_src = io::read_file("shaders/fullscreen_quad.vert");
        VALIDATE(!vert_src.empty(), return, "", "Failed to re-read vert shader")

        GLuint vertShader = compile_shader(GL_VERTEX_SHADER, vert_src.c_str());
        GLuint fragShader = compile_shader(GL_FRAGMENT_SHADER, frag_src.c_str());

        GLuint newProgram = glCreateProgram();
        glAttachShader(newProgram, vertShader);
        glAttachShader(newProgram, fragShader);
        glLinkProgram(newProgram);

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

        glDeleteProgram(m_lighting_shader);
        m_lighting_shader = newProgram;

        glDeleteShader(vertShader);
        glDeleteShader(fragShader);

        LOG(Info, "Shader program reloaded from " << frag_file);
        m_time_total = 0.f;
    }


    void GL_renderer::create_geometry_shader() {
        
        // Vertex shader source
    	const auto vert_source = io::read_file(GLT::util::get_executable_path().parent_path() / "shaders" / "geometry_pass.vert");
        GLuint vertShader = compile_shader(GL_VERTEX_SHADER, vert_source.data());

        const auto frag_source = io::read_file(GLT::util::get_executable_path().parent_path() / "shaders" / "geometry_pass.frag");
        GLuint fragShader = compile_shader(GL_FRAGMENT_SHADER, frag_source.data());

        // Create and link program
        m_geometry_shader = glCreateProgram();
        glAttachShader(m_geometry_shader, vertShader);
        glAttachShader(m_geometry_shader, fragShader);
        glLinkProgram(m_geometry_shader);

        // Check linking errors
        GLint success;
        glGetProgramiv(m_geometry_shader, GL_LINK_STATUS, &success);
        if (!success) {
            char infoLog[512];
            glGetProgramInfoLog(m_geometry_shader, 512, nullptr, infoLog);
            LOG(Error, "Geometry shader linking failed: " << infoLog);
        }

        // Cleanup shaders
        glDeleteShader(vertShader);
        glDeleteShader(fragShader);

        // Get uniform locations
        m_geometry_shader_model_loc = glGetUniformLocation(m_geometry_shader, "model");
        m_geometry_shader_view_loc = glGetUniformLocation(m_geometry_shader, "view");
        m_geometry_shader_proj_loc = glGetUniformLocation(m_geometry_shader, "projection");

        // Set static projection matrix (update on window resize)
        glUseProgram(m_geometry_shader);
        glm::mat4 projection = glm::perspective(
            glm::radians(45.0f), 
            static_cast<f32>(m_window->get_width()) / m_window->get_height(),
            0.1f, 100.0f
        );
        glUniformMatrix4fv(m_geometry_shader_proj_loc, 1, GL_FALSE, glm::value_ptr(projection));
        glUseProgram(0);
    }


    void GL_renderer::upload_mesh(ref<GLT::mesh::static_mesh> mesh) {

        // Create buffers if they don't exist
        if(mesh->vertex_buffer.get_ID() == 0)
            mesh->vertex_buffer.create(mesh->vertices.data(), mesh->vertices.size() * sizeof(GLT::mesh::vertex));
        
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
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GLT::mesh::vertex), (void*)offsetof(GLT::mesh::vertex, position));
        
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(GLT::mesh::vertex), (void*)offsetof(GLT::mesh::vertex, normal));
        
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(GLT::mesh::vertex), (void*)offsetof(GLT::mesh::vertex, texcoord));

        // Create SSBOs for ray tracing
        if (mesh->vertex_ssbo == 0) {
            glGenBuffers(1, &mesh->vertex_ssbo);
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, mesh->vertex_ssbo);
            glBufferData(GL_SHADER_STORAGE_BUFFER, mesh->vertices.size() * sizeof(GLT::mesh::vertex), mesh->vertices.data(), GL_STATIC_DRAW);
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
    
        std::string vert_source = io::read_file("shaders/fullscreen_quad.vert");
        ASSERT(!vert_source.empty(), "", "Failed to read vert shader");
    
        std::string frag_source = io::read_file("shaders/lighting_pass.frag");
        ASSERT(!frag_source.empty(), "", "Failed to read frag shader");
    
        GLuint vert_shader = compile_shader(GL_VERTEX_SHADER, vert_source.data());
        GLuint frag_shader = compile_shader(GL_FRAGMENT_SHADER, frag_source.data());
    
        m_lighting_shader = glCreateProgram();
        glAttachShader(m_lighting_shader, vert_shader);
        glAttachShader(m_lighting_shader, frag_shader);
        glLinkProgram(m_lighting_shader);
        
        glUseProgram(m_lighting_shader);
        glUniform1i(glGetUniformLocation(m_lighting_shader, "gPosition"), 0);
        glUniform1i(glGetUniformLocation(m_lighting_shader, "gNormal"), 1);
        glUniform1i(glGetUniformLocation(m_lighting_shader, "gAlbedoSpec"), 2);
        glUseProgram(0);
        
        GLint success;
        glGetProgramiv(m_lighting_shader, GL_LINK_STATUS, &success);
        if(!success) {
            char infoLog[512];
            glGetProgramInfoLog(m_lighting_shader, 512, nullptr, infoLog);
            LOG(Error, "Shader program linking failed: " << infoLog);
        }
    
        glDeleteShader(vert_shader);
        glDeleteShader(frag_shader);
    }
    

    void GL_renderer::create_fullscreen_quad() {
        
        f32 vertices[] = {
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
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(f32), (void*)0);
        glEnableVertexAttribArray(0);
    
        // Texture coordinate attribute
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(f32), (void*)(2 * sizeof(f32)));
        glEnableVertexAttribArray(1);
    
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

}
