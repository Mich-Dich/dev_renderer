#pragma once

namespace GLT {

	enum notify_filters {
		file_name = 1,
		directory_name = 2,
		attributes = 4,
		size = 8,
		last_write = 16,
		last_access = 32,
		creation_time = 64,
		security = 256
	};

	enum file_actions {
        FILE_ACTION_ADDED = 1,
        FILE_ACTION_REMOVED = 2,
        FILE_ACTION_MODIFIED = 3,
        FILE_ACTION_RENAMED_NEW_NAME = 4,
        FILE_ACTION_ATTRIBUTES_CHANGED = 5
    };


	class file_watcher_system {
	public:

		file_watcher_system();
		~file_watcher_system() { stop(); }
		void start();
		void stop();

	public:
		using on_changed_fnuc = std::function<void(const std::filesystem::path&)>;
		using on_renamed_fnuc = std::function<void(const std::filesystem::path&)>;
		using on_deleted_fnuc = std::function<void(const std::filesystem::path&)>;
		using on_created_fnuc = std::function<void(const std::filesystem::path&)>;
		using compile_fnuc = std::function<void()>;

		on_changed_fnuc				on_changed = nullptr;
		on_renamed_fnuc				on_renamed = nullptr;
		on_deleted_fnuc				on_deleted = nullptr;
		on_created_fnuc				on_created = nullptr;
		compile_fnuc				compile = nullptr;

		int							p_notify_filters = 0;
		bool						include_sub_directories = false;
		std::string					filter = "";
		std::filesystem::path		path = {};

	private:

		void start_thread();
		void process_event(const std::filesystem::path& file, int action);

		bool						m_enable_raising_events = true;
		std::thread					m_thread;

		// for debouncing
		std::mutex					m_mutex;
		std::chrono::milliseconds	m_debounce_time{ 100 };
		std::unordered_map<std::filesystem::path, std::pair<int, std::chrono::steady_clock::time_point>> m_pending_events;

	#ifdef PLATFORM_LINUX
		int m_inotify_fd = -1;
		int m_stop_pipe[2] = {-1, -1};
	#endif

	};

}
