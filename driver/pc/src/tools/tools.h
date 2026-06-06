#pragma once

#include <simpleble/SimpleBLE.h>

#include <vector>
#include <string>
#include <cstdint>


namespace pybricksDriver::tools
{
	uint8_t crc8(const SimpleBLE::ByteArray& data, uint8_t crc = 0x00);

	template<typename T>
	uint16_t readU16LE(const T& b, size_t offset = 0);

	template<typename T>
	int16_t readS16LE(const T& b, size_t offset = 0);

	template<typename T>
	uint32_t readU32LE(const T& b, size_t offset = 0);

	template<typename T>
	int32_t readS32LE(const T& b, size_t offset = 0);

	template<typename T>
	uint64_t readU64LE(const T& b, size_t offset = 0);

	template<typename T>
	int64_t readS64LE(const T& b, size_t offset = 0);

	std::vector<uint8_t> splitU16LE(uint16_t value);
	std::vector<uint8_t> splitU32LE(uint32_t value);
	std::vector<uint8_t> splitU64LE(uint64_t value);

	template<typename T>
	T mix(const T& a, const T& b, double factorA);

	std::string toHexString(const std::vector<uint8_t>& v);

	size_t getSensorDataSize(uint8_t sensorType);
}

#include "tools.hpp"