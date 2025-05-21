#include "util/pch.h"

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "buffer.h"

namespace GLT::render {

#if defined(RENDERER_OPENGL)

    GLenum gl_usage(const buffer::usage usage) {
        switch(usage) {
            case buffer::usage::STATIC: return GL_STATIC_DRAW;
            case buffer::usage::DYNAMIC: return GL_DYNAMIC_DRAW;
            case buffer::usage::STREAM: return GL_STREAM_DRAW;
            default: return GL_STATIC_DRAW;
        }
    }


    buffer::buffer(type type, usage usage)
        : m_type(type), m_usage(usage) {}
    
    buffer::~buffer() {
        if (m_ID)
            glDeleteBuffers(1, &m_ID);
    }

    void buffer::create(const void* data, size_t size) {

        if (m_ID)
            glDeleteBuffers(1, &m_ID);
        
        glGenBuffers(1, &m_ID);
        bind();

        switch(m_type) {
            case type::VERTEX:      glBufferData(GL_ARRAY_BUFFER, size, data, gl_usage(m_usage)); break;
            case type::INDEX:       glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, gl_usage(m_usage)); break;
            default: break;
        }
        
        unbind();
        m_size = size;
    }

    void buffer::bind() const {
        GLenum target = m_type == type::VERTEX ? GL_ARRAY_BUFFER : GL_ELEMENT_ARRAY_BUFFER;
        glBindBuffer(target, m_ID);
    }

    void buffer::unbind() const {
        GLenum target = m_type == type::VERTEX ? GL_ARRAY_BUFFER : GL_ELEMENT_ARRAY_BUFFER;
        glBindBuffer(target, 0);
    }
#else
    #error "No valid renderer configured"
#endif

}
