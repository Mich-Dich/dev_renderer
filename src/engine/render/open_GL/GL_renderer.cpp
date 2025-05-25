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
#include "geometry/static_mesh.h"
#include "engine/platform/window.h"
#include "game_object/camera.h"
#include "project/file_watcher_system.h"

#include "GL_renderer.h"


namespace GLT::render::open_GL {


    static GLT::file_watcher_system		    m_file_watcher{};

	
    void GL_renderer::auto_reload_file(const std::filesystem::path& file) {

        LOG(Trace, "File changed [" << file.generic_string() << "]");
        std::string output{};
        reload_fragment_shader(file, output);
	}
    
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

        LOG(Trace, "Monitoring directory " << util::get_executable_path().parent_path() / "shaders");
        m_file_watcher.path = util::get_executable_path().parent_path() / "shaders";
		m_file_watcher.p_notify_filters = notify_filters::last_access | notify_filters::last_write | notify_filters::file_name | notify_filters::directory_name;
		m_file_watcher.filter = "*.frag";
		m_file_watcher.include_sub_directories = true;
        m_file_watcher.on_changed = [this](const std::filesystem::path& file) { this->auto_reload_file(file); };
        m_file_watcher.on_created = [this](const std::filesystem::path& file) { this->auto_reload_file(file); };
        m_file_watcher.on_renamed = [this](const std::filesystem::path& file) { this->auto_reload_file(file); };
		m_file_watcher.compile = nullptr;
		m_file_watcher.start();

        m_active_camera = application::get().get_world_layer()->get_editor_camera();        // Force get editor camera for now

#ifdef DEBUG
        glGenQueries(1, &m_total_render_time);
#endif
    }
    
    GL_renderer::~GL_renderer() {
        
		m_file_watcher.stop();
    }
    



    void GL_renderer::draw_frame(float delta_time) {
    
#ifdef DEBUG
		m_general_performance_metrik.next_iteration();
        glBeginQuery(GL_TIME_ELAPSED, m_total_render_time);
#endif

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    	
        // view = m_active_camera->get_view();
		// proj = glm::perspective(glm::radians(m_active_camera->get_perspective_fov_y()), (float)m_draw_extent.width / (float)m_draw_extent.height, m_active_camera->get_clipping_far(), m_active_camera->get_clipping_near());

        glUseProgram(m_shader_program);
    
        // ------ bind mesh ------
        ref<GLT::geometry::static_mesh> mesh = application::get().get_world_layer()->GET_RENDER_MESH();
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, mesh->vertex_ssbo);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, mesh->index_ssbo);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, mesh->bvh_ssbo);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, mesh->triidx_ssbo);
        
        // Add BVH-related uniforms
        GLint loc_bvh_root = glGetUniformLocation(m_shader_program, "u_bvh_root");
        GLint loc_bvh_nodes = glGetUniformLocation(m_shader_program, "u_bvh_nodes");
        glUniform1i(loc_bvh_root, 0);  // Root node index
        glUniform1i(loc_bvh_nodes, 2); // SSBO binding point
        mesh->vertex_buffer.bind();
        mesh->index_buffer.bind();

        // ------ BVH debug uniforms ------
        glUniform1i(glGetUniformLocation(m_shader_program, "u_bvh_viz_bounds_depth"), mesh->bvh_viz_max_depth);
        glUniform1i(glGetUniformLocation(m_shader_program, "u_bvh_viz_triangle_depth"), mesh->bvh_show_leaves);
        glUniform4fv(glGetUniformLocation(m_shader_program, "u_bvh_viz_color"), 1, glm::value_ptr(mesh->bvh_viz_color));

        // ------ Camera uniforms ------
        const glm::mat4 inv_proj = m_active_camera->get_inverse_projection((f32)m_window->get_width() / (f32)m_window->get_height());
        const glm::mat4 inv_view = m_active_camera->get_inverse_view();
        const glm::vec3 cam_pos = m_active_camera->get_position();

        glUniformMatrix4fv(glGetUniformLocation(m_shader_program, "u_inv_proj"), 1, GL_FALSE, glm::value_ptr(inv_proj));
        glUniformMatrix4fv(glGetUniformLocation(m_shader_program, "u_inv_view"), 1, GL_FALSE, glm::value_ptr(inv_view));
        glUniform3f(glGetUniformLocation(m_shader_program, "u_cam_pos"), cam_pos.x, cam_pos.y, cam_pos.z);

        // ------ Set general uniforms ------
        const int height = m_window->get_height();
        glUniform2f(glGetUniformLocation(m_shader_program, "u_resolution"), (float)m_window->get_width(), (float)height);
    
        m_window->get_mouse_position(mouse_pos);
        glUniform2f(glGetUniformLocation(m_shader_program, "u_mouse"), mouse_pos.x, (height - mouse_pos.y)); // Flip Y
    
        static float totalTime = 0.0f;
        totalTime += delta_time;
        glUniform1f(glGetUniformLocation(m_shader_program, "u_time"), totalTime);

        // ------ Draw fullscreen quad ------
        glBindVertexArray(m_vao);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glBindVertexArray(0);
    
