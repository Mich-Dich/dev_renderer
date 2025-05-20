
#include "util/pch.h"

#include "layer_stack.h"
#include "layer.h"


namespace GLT {

	layer_stack::layer_stack() { LOG_INIT(); }
	
	layer_stack::~layer_stack() {

		for (layer* layer : m_layers)
			delete layer;

		LOG_SHUTDOWN();
	}

	void layer_stack::push_layer(layer* layer) {

		PROFILE_FUNCTION();

		m_layers.emplace(m_layers.begin() + m_layer_insert, layer);
		m_layer_insert++;

		layer->on_attach();
	}
	
	void layer_stack::pop_layer(layer* layer) {

		PROFILE_FUNCTION();

		auto target = std::find(m_layers.begin(), m_layers.end(), layer);
		if (target != m_layers.end()) {

			m_layers.erase(target);
			m_layer_insert--;

			layer->on_detach();
		}
	}

	void layer_stack::push_overlay(layer* overlay) {

		PROFILE_FUNCTION();

		m_layers.emplace_back(overlay);
		overlay->on_attach();
	}

	void layer_stack::pop_overlay(layer* overlay) {

		PROFILE_FUNCTION();

		auto target = std::find(m_layers.begin(), m_layers.end(), overlay);
		if (target != m_layers.end()) {
		
			m_layers.erase(target);
			overlay->on_detach();
		}

	}

	void layer_stack::delete_all_layers() {

		PROFILE_FUNCTION();

		for (layer* layer : m_layers)
			delete layer;
		m_layer_insert = 0;
	}
}
