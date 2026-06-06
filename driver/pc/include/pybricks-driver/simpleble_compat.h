#pragma once

#ifdef PYBRICKS_DRIVER_WITHOUT_SIMPLEBLE
	namespace kvn
	{
		class bytearray;
	}
	namespace SimpleBLE
	{
		using ByteArray = kvn::bytearray;
	}
#else
	#include <simpleble/Types.h>
#endif


namespace pybricksDriver{
	using CommandAndData = std::tuple<uint8_t, SimpleBLE::ByteArray>;

	struct Event{
		uint8_t command{};
		SimpleBLE::ByteArray data{};
	};
}