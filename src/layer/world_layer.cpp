
#include "util/pch.h"

// #include "application.h"
// #include "engine/render/renderer.h"
// #include "project/script_system.h"
#include "engine/events/event.h"
#include "engine/events/mouse_event.h"
#include "game_object/camera.h"
#include "game_object/player_controller.h"
#include "util/io/serializer_yaml.h"
#include "util/util.h"

#include "world_layer.h"

// ============= DEV-ONLY =============
#include "application.h"
#include "geometry/static_mesh.h"
#include "factories/mesh/asset_importer.h"
#include "engine/render/buffer.h"
#include "engine/render/renderer.h"
// ============= DEV-ONLY =============


namespace GLT {
	
	// ============= DEV-ONLY =============
	static ref<GLT::geometry::static_mesh> MAIN_RENDER_MESH = create_ref<GLT::geometry::static_mesh>();
	ref<GLT::geometry::static_mesh> world_layer::GET_RENDER_MESH() { return MAIN_RENDER_MESH; }
	// ============= DEV-ONLY =============
	
	// ============= TODO: move to editor layer =============
	void world_layer::serialize(const serializer::option option) {

		auto perspective_fov_y = m_editor_camera->get_perspective_fov_y();
		auto direction = m_editor_camera->get_direction();
		auto position = m_editor_camera->get_position();
		LOG(Trace, "direction [" << util::to_string(direction) << "] position [" << util::to_string(position) << "]")

		serializer::yaml(util::get_executable_path().parent_path() / "config" / "editor.yml", "world layer data", option)
			.sub_section("editor camera", [&](serializer::yaml& section) {
				section.entry(KEY_VALUE(perspective_fov_y))
				.entry(KEY_VALUE(direction))
				.entry(KEY_VALUE(position));
			});

		if (option == serializer::option::load_from_file) {

			LOG(Debug, "perspective_fov_y [" << util::to_string(perspective_fov_y) << "] direction [" << util::to_string(direction) << "] position [" << util::to_string(position) << "]")
			m_editor_camera->set_fov_y(perspective_fov_y);
			m_editor_camera->set_view_XYZ(position, direction);
		}
	}
	// ============= TODO: move to editor layer =============
	

	world_layer::world_layer() { 

		LOG_INIT();
	}

	world_layer::~world_layer() { 
		
		LOG_SHUTDOWN();
		// m_map.reset();
	}


	void world_layer::register_player_controller(ref<player_controller> player_controller) {

		m_player_controller = player_controller;
		m_player_controller->set_world_layer_ref(this);
	}

	// void world_layer::set_map(const ref<map> map) {

	// 	m_map = map;
	// 	// script_system::reinit_scripts();		// reregister scripts
	// 	map->create();							// create the entities
	// }

	void world_layer::on_attach() {
	
		m_editor_camera = create_ref<camera>();
		m_editor_camera->set_view_direction(glm::vec3{ 0.0f }, glm::vec3{ 0.5f, 0.0f, 1.0f });
				
		// ============= DEV-ONLY =============
		ASSERT(GLT::factory::geometry::load_static_mesh( util::get_executable_path().parent_path() / "assets" / "meshes" / "Barrel.glb", MAIN_RENDER_MESH), "test mesh imported successfully", "Failed to import test mesh");
		application::get().get_renderer()->upload_static_mesh(MAIN_RENDER_MESH);
		// ============= DEV-ONLY =============
		
		serialize(serializer::option::load_from_file);

		LOG(Trace, "attaching world_layer");
	}

	void world_layer::on_detach() { 

		serialize(serializer::option::save_to_file);

		// m_player_controller.reset();
		m_editor_camera.reset();
		
		LOG(Trace, "detaching world_layer");
	}

	void world_layer::on_update(const f32 delta_time) {

		PROFILE_FUNCTION();

		m_player_controller->update_internal(delta_time);
		// m_map->on_update(delta_time);
		
	}

	void world_layer::on_event(event& event) {

		if (m_player_controller)
			m_player_controller->handle_event(event);
	}

	void world_layer::on_imgui_render() { }

}
