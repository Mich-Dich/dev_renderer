
#pragma once

#include "engine/platform/window.h"

namespace GLT { class layer_stack; }

namespace GLT::render {

    struct general_performance_metrik {

        u32 mesh_draw = 0, draw_calls = 0;
        u64 triangles = 0;
        f32 sleep_time = 0.f, work_time = 0.f;
        u32 material_binding_count = 0, pipline_binding_count = 0;

        f32 renderer_draw_time[200] = {};
        f32 draw_geometry_time[200] = {};
        f32 waiting_idle_time[200] = {};
        u16 current_index = 0;

        void reset() {

            current_index = (current_index + 1) % 200;

            material_binding_count = pipline_binding_count = draw_calls = mesh_draw = 0;
            triangles = 0;
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

        virtual void reload_fragment_shader(const std::filesystem::path& frag_file) = 0;

        // -------- ImGui --------
        virtual void imgui_init() = 0;
        virtual void imgui_shutdown() = 0;
        virtual void imgui_create_fonts() = 0;

        // -------- fixed for all sub-classes --------
        void set_state(system_state new_state) { m_system_state = new_state;}

    protected:

        ref<GLT::window>        m_window;
        ref<GLT::layer_stack>   m_layer_stack;
        system_state            m_system_state = system_state::inactive;
        
    private:

		general_performance_metrik           m_general_performance_metrik{};

    };

}
