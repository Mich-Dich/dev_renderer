#pragma once

#include "util/pch.h"

#include "util/core_config.h"
#include "util/data_structures/data_types.h"
#include "util/io/logger.h"
#include "util/macros.h"
#include "util/timing/stopwatch.h"

// #include <algorithm>
// #include <chrono>
// #include <fstream>
// #include <filesystem>
// #include <iomanip>
// #include <string>
// #include <thread>
// #include <mutex>
// #include <sstream>


namespace GLT {

// #if defined(DEBUG)

	namespace Profiling {

#define ISOLATED_PROFILER_SCOPED(num_of_tests, message, profile_duration_precision)     static Profiling::simple_profiler loc_simple_profiler(num_of_tests, message, profile_duration_precision);      \
                                                                                            stopwatch loc_stopwatch_for_profiling(&loc_simple_profiler.single_duration, profile_duration_precision);       \
                                                                                            if (loc_simple_profiler.add_value())                                                                                \
                                                                                                loc_stopwatch_for_profiling.restart()


#define DURATION_microseconds       " microseconds"
#define DURATION_miliseconds        " milliseconds"
#define DURATION_seconds            " seconds"

#define ISOLATED_PROFILER_LOOP(num_of_iterations, message, profile_duration_precision, func)                                                                                                      \
            {                                                                                                                                                                                         \
                f32 duration = -1.f;                                                                                                                                                                  \
                {   stopwatch loc_stpowatch(&duration);                                                                                                                                          \
                    for (size_t x = 0; x < num_of_iterations; x++) { func }                                                                                                                           \
                } LOG(Info, message << " => sample count: " << num_of_iterations << " average duration: " << (duration / (f64)num_of_iterations) << DURATION_##profile_duration_precision);  \
            }

		class simple_profiler {
		public:

			simple_profiler(u64 num_of_tests, std::string message, duration_precision presition = duration_precision::milliseconds)
				: m_num_of_tests(num_of_tests), m_message(message), m_presition(presition) {}

			bool add_value() {

				if (single_duration == -1.f)
					return true;

				if (m_number_of_values >= m_num_of_tests)
					return false;

				m_durations += single_duration;
				m_number_of_values++;
				if (m_number_of_values >= m_num_of_tests) {

					std::string precition_string;
					switch (m_presition) {
						case duration_precision::microseconds:     precition_string = " microseconds"; break;
						case duration_precision::seconds:          precition_string = " seconds"; break;
						default:
						case duration_precision::milliseconds:     precition_string = " milliseconds"; break;
					}
					LOG(Info, m_message << " => sample count: " << m_number_of_values << " average duration: " << (m_durations / (f64)m_number_of_values) << precition_string);
				}
				return true;
			}
			f32  single_duration = -1.f;

		private:
			u64							m_num_of_tests{};
			std::string					m_message{};
			duration_precision			m_presition{};
			f64							m_durations{};
			u32							m_number_of_values{};
		};

	}
// #else
// 	#define ISOLATED_PROFILER_SCOPED(...)
// 	#define ISOLATED_PROFILER_LOOP(...)
// #endif

	using float_microseconds = std::chrono::duration<double, std::micro>;

	struct profile_result {

		std::string name;
		float_microseconds start;
		std::chrono::microseconds elapsed_time;
		std::thread::id thread_ID;
	};

	struct instumentation_session {

		std::string name;
	};

	// ==================================================================== instrumentor ====================================================================

	class instrumentor {
	public:

		DELETE_COPY_CONSTRUCTOR(instrumentor);

		void begin_session(const std::string& name, const std::string& directory, const std::string& filename = "result.json") {
			
			std::lock_guard lock(m_mutex);

			if (m_current_session) {

				// If there is already a current session, then close it before beginning new one.
				// Subsequent profiling output meant for the original session will end up in the newly opened session instead.  That's better than having badly formatted profiling output
				LOG(Error, "Instrumentor::BeginSession(" << name << ") when session [" << m_current_session->name << "] already open");
				internal_end_session();
			}

			// InstrumentorUtils::create_directory(directory);

			if (!std::filesystem::exists(directory)) {
				if (!std::filesystem::create_directory(directory)) {

					LOG(Error, "Failed to create folder");
				}
			}

			std::string total_filepath = directory + "/" + filename;

			m_output_stream.open(total_filepath);
			if (m_output_stream.is_open()) {

				m_current_session = new instumentation_session{ name };
				write_header();
			} else
				LOG(Error, "Instrumentor could not open file: " << filename);
		}

		void end_session() {

			std::lock_guard lock(m_mutex);
			internal_end_session();
		}

		void write_profile(const profile_result& result) {

			std::stringstream json;

			json << std::setprecision(3) << std::fixed;
			json << ",{";
			json << "\"cat\":\"function\",";
			json << "\"dur\":" << (result.elapsed_time.count()) << ',';
			json << "\"name\":\"" << result.name << "\",";
			json << "\"ph\":\"X\",";
			json << "\"pid\":0,";
			json << "\"tid\":" << result.thread_ID << ",";
			json << "\"ts\":" << result.start.count();
			json << "}";

			std::lock_guard lock(m_mutex);

			if (m_current_session) {

				m_output_stream << json.str();
				m_output_stream.flush();
				//LOG(Error, "profile function: " << result.name);
			}

		}