#ifdef DEBUG        // set performance stuff
        glEndQuery(GL_TIME_ELAPSED);
        GLint available = 0;
        while (!available)
            glGetQueryObjectiv(m_total_render_time, GL_QUERY_RESULT_AVAILABLE, &available);

        GLuint64 elapsed_time;
        glGetQueryObjectui64v(m_total_render_time, GL_QUERY_RESULT, &elapsed_time);
        m_general_performance_metrik.renderer_draw_time[m_general_performance_metrik.current_index] = elapsed_time / 1e6f;
        m_general_performance_metrik.meshes = 1;
        m_general_performance_metrik.vertices = mesh->vertices.size();
#endif

        // ------ start new ImGui frame ------
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        for (layer* layer : *renderer::m_layer_stack) 
            layer->on_imgui_render();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Swap buffers
        glfwSwapBuffers(m_window->get_window());
    }
    

    bool GL_renderer::compile_shader(GLuint& shader_handle, GLenum type, const char* source, std::string& output) {

        shader_handle = glCreateShader(type);
        glShaderSource(shader_handle, 1, &source, nullptr);
        glCompileShader(shader_handle);
        
        GLint success;
        glGetShaderiv(shader_handle, GL_COMPILE_STATUS, &success);
        if(!success) {
            char compiler_log[1024];
            glGetShaderInfoLog(shader_handle, 1024, nullptr, compiler_log);
            LOG(Error, "Shader compilation failed: " << compiler_log);
            output = compiler_log;
            return false;
        }
        return true;
    }
        

    void GL_renderer::set_size(const u32 width, const u32 height) { glViewport(0, 0, width, height); }
    

    bool GL_renderer::reload_fragment_shader(const std::filesystem::path& frag_file, std::string& output) {

        // Read new fragment shader source
        std::string frag_src = io::read_file(frag_file.string());
        VALIDATE(!frag_src.empty(), output = "Could not find the file [" + frag_file.generic_string() + "]"; return false, "", "Failed to read frag shader: " << frag_file.generic_string() );

        // Read vertex shader again (to use same entry point)
        std::string vert_src = io::read_file("shaders/fullscreen_quad.vert");
        VALIDATE(!vert_src.empty(), output = "Could not read the file [" + frag_file.generic_string() + "]"; return false, "", "Failed to re-read vert shader");

        GLuint vertShader{}, fragShader{};
        VALIDATE(compile_shader(vertShader, GL_VERTEX_SHADER, vert_src.c_str(), output), return false, "", "Failed to compile vertex shader: " << output);
        VALIDATE(compile_shader(fragShader, GL_FRAGMENT_SHADER, frag_src.c_str(), output), return false, "", "Failed to compile fragment shader: " << output);

        GLuint newProgram = glCreateProgram();
        glAttachShader(newProgram, vertShader);
        glAttachShader(newProgram, fragShader);
        glLinkProgram(newProgram);

        // Check link status
        GLint success;
        glGetProgramiv(newProgram, GL_LINK_STATUS, &success);
        if (!success) {
            char compiler_log[1024];
            glGetProgramInfoLog(newProgram, 1024, nullptr, compiler_log);
            LOG(Error, "Shader relink failed: " << compiler_log);
            // Cleanup
            glDeleteShader(vertShader);
            glDeleteShader(fragShader);
            glDeleteProgram(newProgram);
            output = compiler_log;
            return false;
        }

        // If successful, swap programs
        glDeleteProgram(m_shader_program);
        m_shader_program = newProgram;

        // Cleanup shaders (they can be deleted after linking)
        glDeleteShader(vertShader);
        glDeleteShader(fragShader);

        LOG(Info, "Shader program reloaded from " << frag_file);
        return true;
    }


    void GL_renderer::upload_static_mesh(ref<GLT::geometry::static_mesh> mesh) {

        f32 VBH_generation_time = 0.f;
        util::stopwatch VBH_generation_time_stopwatch = util::stopwatch(&VBH_generation_time, duration_precision::microseconds);
        mesh->build_BVH(16);
        VBH_generation_time_stopwatch.stop();
        LOG(Debug, "BVH_generation_time [" << VBH_generation_time << "]")
        
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

        // Add BVH buffers
        if (mesh->bvh_ssbo == 0) {
            glGenBuffers(1, &mesh->bvh_ssbo);
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, mesh->bvh_ssbo);
            glBufferData(GL_SHADER_STORAGE_BUFFER, mesh->BVH_nodes.size() * sizeof(GLT::geometry::BVH_node), mesh->BVH_nodes.data(), GL_STATIC_DRAW);
        }

        if (mesh->triidx_ssbo == 0) {
            glGenBuffers(1, &mesh->triidx_ssbo);
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, mesh->triidx_ssbo);
            glBufferData(GL_SHADER_STORAGE_BUFFER, mesh->triIdx.size() * sizeof(u32), mesh->triIdx.data(), GL_STATIC_DRAW);
        }

        glBindVertexArray(0);
    }


    void GL_renderer::remove_static_mesh(ref<GLT::geometry::static_mesh> mesh) {
        
        // Delete Vertex Array Object
        if (mesh->vao != 0) {
            glDeleteVertexArrays(1, &mesh->vao);
            mesh->vao = 0;
        }

        // Delete Vertex Buffer
        if (mesh->vertex_buffer.get_ID() != 0) {
            GLuint vertex_id = mesh->vertex_buffer.get_ID();
            glDeleteBuffers(1, &vertex_id);
            mesh->vertex_buffer.set_ID(0);
        }

        // Delete Index Buffer
        if (mesh->index_buffer.get_ID() != 0) {
            GLuint index_id = mesh->index_buffer.get_ID();
            glDeleteBuffers(1, &index_id);
            mesh->index_buffer.set_ID(0);
        }

#define DELETE_SSBO(var)        if (var != 0) { glDeleteBuffers(1, &var); var = 0; }

        DELETE_SSBO(mesh->vertex_ssbo)
        DELETE_SSBO(mesh->index_ssbo)
        DELETE_SSBO(mesh->bvh_ssbo)
        DELETE_SSBO(mesh->triidx_ssbo)

#undef DELETE_SSBO

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
    
        std::string shader_comp_output{};
        GLuint vertexShader{}, fragmentShader{};
        VALIDATE(compile_shader(vertexShader, GL_VERTEX_SHADER, vert_content.data(), shader_comp_output), return, "", "Failed to compile shader: " << shader_comp_output);
        VALIDATE(compile_shader(fragmentShader, GL_FRAGMENT_SHADER, frag_content.data(), shader_comp_output), return, "", "Failed to compile shader: " << shader_comp_output);
    
        m_shader_program = glCreateProgram();
        glAttachShader(m_shader_program, vertexShader);
        glAttachShader(m_shader_program, fragmentShader);
        glLinkProgram(m_shader_program);
    
        GLint success;
        glGetProgramiv(m_shader_program, GL_LINK_STATUS, &success);
        if(!success) {
            char compiler_log[1024];
            glGetProgramInfoLog(m_shader_program, 1024, nullptr, compiler_log);
            LOG(Error, "Shader program linking failed: " << compiler_log);
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
