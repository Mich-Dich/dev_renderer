
#include "util/pch.h"

// #include "application.h"
// #include "engine/render/renderer.h"
// #include "engine/events/event.h"
// #include "project/script_system.h"

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

	world_layer::world_layer() { 

		LOG_INIT();
	}

	world_layer::~world_layer() { 
		
		LOG_SHUTDOWN();
		// m_map.reset();
	}


	// void world_layer::register_player_controller(ref<player_controller> player_controller) {

	// 	m_player_controller = player_controller;
	// 	m_player_controller->set_world_layer_ref(this);
	// }

	// void world_layer::set_map(const ref<map> map) {

	// 	m_map = map;
	// 	// script_system::reinit_scripts();		// reregister scripts
	// 	map->create();							// create the entities
	// }

	void world_layer::on_attach() {
	
		// m_editor_camera = std::make_shared<camera>();
		// m_editor_camera->set_view_direction(glm::vec3{ 0.0f }, glm::vec3{ 0.5f, 0.0f, 1.0f });
		//m_editor_camera.set_orthographic_projection(-aspect, aspect, -1, 1, 0, 10);
		//m_editor_camera->set_perspective_projection(glm::radians(50.f), application::get().application::get().get_renderer()()->get_aspect_ratio(), 0.1f, 350.0f);

		//float aspect = m_swapchain->get_extentAspectRatio();
		//m_editor_camera.set_view_target(glm::vec3(-1.0f, -2.0f, -3.0f), glm::vec3(0.0f));
				
		// ============= DEV-ONLY =============
		ASSERT(GLT::factory::geometry::load_mesh("/home/mich/Documents/gameassets_3D/_exports/basic_test_meshes/sphere.glb", MAIN_RENDER_MESH), "test mesh imported successfully", "Failed to import test mesh");
		application::get().get_renderer()->upload_mesh(MAIN_RENDER_MESH);
		// ============= DEV-ONLY =============


		LOG(Trace, "attaching world_layer");
	}

	void world_layer::on_detach() { 

		// m_player_controller.reset();
		// m_editor_camera.reset();
		
		LOG(Trace, "detaching world_layer");
	}

	void world_layer::on_update(const f32 delta_time) {

		PROFILE_FUNCTION();

		// m_player_controller->update_internal(delta_time);
	
		// m_map->on_update(delta_time);
		
	}

	void world_layer::on_event(event& event) {

		PROFILE_FUNCTION();

		// if (m_player_controller)
		// 	m_player_controller->handle_event(event);
	}

	void world_layer::on_imgui_render() { }

}
