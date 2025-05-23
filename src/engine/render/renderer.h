
#pragma once

#include "engine/platform/window.h"
#include "engine/render/buffer.h"

namespace GLT {
    class layer_stack;
    class window;
    class camera;
}
namespace GLT::geometry { class static_mesh; }


namespace GLT::render {

    struct general_performance_metrik {

        u32 meshes = 0, draw_calls = 0;
        u64 vertices = 0;
        f32 sleep_time = 0.f, work_time = 0.f;
        u32 material_binding_count = 0, pipline_binding_count = 0;

        #define GENERAL_PERFORMANCE_METRIK_ARRAY_SIZE       200
        f32 renderer_draw_time[GENERAL_PERFORMANCE_METRIK_ARRAY_SIZE] = {};
        f32 draw_geometry_time[GENERAL_PERFORMANCE_METRIK_ARRAY_SIZE] = {};
        f32 waiting_idle_time[GENERAL_PERFORMANCE_METRIK_ARRAY_SIZE] = {};
        u16 current_index = 0;

        void next_iteration() {

            current_index = (current_index + 1) % 200;
            // renderer_draw_time[current_index] = draw_geometry_time[current_index] = waiting_idle_time[current_index] = 0.f;
            material_binding_count = pipline_binding_count = draw_calls = meshes = 0;
            vertices = 0;
            sleep_time = work_time = 0.f;
        }
    };

    class renderer {
    public:

        renderer(ref<window> window, ref<layer_stack> layer_stack)
            : m_window(window), m_layer_stack(layer_stack) {}
        virtual ~renderer() = default;
    
		DEFAULT_GETTERS(general_performance_metrik,	        general_performance_metrik)

        virtual void draw_frame(float delta_time) = 0;
        virtual void set_size(const u32 width, const u32 height) = 0;

        virtual void upload_static_mesh(ref<GLT::geometry::static_mesh> mesh) = 0;
        virtual bool reload_fragment_shader(const std::filesystem::path& frag_file, std::string& output) = 0;

        // -------- ImGui --------
        virtual void imgui_init() = 0;
        virtual void imgui_shutdown() = 0;
        virtual void imgui_create_fonts() = 0;

        // -------- fixed for all sub-classes --------
        FORCEINLINE void set_state(system_state new_state)      { m_system_state = new_state;}
		FORCEINLINE void set_active_camera(ref<camera> camera)	{ m_active_camera = camera; }

    protected:

        ref<GLT::window>                    m_window;
        ref<GLT::layer_stack>               m_layer_stack;
        system_state                        m_system_state = system_state::inactive;
        general_performance_metrik          m_general_performance_metrik{};
        ref<camera>                         m_active_camera;
    
    };

}
