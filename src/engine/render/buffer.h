#pragma once


namespace GLT::render {
    
    // Base buffer interface
    class buffer {
    public:
    
        enum class type { VERTEX, INDEX };
        enum class usage { STATIC, DYNAMIC, STREAM };
    
        buffer(type type, usage usage);
        ~buffer();
        
        void bind() const;
        void unbind() const;
        void create(const void* data, size_t size);
    
        DEFAULT_GETTER(u32,         ID)
        DEFAULT_GETTER(type,        type)
        DEFAULT_GETTER(size_t,      size)

    protected:
        type        m_type;
        usage       m_usage;
        u32         m_ID = 0;
        size_t      m_size = 0;
    };

}