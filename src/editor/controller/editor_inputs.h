#pragma once

#include "io/input_mapping.h"

namespace GLT {

	class editor_inputs: public input_mapping {
	public:

		editor_inputs();
		~editor_inputs();

		input_action move;
		input_action capture_mouse;
		input_action change_rotation_origin;
		input_action change_move_speed;
		input_action look;
		input_action toggle_fps;

		input_action transform_operation_to_translate;
		input_action transform_operation_to_rotate;
		input_action transform_operation_to_scale;
	};

	editor_inputs::editor_inputs() {

		move = input_action{};
		move.description = "reposition the editor camera";
		move.triger_when_paused = false;
		move.flags = INPUT_ACTION_MODEFIER_SMOOTH_INTERP | INPUT_ACTION_MODEFIER_USE_VEC_NORMAL | INPUT_ACTION_MODEFIER_AUTO_RESET;
		move.value = input::action_type::vec_3D;
		move.duration_in_sec = 0.5f;
		move.keys_bindings = {
			{key_code::key_W, INPUT_ACTION_TRIGGER_KEY_DOWN, INPUT_ACTION_MODEFIER_NEGATE},
			{key_code::key_S, INPUT_ACTION_TRIGGER_KEY_DOWN},
			{key_code::key_A, INPUT_ACTION_TRIGGER_KEY_DOWN, INPUT_ACTION_MODEFIER_AXIS_2 | INPUT_ACTION_MODEFIER_NEGATE},
			{key_code::key_D, INPUT_ACTION_TRIGGER_KEY_DOWN, INPUT_ACTION_MODEFIER_AXIS_2},
			{key_code::key_space, INPUT_ACTION_TRIGGER_KEY_DOWN, INPUT_ACTION_MODEFIER_AXIS_3 | INPUT_ACTION_MODEFIER_NEGATE},
			{key_code::key_left_shift, INPUT_ACTION_TRIGGER_KEY_DOWN, INPUT_ACTION_MODEFIER_AXIS_3 },
		};								
		move.set_name("move");
		register_action(&move, true);


		capture_mouse = input_action{};
		capture_mouse.description = "left clicl to change the camera orientation";
		capture_mouse.triger_when_paused = false;
		capture_mouse.flags = 0;
		capture_mouse.value = input::action_type::boolean;
		capture_mouse.duration_in_sec = 0.5f;
		capture_mouse.keys_bindings = {
			{key_code::mouse_bu_right, INPUT_ACTION_TRIGGER_KEY_DOWN},
		};
		capture_mouse.set_name("capture_mouse");
		register_action(&capture_mouse, true);

		
		change_rotation_origin = input_action{};
		change_rotation_origin.description = "left control to let the camera rotate around the origin";
		change_rotation_origin.triger_when_paused = false;
		change_rotation_origin.flags = 0;
		change_rotation_origin.value = input::action_type::boolean;
		change_rotation_origin.duration_in_sec = 0.5f;
		change_rotation_origin.keys_bindings = {
			{key_code::key_left_control, INPUT_ACTION_TRIGGER_KEY_DOWN},
		};
		change_rotation_origin.set_name("change_rotation_origin");
		register_action(&change_rotation_origin, true);


		look = input_action{};
		look.description = "change the direction of the editor camera";
		look.triger_when_paused = false;
		look.flags = INPUT_ACTION_MODEFIER_AUTO_RESET_ALL | INPUT_ACTION_MODEFIER_SMOOTH_INTERP;
		look.value = input::action_type::vec_2D;
		look.duration_in_sec = 0.5f;
		look.keys_bindings = {
			{key_code::mouse_moved_x, INPUT_ACTION_TRIGGER_MOUSE_POS_AND_NEG, INPUT_ACTION_MODEFIER_NEGATE},
			{key_code::mouse_moved_y, INPUT_ACTION_TRIGGER_MOUSE_POS_AND_NEG, INPUT_ACTION_MODEFIER_AXIS_2 | INPUT_ACTION_MODEFIER_NEGATE},
		};
		look.set_name("look");
		register_action(&look, true);


		change_move_speed = input_action{};
		change_move_speed.description = "increase/decrease the movement speed of the editor camera";
		change_move_speed.triger_when_paused = false;
		change_move_speed.flags = INPUT_ACTION_MODEFIER_AUTO_RESET_ALL;
		change_move_speed.value = input::action_type::vec_1D;
		change_move_speed.duration_in_sec = 0.5f;
		change_move_speed.keys_bindings = {
			{key_code::mouse_scrolled_y, INPUT_ACTION_TRIGGER_MOUSE_POS_AND_NEG},
		};
		change_move_speed.set_name("change_move_speed"); 
		register_action(&change_move_speed, true);


		toggle_fps = input_action{};
		toggle_fps.description = "toggle the fps limiter";
		toggle_fps.triger_when_paused = false;
		toggle_fps.flags = INPUT_ACTION_MODEFIER_AUTO_RESET_ALL;
		toggle_fps.value = input::action_type::boolean;
		toggle_fps.duration_in_sec = 0.f;
		toggle_fps.keys_bindings = {
			{key_code::key_P, INPUT_ACTION_TRIGGER_KEY_MOVE_DOWN},
		};
		toggle_fps.set_name("toggle_fps");
		register_action(&toggle_fps, true);


		transform_operation_to_translate = input_action{};
		transform_operation_to_translate.description = "set transform operation to translate";
		transform_operation_to_translate.triger_when_paused = false;
		transform_operation_to_translate.flags = INPUT_ACTION_MODEFIER_AUTO_RESET_ALL;
		transform_operation_to_translate.value = input::action_type::boolean;
		transform_operation_to_translate.duration_in_sec = 0.f;
		transform_operation_to_translate.keys_bindings = {
			{key_code::key_E, INPUT_ACTION_TRIGGER_KEY_MOVE_DOWN},
		};
		transform_operation_to_translate.set_name("transform_operation_to_translate");
		register_action(&transform_operation_to_translate, true);


		transform_operation_to_rotate = input_action{};
		transform_operation_to_rotate.description = "set transform operation to rotate";
		transform_operation_to_rotate.triger_when_paused = false;
		transform_operation_to_rotate.flags = INPUT_ACTION_MODEFIER_AUTO_RESET_ALL;
		transform_operation_to_rotate.value = input::action_type::boolean;
		transform_operation_to_rotate.duration_in_sec = 0.f;
		transform_operation_to_rotate.keys_bindings = {
			{key_code::key_R, INPUT_ACTION_TRIGGER_KEY_MOVE_DOWN},
		};
		transform_operation_to_rotate.set_name("transform_operation_to_rotate");
		register_action(&transform_operation_to_rotate, true);


		transform_operation_to_scale = input_action{};
		transform_operation_to_scale.description = "set transform operation to scale";
		transform_operation_to_scale.triger_when_paused = false;
		transform_operation_to_scale.flags = INPUT_ACTION_MODEFIER_AUTO_RESET_ALL;
		transform_operation_to_scale.value = input::action_type::boolean;
		transform_operation_to_scale.duration_in_sec = 0.f;
		transform_operation_to_scale.keys_bindings = {
			{key_code::key_T, INPUT_ACTION_TRIGGER_KEY_MOVE_DOWN},
		};
		transform_operation_to_scale.set_name("transform_operation_to_scale");
		register_action(&transform_operation_to_scale, true);

	}

	editor_inputs::~editor_inputs() {}

}
