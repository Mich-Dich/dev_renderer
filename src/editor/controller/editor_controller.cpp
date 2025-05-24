
#include "util/pch.h"
//#include <engine/Events/Event.h>

#include "application.h"
#include "editor_inputs.h"
#include "layer/world_layer.h"
#include "game_object/camera.h"
#include "util/util.h"

#include "editor_controller.h"


namespace GLT {

	constexpr float MAX_PITCH_DEGREES = 89.0f;                 
	constexpr float MAX_PITCH   = glm::radians(MAX_PITCH_DEGREES);
	constexpr float MIN_PITCH   = glm::radians(-MAX_PITCH_DEGREES);

	editor_controller::editor_controller() {

		LOG(Trace, "init");
		m_camera_pos = application::get().get_world_layer()->get_editor_camera()->get_position();
		m_camera_direction = application::get().get_world_layer()->get_editor_camera()->get_direction();
		m_input_mapping = register_mapping<editor_inputs>();
	}

	editor_controller::~editor_controller() {

		LOG(Trace, "shutdown");
	}

	void editor_controller::update(const f32 delta_time) {

		// if (m_input_mapping->transform_operation_to_translate.data.boolean) { PFF_editor::get().get_editor_layer()->get_world_viewport_window()->get_gizmo_operation_ref() = transform_operation::translate; }
		// if (m_input_mapping->transform_operation_to_rotate.data.boolean) { PFF_editor::get().get_editor_layer()->get_world_viewport_window()->get_gizmo_operation_ref() = transform_operation::rotate; }
		// if (m_input_mapping->transform_operation_to_scale.data.boolean) { PFF_editor::get().get_editor_layer()->get_world_viewport_window()->get_gizmo_operation_ref() = transform_operation::scale; }

		//if (m_input_mapping->capture_mouse.get_data<bool>()) { }
		if (m_input_mapping->capture_mouse.data.boolean) {

			if (m_input_mapping->change_rotation_origin.data.boolean) {
				
				glm::vec2 sensitivity = glm::vec2(.2f, .2f);
				f32 yaw_angle = m_input_mapping->look.data.vec_2D.x * delta_time * sensitivity.x;
				glm::mat4 yaw_rotation = glm::rotate(glm::mat4(1.0f), yaw_angle, glm::vec3(0.0f, 1.0f, 0.0f));				// rotation matrix around the Z-axis

				// f32 pitch_angle = m_input_mapping->look.data.vec_2D.y * delta_time * sensitivity.y;
				// glm::mat4 pitch_rotation = glm::rotate(glm::mat4(1.0f), pitch_angle, glm::vec3(1.0f, 0.0f, 0.0f));				// rotation matrix around the Z-axis

				glm::vec4 rotated_dir = yaw_rotation * /*pitch_rotation **/ glm::vec4(m_camera_pos.x, m_camera_pos.y, m_camera_pos.z, 1.f);
				m_camera_pos = glm::vec3(rotated_dir.x, rotated_dir.y, rotated_dir.z);
				
				// m_camera_direction = glm::vec3(m_camera_pos.x, m_camera_pos.y, 0.f);
				m_camera_direction.y += m_input_mapping->look.data.vec_2D.x * delta_time;

			} else {
				// application::get().capture_cursor();
				m_camera_direction.y += m_input_mapping->look.data.vec_2D.x * 0.1f * delta_time;
				m_camera_direction.x += m_input_mapping->look.data.vec_2D.y * 0.1f * delta_time;
				m_camera_direction.x = glm::clamp(m_camera_direction.x, MIN_PITCH, MAX_PITCH);									// clamp pitch to [-89°, +89°] so camera can’t flip over
	
				if (m_input_mapping->change_move_speed.data.vec_1D != 0) {
					float speed_change = m_move_speed * 0.1f * m_input_mapping->change_move_speed.data.vec_1D;
					m_move_speed += speed_change;
					m_move_speed = std::clamp(m_move_speed, 1.0f, 100000.0f);
				}
					// m_move_speed = std::clamp(m_move_speed += (m_move_speed * 0.1f * m_input_mapping->change_move_speed.data.vec_1D), 1.0f, 100000.0f);
	
				float yaw   = m_camera_direction.y;
				float pitch = m_camera_direction.x;
				// const glm::vec3 forward_dir{ sin(-yaw), m_camera_direction.x, cos(yaw) };
				const glm::vec3 forward_dir{cos(pitch) * sin(-yaw), sin(pitch), cos(pitch) * cos(yaw)};
				const glm::vec3 right_dir{ forward_dir.z, 0.f, -forward_dir.x };
				
				// LOG(Trace, "Move [" << util::to_string(m_input_mapping->move.data.vec_3D) << "] m_move_speed [" << m_move_speed << "]")
				m_camera_pos += forward_dir * (m_input_mapping->move.data.vec_3D.x * m_move_speed * delta_time);					// Move forward	/backward
				m_camera_pos += right_dir * (m_input_mapping->move.data.vec_3D.y * m_move_speed * delta_time);					// Move right	/left
				m_camera_pos += glm::vec3{ 0.f, -1.f, 0.f } *(m_input_mapping->move.data.vec_3D.z * m_move_speed * delta_time);	// Move up		/down
			}

			application::get().get_world_layer()->get_editor_camera()->set_view_XYZ(m_camera_pos, m_camera_direction);

		} // else
			// application::get().release_cursor();

		if (m_input_mapping->toggle_fps.data.boolean)
			application::get().limit_fps(!application::get().get_limit_fps(), application::get().get_target_fps());
		
		//LOG(Trace, vec_to_str(m_camera_direction, "dir") << vec_to_str(m_camera_pos, "pos") << vec_to_str(m_input_mapping->move.data._3D, "input"));
	}

	void editor_controller::serialize(serializer::option option) {

		// serializer::yaml(config::get_filepath_from_configtype(PFF_editor::get().get_project_path(), config::file::editor), "editor_camera", option)
		// 	.entry(KEY_VALUE(m_move_speed));

	}
}
