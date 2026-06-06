#pragma once

#include <spdlog/logger.h>

namespace pybricksDriver::logging
{
	using Logger = std::shared_ptr<spdlog::logger>;

	void initLogging(const std::string& logPath);

	Logger& registerLogger(const std::string& loggerName);

	Logger createLogger(const std::string& loggerName);

	void deleteLogger(const std::string& loggerName);

	void deleteAllLoggers();

}