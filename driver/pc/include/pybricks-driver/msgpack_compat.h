#pragma once

#ifdef PYBRICKS_DRIVER_WITHOUT_MSGPACK
	#define PYBRICKS_MSGPACK_DEFINE(...)
	#define PYBRICKS_MSGPACK_BASE(...)
#else
	#include <msgpack.hpp>
	#define PYBRICKS_MSGPACK_DEFINE(...) MSGPACK_DEFINE(__VA_ARGS__)
	#define PYBRICKS_MSGPACK_BASE(...) MSGPACK_BASE(__VA_ARGS__)
#endif
