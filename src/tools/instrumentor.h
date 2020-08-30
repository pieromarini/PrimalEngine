#include <string>
#include <chrono>
#include <algorithm>
#include <fstream>

#include <thread>

namespace primal {

  struct ProfileResult {
	std::string name;
	long long start, end;
	uint32_t threadID;
  };

  struct InstrumentationSession {
	std::string name;
  };

  class Instrumentor {
	public:
	  Instrumentor() : m_currentSession(nullptr), m_profileCount(0) { }

	  void beginSession(const std::string& name, const std::string& filepath = "results.json") {
		m_outputStream.open(filepath);
		writeHeader();
		m_currentSession = new InstrumentationSession{name};
	  }

	  void endSession() {
		writerFooter();
		m_outputStream.close();
		delete m_currentSession;
		m_currentSession = nullptr;
		m_profileCount = 0;
	  }

	  void writeProfile(const ProfileResult& result) {
		if (m_profileCount++ > 0)
		  m_outputStream << ",";

		std::string name = result.name;
		std::replace(name.begin(), name.end(), '"', '\'');

		m_outputStream << "{";
		m_outputStream << "\"cat\":\"function\",";
		m_outputStream << "\"dur\":" << (result.end - result.start) << ',';
		m_outputStream << "\"name\":\"" << name << "\",";
		m_outputStream << "\"ph\":\"X\",";
		m_outputStream << "\"pid\":0,";
		m_outputStream << "\"tid\":" << result.threadID << ",";
		m_outputStream << "\"ts\":" << result.start;
		m_outputStream << "}";

		m_outputStream.flush();
	  }

	  void writeHeader() {
		m_outputStream << "{\"otherData\": {},\"traceEvents\":[";
		m_outputStream.flush();
	  }

	  void writerFooter() {
		m_outputStream << "]}";
		m_outputStream.flush();
	  }

	  static Instrumentor& get() {
		static Instrumentor instance;
		return instance;
	  }

	private:
	  InstrumentationSession* m_currentSession;
	  std::ofstream m_outputStream;
	  int m_profileCount;
  };

  class InstrumentationTimer {
	public:
	  InstrumentationTimer(const char* name) : m_name(name), m_stopped(false) {
		m_startTimepoint = std::chrono::high_resolution_clock::now();
	  }

	  ~InstrumentationTimer() {
		if (!m_stopped)
		  stop();
	  }

	  void stop() {
		auto endTimepoint = std::chrono::high_resolution_clock::now();

		long long start = std::chrono::time_point_cast<std::chrono::microseconds>(m_startTimepoint).time_since_epoch().count();
		long long end = std::chrono::time_point_cast<std::chrono::microseconds>(endTimepoint).time_since_epoch().count();

		uint32_t threadID = std::hash<std::thread::id>{}(std::this_thread::get_id());
		Instrumentor::get().writeProfile({ m_name, start, end, threadID });

		m_stopped = true;
	  }
	private:
	  const char* m_name;
	  bool m_stopped;
	  std::chrono::time_point<std::chrono::high_resolution_clock> m_startTimepoint;
  };
}

#define PRIMAL_PROFILE 1
#if PRIMAL_PROFILE
	#if defined(__GNUC__) || (defined(__MWERKS__) && (__MWERKS__ >= 0x3000)) || (defined(__ICC) && (__ICC >= 600)) || defined(__ghs__)
		#define PRIMAL_FUNC_SIG __PRETTY_FUNCTION__
	#elif defined(__DMC__) && (__DMC__ >= 0x810)
		#define PRIMAL_FUNC_SIG __PRETTY_FUNCTION__
	#elif defined(__FUNCSIG__)
		#define PRIMAL_FUNC_SIG __FUNCSIG__
	#elif (defined(__INTEL_COMPILER) && (__INTEL_COMPILER >= 600)) || (defined(__IBMCPP__) && (__IBMCPP__ >= 500))
		#define PRIMAL_FUNC_SIG __FUNCTION__
	#elif defined(__BORLANDC__) && (__BORLANDC__ >= 0x550)
		#define PRIMAL_FUNC_SIG __FUNC__
	#elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901)
		#define PRIMAL_FUNC_SIG __func__
	#elif defined(__cplusplus) && (__cplusplus >= 201103)
		#define PRIMAL_FUNC_SIG __func__
	#else
		#define PRIMAL_FUNC_SIG "PRIMAL_FUNC_SIG unknown!"
	#endif

	#define PRIMAL_PROFILE_BEGIN_SESSION(name, filepath) ::primal::Instrumentor::get().beginSession(name, filepath)
	#define PRIMAL_PROFILE_END_SESSION() ::primal::Instrumentor::get().endSession()
	#define PRIMAL_PROFILE_SCOPE(name) ::primal::InstrumentationTimer timer##__LINE__(name);
	#define PRIMAL_PROFILE_FUNCTION() PRIMAL_PROFILE_SCOPE(PRIMAL_FUNC_SIG)
#else
	#define PRIMAL_PROFILE_BEGIN_SESSION(name, filepath)
	#define PRIMAL_PROFILE_END_SESSION()
	#define PRIMAL_PROFILE_SCOPE(name)
	#define PRIMAL_PROFILE_FUNCTION()
#endif
