#pragma once

#include <pybricks-driver/types.h>

#include <simpleble/Types.h>

#include <string>
#include <vector>
#include <variant>
#include <functional>
#include <optional>
#include <type_traits>


namespace pybricksDriver::messaging
{
	using CommandHandler = std::function<bool(const CommandInData&)>;

	std::vector<Event> consumeDataForEvent(SimpleBLE::ByteArray& buffer, SimpleBLE::ByteArray& data);
	std::vector<std::string> consumeDataForPrintableContent(SimpleBLE::ByteArray& buffer, SimpleBLE::ByteArray& data);
	std::optional<CommandInData> getCommandInDataFromEvent(const Event& event);

	template<typename T>
	requires std::is_signed_v<T>
	class Unmodulator
	{
		public:
			explicit Unmodulator(const T& maxVal);
			~Unmodulator();

			T operator()(const T& rawVal);

		private:
			T _maxVal;
			T _lastRawVal;
			bool _hasLastRawVal;
			int _wrapAroundCount;
	};

}


#include "messaging.hpp"