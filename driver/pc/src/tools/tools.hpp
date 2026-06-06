#pragma once

#include <format>
#include <type_traits>
#include <cstdint>

namespace pybricksDriver::tools
{
	template<typename T, typename R> //Byte array, result type
	R readLE(const T& b, const size_t offset, const size_t n) {
		static_assert(std::is_integral_v<R>, "R must be an integral type");
		static_assert(!std::is_same_v<R, bool>, "R must not be bool");

		if (offset + n > b.size()) {
			throw std::out_of_range(std::format("Offset out of range for reading {}", typeid(R).name()));
		}

		using U = std::make_unsigned_t<R>;
		U r = 0;
		for (size_t i = 0; i < n; i++) {
			r |= static_cast<U>(b[offset+i]) << (8*i);
		}

		return static_cast<R>(r);
	}

	template<typename T>
	T mix(const T& a, const T& b, double factorA) {
		return static_cast<T>(a * factorA + b * (1.0 - factorA));
	}

	template<typename T>
	uint16_t readU16LE(const T& b, const size_t offset) {
		return readLE<T, uint16_t>(b, offset, 2);
	}

	template<typename T>
	int16_t readS16LE(const T& b, const size_t offset) {
		return readLE<T, int16_t>(b, offset, 2);
	}

	template<typename T>
	uint32_t readU32LE(const T& b, const size_t offset) {
		return readLE<T, uint32_t>(b, offset, 4);
	}

	template<typename T>
	int32_t readS32LE(const T& b, const size_t offset) {
		return readLE<T, int32_t>(b, offset, 4);
	}

	template<typename T>
	uint64_t readU64LE(const T& b, const size_t offset) {
		return readLE<T, uint64_t>(b, offset, 8);
	}

	template<typename T>
	int64_t readS64LE(const T& b, const size_t offset) {
		return readLE<T, int64_t>(b, offset, 8);
	}

}