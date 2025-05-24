#pragma once

#include "layer/layer.h"
// #include "engine/game_objects/camera.h"
// #include "engine/game_objects/player_controller.h"
// #include "engine/geometry/entity.h"		// importa map
// #include "engine/geometry/map.h"

namespace GLT {
	
	class camera;
	class player_controller;
	namespace serializer { enum class option; }
	
	// ============= DEV-ONLY =============
	namespace geometry { class static_mesh; }
	// ============= DEV-ONLY =============


	class world_layer : public layer {
	public:

		world_layer();
		~world_layer();
		DELETE_COPY_CONSTRUCTOR(world_layer);

		DEFAULT_GETTER_C(ref<camera>, editor_camera)
		// DEFAULT_GETTER_C(const ref<map>&, map)
		GETTER_C(ref<player_controller>, current_player_controller, m_player_controller)

		void register_player_controller(ref<player_controller> player_controller);
		// void set_map(const ref<map> map);
		
		virtual void on_attach();
		virtual void on_detach();
		virtual void on_update(const f32 delta_time);
		virtual void on_event(event& event);
		virtual void on_imgui_render();

		// ---------------------------- runtime/simulation ----------------------------

		// @brief Checks if the system is currently running.
		// @return True if the system is active, false otherwise.
		FORCEINLINE bool is_running() const { return m_system_state == system_state::active; }

		// @brief Checks if the system is currently paused.
		// @return True if the system is suspended, false otherwise.
		FORCEINLINE bool is_paused() const { return m_system_state == system_state::suspended; }

		// @brief Pauses or unpauses the system.
		// @param [should_pause] Whether the system should be paused.
		void pause(bool should_pause);

		// ============= DEV-ONLY =============
		ref<GLT::geometry::static_mesh> GET_RENDER_MESH();
		void serialize(const serializer::option option);
		// ============= DEV-ONLY =============

	private:

		// ref<map>					m_map{};
		ref<camera>					m_editor_camera{};
		ref<player_controller>		m_player_controller{};
		system_state				m_system_state = system_state::inactive;
	};
	
}
