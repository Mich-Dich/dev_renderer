
#include "util/pch.h"

#include "layer.h"

namespace GLT {



	layer::layer(const std::string& name) 
		: m_debugname(name), m_enabled(false) {}

	layer::~layer() {}

	void layer::on_attach() { }

	void layer::on_detach() { }

	void layer::on_update(const f32 delta_time) {

		PROFILE_FUNCTION();

	}

	void layer::on_event(event& event) {

		PROFILE_FUNCTION();

	}

	void layer::on_imgui_render() {

		PROFILE_FUNCTION();

	}


}