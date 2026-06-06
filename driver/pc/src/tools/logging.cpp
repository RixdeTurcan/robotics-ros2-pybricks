#include <pybricks-driver/logging.h>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/async.h>

#include <tuple>

namespace pybricksDriver::logging
{
	namespace
	{
		std::string _logPath;
		std::vector<std::tuple<std::shared_ptr<Logger>, std::string>> registeredLoggers;
		bool initialized = false;
	}

	void initLogging(const std::string& logPath){
		spdlog::init_thread_pool(8192, 1);
		_logPath = logPath;
		for (auto& [loggerWrapper, loggerName]: registeredLoggers) {
			*loggerWrapper = createLogger(loggerName);
		}
		initialized = true;
	}

	Logger& registerLogger(const std::string& loggerName) {
		Logger logger;
		if (initialized) {
			logger = createLogger(loggerName);
		}
		auto loggerWrapper = std::make_shared<Logger>(logger);
		registeredLoggers.emplace_back(loggerWrapper, loggerName);
		return *std::get<0>(registeredLoggers.back());
	}

	std::shared_ptr<spdlog::sinks::sink> _getSink() {
		static auto sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(_logPath, 1024*1024*10, 10, false);
		return sink;
	}

	Logger createLogger(const std::string& loggerName) {
		auto sink = _getSink();
		const auto logger = std::make_shared<spdlog::async_logger>(loggerName, sink, spdlog::thread_pool(), spdlog::async_overflow_policy::block);
		logger->set_pattern("%Y-%m-%d %H:%M:%S.%f - Robotics - %n - %l - . - . - %v", spdlog::pattern_time_type::utc);
		logger->set_level(spdlog::level::trace);
		logger->flush_on(spdlog::level::trace);
		spdlog::register_logger(logger);
		return logger;
	}

	void deleteLogger(const std::string& loggerName) {
		spdlog::drop(loggerName);
	}

	void deleteAllLoggers() {
		spdlog::drop_all();
		spdlog::shutdown();
	}

}