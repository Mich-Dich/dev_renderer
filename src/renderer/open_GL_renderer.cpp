#include "util/pch.h"

#include "util/util.h"

#include "open_GL_renderer.h"

openGL_renderer::openGL_renderer(ref<window> window) 
    : m_window(window) {
    
    GLFWwindow* glfwWindow = m_window->get_window();
    glfwMakeContextCurrent(glfwWindow);
    
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    ASSERT(err == GLEW_OK, "", "Failed to initialize GLEW: " << glewGetErrorString(err))
    ASSERT(GLEW_VERSION_2_0, "", "OpenGL 2.0 not supported!")

    LOG(Info, "OpenGL Version: " << glGetString(GL_VERSION));
    LOG(Info, "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION));
    LOG(Info, "Vendor: " << glGetString(GL_VENDOR));
    LOG(Info, "Renderer: " << glGetString(GL_RENDERER));

    create_shader_program();
    create_fullscreen_quad();
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
}

openGL_renderer::~openGL_renderer() {
    
    shutdown();
}

void openGL_renderer::init() { }

void openGL_renderer::shutdown() { }

void openGL_renderer::draw_frame(float delta_time) {

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(m_shader_program);

    // Set uniforms
    GLint loc_resolution = glGetUniformLocation(m_shader_program, "u_resolution");
    GLint loc_mouse = glGetUniformLocation(m_shader_program, "u_mouse");
    GLint loc_time = glGetUniformLocation(m_shader_program, "u_time");

    int width, height;
    glfwGetFramebufferSize(m_window->get_window(), &width, &height);
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

void openGL_renderer::set_size(const u32 width, const u32 height) { glViewport(0, 0, width, height); }



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

void openGL_renderer::create_shader_program() {

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

void openGL_renderer::create_fullscreen_quad() {
    // Vertex data for fullscreen quad
    float vertices[] = {
        // positions    // texCoords
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
