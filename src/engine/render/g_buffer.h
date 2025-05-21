#pragma once


namespace GLT::render {
    
    // Base buffer interface
    struct g_buffer {
    
        u32 FBO{};
        u32 position{};
        u32 normal{};
        u32 albedo_spec{};
        u32 depth_rbo{};
        
        void create(int width, int height);
        void destroy();
        void resize(int width, int height);
        
    };

}
