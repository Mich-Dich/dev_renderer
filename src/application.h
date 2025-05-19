#pragma once

#include "util/pch.h"

// #include "layer/layer_stack.h"
// #include "layer/imgui_layer.h"
// #include "layer/world_layer.h"

class window;
class renderer;
class event;
class window_resize_event;
class window_close_event;
class window_refresh_event;
class window_focus_event;
class map;
class camera;
class openGL_renderer;

class application {
public:

    application(int argc, char** argv);
    virtual ~application();

    DELETE_COPY_CONSTRUCTOR(application);

    DEFAULT_GETTER_C(f64,											    delta_time)
    DEFAULT_GETTER_C(f64,											    absolute_time)
    DEFAULT_GETTER_C(u32,											    target_fps)
    DEFAULT_GETTERS(u32,											    target_fps)
    DEFAULT_GETTERS(u32,											    nonefocus_fps)
    DEFAULT_GETTERS(bool,											    limit_fps)
    DEFAULT_GETTER(static ref<window>,							        window);
    // DEFAULT_GETTER(UI::imgui_layer*,								    imgui_layer);

    FORCEINLINE static application& get()								{ return *s_instance; }
    FORCEINLINE static void close_application()							{ m_running = false; }
    // FORCEINLINE static RENDERER& get_renderer()							{ return GET_RENDERER; }
    FORCEINLINE static void set_render_state(system_state state)		{} // { GET_RENDERER.set_state(state); }
    // FORCEINLINE USE_IN_EDITOR void push_overlay(layer* overlay)			{ m_layerstack->push_overlay(overlay); }
    // FORCEINLINE USE_IN_EDITOR void pop_overlay(layer* overlay)			{ m_layerstack->pop_overlay(overlay); }

    void run();

    void set_fps_settings(const bool set_for_engine_focused, const u32 new_limit);

    void limit_fps(const bool new_value, const u32 new_limit);
    void minimize_window();
    void restore_window();
    void maximize_window();

    // void register_player_controller(ref<player_controller> player_controller) { m_world_layer->register_player_controller(player_controller); }
    void get_fps_values(bool& limit_fps, u32& target_fps, u32& current_fps, f32& work_time, f32& sleep_time);

    // ---------------------- fps control ---------------------- 
    void set_fps_settings(u32 target_fps);

private:

    void init_engine();

    void on_event(event& event);
    bool on_window_close(window_close_event& event);
    bool on_window_resize(window_resize_event& event);
    bool on_window_refresh(window_refresh_event& event);
    bool on_window_focus(window_focus_event& event);
    
    void start_fps_measurement();
    void end_fps_measurement(f32& work_time);
    void limit_fps();

    static application*			s_instance;
    static ref<window>		    m_window;
    static bool					m_running;
    
    // ref<layer_stack>			m_layerstack{};
    std::vector<event>			m_event_queue;		// TODO: change to queue
    // ref<map>					m_current_map = nullptr;
    ref<openGL_renderer>        m_renderer{};

    bool						m_focus = true;
    bool						m_limit_fps = true;
    u32							m_target_fps = 60;
    u32							m_nonefocus_fps = 30;
    u32							m_fps{};
    f32							m_delta_time = 0.f;
    f32							m_absolute_time = 0.f;
    f32							m_work_time{}, m_sleep_time{};
    f32							target_duration{};
    f32							m_last_frame_time = 0.f;

};
