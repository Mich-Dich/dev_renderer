
#include "util/pch.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "util/util.h"
#include "engine/events/event.h"
#include "engine/events/application_event.h"
#include "engine/events/mouse_event.h"
#include "engine/events/key_event.h"
#include "engine/render/renderer.h"
#include "engine/render/open_GL/GL_renderer.h"
#include "engine/platform/window.h"

#include "layer/layer.h"
#include "layer/layer_stack.h"
#include "layer/imgui_layer.h"
#include "layer/world_layer.h"

// TODO: move to editor init function        
#include "editor/controller/editor_controller.h"

#include "application.h"

// ==================================================================== setup ====================================================================

namespace GLT {

    application* application::s_instance = nullptr;
    
    //world_layer* application::m_world_layer;
    ref<window> application::m_window;
    bool application::m_running;
    
    application::application(int argc, char** argv) {
    
        logger::init("[$B$T:$J$E] [$B$L$X $I - $P:$G$E] $C$Z", true);
        logger::set_buffer_threshhold(logger::severity::Warn);
    
        ASSERT(!s_instance, "", "Application already exists");
    
        LOG_INIT();
        s_instance = this;
    
        LOG(Trace, "init engine")
    
        // ---------------------------------------- general subsystems ----------------------------------------
        set_fps_settings(m_target_fps);
    
        m_window = std::make_shared<window>();
        m_window->set_event_callback(BIND_FUNKTION(application::on_event));
        m_layerstack = create_ref<layer_stack>();
        m_renderer = create_ref<GLT::render::open_GL::GL_renderer>(m_window, m_layerstack);

        // ---------------------------------------- layers ----------------------------------------
        m_world_layer = new world_layer();
		m_layerstack->push_layer(m_world_layer);
		m_renderer->set_active_camera(m_world_layer->get_editor_camera());

        m_imgui_layer = new UI::imgui_layer();
        m_layerstack->push_overlay(m_imgui_layer);

        // TODO: move to editor init function        
		LOG(Trace, "register editor controller");
        m_world_layer->register_player_controller(create_ref<editor_controller>());
    }
    
    application::~application() {
    
        m_layerstack->pop_overlay(m_imgui_layer);
		delete m_imgui_layer;
		m_imgui_layer = nullptr;

		m_layerstack->pop_layer(m_world_layer);
		delete m_world_layer;
		m_world_layer = nullptr;

		m_renderer.reset();
		m_layerstack.reset();
        m_window.reset();
    
        LOG_SHUTDOWN();
        logger::shutdown();
    }
    
    // ==================================================================== main loop ====================================================================
    
    void application::run() {
    
        // ---------------------------------------- finished setup ----------------------------------------
        // m_renderer->set_state(system_state::active);
        m_running = true;
        m_window->show_window(true);
        m_window->poll_events();
        start_fps_measurement();
    
        while (m_running) {
    
            // PROFILE_SCOPE("run");			
            m_window->poll_events();				// update internal state
            
            for (layer* layer : *m_layerstack)		// engine update for all layers [world_layer, debug_layer, imgui_layer]
                layer->on_update(m_delta_time);
    
            m_renderer->draw_frame(m_delta_time);
            limit_fps();
        }
    
        LOG(Trace, "Exiting main run loop")
        // application::get().get_renderer().wait_idle();
    }
    
    // ==================================================================== PUBLIC ====================================================================
    
    void application::set_fps_settings(const bool set_for_engine_focused, const u32 new_limit) {
    
        if (set_for_engine_focused)
            m_target_fps = new_limit;
        else
            m_nonefocus_fps = new_limit;
    
        set_fps_settings(m_focus ? m_target_fps : m_nonefocus_fps);
    }
    
    void application::limit_fps(const bool new_value, const u32 new_limit) {
    
        m_limit_fps = new_value;
        set_fps_settings(math::max(new_limit, (u32)1 ));
    }
    
    void application::register_player_controller(ref<player_controller> player_controller) { m_world_layer->register_player_controller(player_controller); }

    void application::get_fps_values(bool& limit_fps, u32& target_fps, u32& current_fps, f32& work_time, f32& sleep_time) {
    
        limit_fps = m_limit_fps;
        target_fps = (m_focus ? m_target_fps : m_nonefocus_fps);
        current_fps = m_fps;
        work_time = m_work_time * 1000;
        sleep_time = m_sleep_time;
    }
    
    void application::minimize_window() { m_window->queue_event([window = m_window] { window->minimize_window(); }); }
    
    void application::restore_window() { m_window->queue_event([window = m_window] { window->restore_window(); }); }
    
    void application::maximize_window() { m_window->queue_event([window = m_window] { window->maximize_window(); }); }
    
    const std::filesystem::path application::get_project_path() { return util::get_executable_path().parent_path(); }               // TODO: change to actual project path when implemented


    // ==================================================================== PRIVATE ====================================================================
    
    void application::set_fps_settings(u32 target_fps) { target_duration = static_cast<f32>(1.0 / target_fps); }
    
    void application::start_fps_measurement() { m_last_frame_time = static_cast<f32>(glfwGetTime()); }
    
    void application::end_fps_measurement(f32& work_time) { work_time = static_cast<f32>(glfwGetTime()) - m_last_frame_time; }
    
    void application::limit_fps() {
    
        //m_limit_fps, m_fps, m_delta_time, m_work_time, m_sleep_time
    
        m_work_time = static_cast<f32>(glfwGetTime()) - m_last_frame_time;
        if (m_work_time < target_duration && m_limit_fps) {
    
            // PROFILE_SCOPE("sleep");
            m_sleep_time = (target_duration - m_work_time) * 1000;
            util::high_precision_sleep(m_sleep_time / 1000);
        } else
            m_sleep_time = 0;
    
        f32 time = static_cast<f32>(glfwGetTime());
        m_delta_time = std::min<f32>(time - m_last_frame_time, 100000);
        m_absolute_time += m_delta_time;
        m_last_frame_time = time;
        m_fps = static_cast<u32>(1.0 / (m_work_time + (m_sleep_time * 0.001)) + 0.5); // Round to nearest integer
    }
    
    // ==================================================================== event handling ====================================================================
    
    void application::on_event(event& event) {
    
        // application events
        event_dispatcher dispatcher(event);
        dispatcher.dispatch<window_close_event>(BIND_FUNKTION(application::on_window_close));
        dispatcher.dispatch<window_resize_event>(BIND_FUNKTION(application::on_window_resize));
        dispatcher.dispatch<window_refresh_event>(BIND_FUNKTION(application::on_window_refresh));
        dispatcher.dispatch<window_focus_event>(BIND_FUNKTION(application::on_window_focus));
    
        // none application events
        if (!event.handled) {
            for (auto layer = m_layerstack->end(); layer != m_layerstack->begin(); ) {
    
                (*--layer)->on_event(event);
                if (event.handled)
                    break;
            }
        }
    }
    
    bool application::on_window_close(window_close_event& event) {
    
        m_running = false;
        return true;
    }
    
    bool application::on_window_resize(window_resize_event& event) {
    
        m_renderer->set_size(event.get_width(), event.get_height());
    
        return true;
    }
    
    bool application::on_window_refresh(window_refresh_event& event) {
    
        limit_fps();
        return true;
    }
    
    bool application::on_window_focus(window_focus_event& event) {
    
        m_focus = event.get_focus();
        if (event.get_focus())
            set_fps_settings(m_target_fps);
        else
            set_fps_settings(m_nonefocus_fps);
    
        return true;
    }

}    