		static instrumentor& get() {

			static instrumentor instance;
			return instance;
		}

	private:
		instrumentor()
			: m_current_session(nullptr) {}

		~instrumentor() {
			
			end_session();
		};

		void write_header() {

			m_output_stream << "{\"otherData\": {},\"traceEvents\":[{}";
			m_output_stream.flush();
		}

		void write_footer() {

			m_output_stream << "]}";
			m_output_stream.flush();
		}


		// NOTE: you must already own lock on m_Mutex before calling internal_end_session()
		void internal_end_session() {
			if (m_current_session) {

				write_footer();
				m_output_stream.close();
				delete m_current_session;
				m_current_session = nullptr;
			}
		}

	private:
		std::mutex m_mutex;
		instumentation_session* m_current_session;
		std::ofstream m_output_stream;

	};

	// ==================================================================== instrumentor_timer ====================================================================

	class instrumentor_timer {
	public:

		instrumentor_timer(const char* name)
			: m_name(name), m_stopped(false) {

			m_start_timepoint = std::chrono::steady_clock::now();
		}

		~instrumentor_timer() {

			if (!m_stopped)
				stop();
		}

		void stop() {

			auto end_timepoint = std::chrono::steady_clock::now();
			auto high_res_start = float_microseconds{ m_start_timepoint.time_since_epoch() };
			auto elapsed_time = std::chrono::time_point_cast<std::chrono::microseconds>(end_timepoint).time_since_epoch() - std::chrono::time_point_cast<std::chrono::microseconds>(m_start_timepoint).time_since_epoch();

			instrumentor::get().write_profile({ m_name, high_res_start, elapsed_time, std::this_thread::get_id() });
			m_stopped = true;
		}

	private:

		const char* m_name;
		bool m_stopped;
		std::chrono::time_point<std::chrono::steady_clock> m_start_timepoint;

	};



// ==================================== profiler ENABLED ====================================
#if PROFILE

	// Resolve which function signature macro will be used
	// NOTE: this is only resolved when the (pre)compiler starts, so the syntax highlighting could mark the wrong one in editor!
	#if defined(__GNUC__) || (defined(__MWERKS__) && (__MWERKS__ >= 0x3000)) || (defined(__ICC) && (__ICC >= 600)) || defined(__ghs__)
		#define FUNC_SIG __PRETTY_FUNCTION__
	#elif defined(__DMC__) && (__DMC__ >= 0x810)
		#define FUNC_SIG __PRETTY_FUNCTION__
	#elif (defined(__FUNCSIG__) || (_MSC_VER))
		#define FUNC_SIG __FUNCSIG__
	#elif (defined(__INTEL_COMPILER) && (__INTEL_COMPILER >= 600)) || (defined(__IBMCPP__) && (__IBMCPP__ >= 500))
		#define FUNC_SIG __FUNCTION__
	#elif defined(__BORLANDC__) && (__BORLANDC__ >= 0x550)
		#define FUNC_SIG __FUNC__
	#elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901)
		#define FUNC_SIG __func__
	#elif defined(__cplusplus) && (__cplusplus >= 201103)
		#define FUNC_SIG __func__
	#else
		#define FUNC_SIG "FUNC_SIG unknown!"
	#endif

	// double abstraction to take line-number into name string
	#define PROFILE_SCOPE_LINE2(name, line)						constexpr auto fixedName##line = ::util::remove_substring(name, "__cdecl ");		\
																	instrumentor_timer benchmark_timer##line(fixedName##line.data)
	#define PROFILE_SCOPE_LINE(name, line)						PROFILE_SCOPE_LINE2(name, line)

	#define PROFILE_BEGIN_SESSION(name, directory, filename)	instrumentor::get().begin_session(name, directory, filename)
	#define PROFILE_END_SESSION()								instrumentor::get().end_session()
	#define PROFILE_SCOPE(name)									PROFILE_SCOPE_LINE(name, __LINE__)
	#define PROFILE_FUNCTION()									PROFILE_SCOPE(FUNC_SIG)

// ------------------------------------ subsystem: renderer ------------------------------------ 
#if PROFILE_RENDERER
	#define PROFILE_RENDER_SCOPE(name)									PROFILE_SCOPE_LINE(name, __LINE__)
	#define PROFILE_RENDER_FUNCTION()									PROFILE_SCOPE(FUNC_SIG)
#else
	#define PROFILE_RENDER_SCOPE(name)
	#define PROFILE_RENDER_FUNCTION()
#endif

// ==================================== profiler DISABLED ====================================
#else
	#define PROFILE_BEGIN_SESSION(name, directory, filename)
	#define PROFILE_END_SESSION()
	#define PROFILE_SCOPE(name)
	#define PROFILE_FUNCTION()

// ------------------------------------ subsystem: renderer ------------------------------------ 
	#define PROFILE_RENDER_SCOPE(name)
	#define PROFILE_RENDER_FUNCTION()

#endif	// PROFILE

}
