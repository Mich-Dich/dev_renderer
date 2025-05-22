
#include "util/pch.h"

#include <stb_image.h>

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

// #include "application.h"
// #include "util/system.h"
// #include "util/serializer_yaml.h"
#include "engine/events/event.h"
#include "engine/events/application_event.h"
#include "engine/events/mouse_event.h"
#include "engine/events/key_event.h"

#include "window.h"


namespace GLT {

	static bool s_GLFWinitialized = false;
		
	static FORCEINLINE void GLFW_error_callback(int errorCode, const char* description) { LOG(Error, "[GLFW Error: " << errorCode << "]: " << description); }
	
	// ================================================================================== setup ==================================================================================
	
	window::window(window_attrib attributes) :
		m_data(attributes) {
	
		LOG_INIT();
	
		// serialize_window_atributes(&m_data, serializer::option::load_from_file);
		
		if (!s_GLFWinitialized) {
	
			glfwSetErrorCallback(GLFW_error_callback);
			ASSERT(glfwInit(), "GLFW initialized", "Could not initialize GLFW");
			s_GLFWinitialized = true;
		}
	
		// In window constructor (window.cpp)
		glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);  // Force OpenGL
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
		glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);  // Enable double buffering
	
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
		glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
		glfwWindowHint(GLFW_MAXIMIZED, (m_data.size_state == window_size_state::fullscreen || m_data.size_state == window_size_state::fullscreen_windowed) ? GLFW_TRUE : GLFW_FALSE);
	
		int max_posible_height = 0;
		int max_posible_width = 0;
		int monitor_count;
		auto monitors = glfwGetMonitors(&monitor_count);
	
		for (int x = 0; x < monitor_count; x++) {
	
			int xpos, ypos, width, height;
			glfwGetMonitorWorkarea(monitors[x], &xpos, &ypos, &width, &height);
			max_posible_height = math::max(max_posible_height, height);
			max_posible_width = math::max(max_posible_width, width);
	
			LOG(Trace, "Monitor: " << x << " data: " << xpos << " / " << ypos << " / " << width << " / " << height);
		}
	
		// ensure window is never bigger than possible OR smaller then logical
		m_data.height = math::clamp((int)m_data.height, 200, max_posible_height);
		m_data.width = math::clamp((int)m_data.width, 300, max_posible_width);
		m_data.height -= 35;
		m_data.width -= 8;
		m_Window = glfwCreateWindow(static_cast<int>(m_data.width), static_cast<int>(m_data.height), m_data.title.c_str(), nullptr, nullptr);
	
		ASSERT(glfwVulkanSupported(), "", "GLFW does not support Vulkan");
		LOG(Trace, "Creating window [" << m_data.title << " width: " << m_data.width << "  height: " << m_data.height << "]");
	
		//glfwSetWindowPos(m_Window, 100, 100);
		glfwSetWindowPos(m_Window, m_data.pos_x, m_data.pos_y);
		LOG(Trace, "window pos [" << m_data.title << " X: " << m_data.pos_x << "  Y: " << m_data.pos_y << "]");
		
		glfwSetWindowUserPointer(m_Window, &m_data);
		glfwGetCursorPos(m_Window, &m_data.cursor_pos_x, &m_data.cursor_pos_y);
		set_vsync(m_data.vsync);
	
		GLFWmonitor* primary = glfwGetPrimaryMonitor();
		const GLFWvidmode* mode = glfwGetVideoMode(primary);
		glfwSetWindowMonitor(m_Window, NULL, m_data.pos_x, m_data.pos_y, m_data.width, m_data.height, mode->refreshRate);
	
		if (m_data.size_state == window_size_state::fullscreen
			|| m_data.size_state == window_size_state::fullscreen_windowed)
			glfwMaximizeWindow(m_Window);
	
		bind_event_calbacks();
	}
	
	window::~window() {
	
		const bool is_maximized = this->is_maximized();
		int titlebar_vertical_offset = is_maximized ? 12 : 6;
	
		int loc_pos_x;
		int loc_pos_y;
		glfwGetWindowPos(m_Window, &loc_pos_x, &loc_pos_y);
		m_data.pos_x = loc_pos_x + 4;								// window padding
		m_data.pos_y = loc_pos_y + 25 + titlebar_vertical_offset;	// [custom titlebar height offset] + [aximize offset]
	
		glfwDestroyWindow(m_Window);
		glfwTerminate();
		LOG_SHUTDOWN();
	}
	
	// ============================================================================== inplemantation ==============================================================================
	
	
	void window::bind_event_calbacks() {
	
		glfwSetWindowRefreshCallback(m_Window, [](GLFWwindow* window) {
			
			window_attrib& data = *(window_attrib*)glfwGetWindowUserPointer(window);
			window_refresh_event event;
			data.event_callback(event);
		});
	
		glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* window, int width, int height) {
	
			window_attrib& Data = *(window_attrib*)glfwGetWindowUserPointer(window);
	
			if (Data.size_state == window_size_state::windowed) {
	
				Data.width = static_cast<u32>(width);
				Data.height = static_cast<u32>(height);
			}
	
			window_resize_event event(static_cast<u32>(width), static_cast<u32>(height));
			Data.event_callback(event);
		});
	
		glfwSetWindowFocusCallback(m_Window, [](GLFWwindow* window, int focused) {
	
			window_attrib& data = *(window_attrib*)glfwGetWindowUserPointer(window);
			window_focus_event event(focused == GLFW_TRUE);
			data.event_callback(event);
		});
	
		glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* window) {
			
			window_attrib& Data = *(window_attrib*)glfwGetWindowUserPointer(window);
			window_close_event event;
			Data.event_callback(event);
		});
	
		glfwSetWindowPosCallback(m_Window, [](GLFWwindow* window, int x, int y) {
	
		});
	
		glfwSetWindowMaximizeCallback(m_Window, [](GLFWwindow* window, int maximized) {
	
			window_attrib& Data = *(window_attrib*)glfwGetWindowUserPointer(window);
			Data.size_state = maximized? window_size_state::fullscreen : window_size_state::windowed;
				LOG(Trace, "Maximize window: " << maximized);
		});
	
		glfwSetScrollCallback(m_Window, [](GLFWwindow* window, double xOffset, double yOffset) {
	
			window_attrib& Data = *(window_attrib*)glfwGetWindowUserPointer(window);
			mouse_event event_x(key_code::mouse_scrolled_x, static_cast<f32>(xOffset));
			mouse_event event_y(key_code::mouse_scrolled_y, static_cast<f32>(yOffset));
			Data.event_callback(event_x);
			Data.event_callback(event_y);
		});
	
		glfwSetCursorPosCallback(m_Window, [](GLFWwindow* window, double xPos, double yPos) {
	
			window_attrib& Data = *(window_attrib*)glfwGetWindowUserPointer(window);
			mouse_event event_x(key_code::mouse_moved_x, static_cast<f32>(Data.cursor_pos_x - xPos));
			mouse_event event_y(key_code::mouse_moved_y, static_cast<f32>(Data.cursor_pos_y - yPos));
			Data.event_callback(event_x);
			Data.event_callback(event_y);
	
			Data.cursor_pos_x = xPos;
			Data.cursor_pos_y = yPos;
		});
	
		glfwSetKeyCallback(m_Window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
	
			window_attrib& Data = *(window_attrib*)glfwGetWindowUserPointer(window);
			key_event event(static_cast<key_code>(key), static_cast<key_state>(action));
			Data.event_callback(event);
		});
	
		glfwSetMouseButtonCallback(m_Window, [](GLFWwindow* window, int button, int action, int mods) {
	
			window_attrib& Data = *(window_attrib*)glfwGetWindowUserPointer(window);
			key_event event(static_cast<key_code>(button), static_cast<key_state>(action));
			Data.event_callback(event);
		});
	
	}
	
	void window::poll_events() {
	
		glfwPollEvents();
	
		// prossess constom queue
		std::scoped_lock<std::mutex> lock(m_event_queue_mutex);
		while (m_event_queue.size() > 0) {
	
			auto& func = m_event_queue.front();
			func();
			m_event_queue.pop();
		}
	}
	
	void window::capture_cursor() { glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); }
	
	void window::release_cursor() { glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL); }
	
	void window::set_vsync(bool enable) { LOG(Warn, "VSync functionality not supported yet"); }
	
	VkExtent2D window::get_extend() { return { static_cast<u32>(m_data.width), static_cast<u32>(m_data.height) }; }
	
	bool window::should_close() { return glfwWindowShouldClose(m_Window); }
	
	void window::get_framebuffer_size(int& width, int& height) { glfwGetFramebufferSize(m_Window, &width, &height); }
	
	void window::show_window(bool show) { show ? glfwShowWindow(m_Window) : glfwHideWindow(m_Window); }
	
	bool window::is_maximized() { return static_cast<bool>(glfwGetWindowAttrib(m_Window, GLFW_MAXIMIZED)); }
	
	void window::minimize_window() {
		
		LOG(Trace, "minimizing window");
		glfwIconifyWindow(m_Window);
		m_data.size_state = window_size_state::minimised;
		//application::set_render_state(system_state::inactive);
	}
	
	void window::restore_window() {
		
		LOG(Trace, "restoring window");
		glfwRestoreWindow(m_Window); 
		//application::set_render_state(system_state::active);
	}
	
	void window::maximize_window() { 
		
		LOG(Trace, "maximising window");
		glfwMaximizeWindow(m_Window);
		m_data.size_state = window_size_state::fullscreen_windowed;
		//application::set_render_state(system_state::active);
	}
	
	void window::show_titlebar(bool show) {
		
		glfwWindowHint(GLFW_TITLEBAR, show ? GLFW_TRUE : GLFW_FALSE);
	}
	
	void window::get_monitor_size(int* width, int* height) {
	
		auto monitor = glfwGetWindowMonitor(m_Window);
		if (!monitor)
			monitor = glfwGetPrimaryMonitor();
	
		auto video_mode = glfwGetVideoMode(monitor);
	
		*width = video_mode->width;
		*height = video_mode->height;
	}
	
	void window::get_mouse_position(glm::vec2& pos) {
	
		// Get window position and size
		int winX, winY, winWidth, winHeight;
		glfwGetWindowPos(m_Window, &winX, &winY);
		glfwGetWindowSize(m_Window, &winWidth, &winHeight);
	
		double screenX, screenY;
		glfwGetCursorPos(m_Window, &screenX, &screenY);
	
		screenX += winX;
		screenY += winY;
	
		if (screenX >= winX && screenX <= winX + winWidth &&
			screenY >= winY && screenY <= winY + winHeight) {
	
			pos.x = static_cast<f32>(screenX - winX);
			pos.y = static_cast<f32>(screenY - winY);
		}
	}
	
	void window::create_vulkan_surface(VkInstance_T* instance, VkSurfaceKHR_T** get_surface) {
	
		ASSERT(glfwCreateWindowSurface(instance, m_Window, nullptr, get_surface) == VK_SUCCESS, "", "Failed to create a window surface");
	}

}	
