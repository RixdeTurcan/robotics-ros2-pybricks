#pragma once

#include <string>
#include <cstdint>


namespace pybricksDriver::messaging
{
	extern const std::string DEFAULT_DEVICE_NAME;
	extern const std::string DEFAULT_SERVICE_UUID;
	extern const std::string DEFAULT_CHARACTERISTIC_UUID;

	extern const uint8_t GATT_COMMAND_STOP_PROGRAM;
	extern const uint8_t GATT_COMMAND_START_PROGRAM;
	extern const uint8_t GATT_COMMAND_WRITE_TO_STDIN;

	extern const uint8_t GATT_MESSAGE_STATUS;
	extern const uint8_t GATT_MESSAGE_READ_FROM_STDOUT;
	extern const uint8_t GATT_MESSAGE_READ_FROM_DATA;

	extern const uint8_t MESSAGE_BOUNDARY_START;
	extern const uint8_t MESSAGE_BOUNDARY_ESCAPE;
	extern const uint8_t MESSAGE_BOUNDARY_ESCAPE_START;

	extern const uint8_t MESSAGE_PRINT_END_0;
	extern const uint8_t MESSAGE_PRINT_END_1;

	extern const uint8_t MESSAGE_OUT_COMMAND_PING;
	extern const uint8_t MESSAGE_OUT_COMMAND_START_LISTENING;
	extern const uint8_t MESSAGE_OUT_COMMAND_STOP_LISTENING;
	extern const uint8_t MESSAGE_OUT_COMMAND_STOP_ALL_LISTENING;
	extern const uint8_t MESSAGE_OUT_COMMAND_GET_CAPABILITIES;
	extern const uint8_t MESSAGE_OUT_COMMAND_TIMESYNC;
	extern const uint8_t MESSAGE_OUT_COMMAND_ACTUATOR;

	extern const uint8_t MESSAGE_IN_COMMAND_RESPONSE;
	extern const uint8_t MESSAGE_IN_COMMAND_DATA;
	extern const uint8_t MESSAGE_IN_COMMAND_HUB_READY;

	extern const uint16_t FLAG_HUB_IMU;
	extern const uint16_t FLAG_HUB_BUTTONS;
	extern const uint16_t FLAG_HUB_SYSTEM;
	extern const uint16_t FLAG_HUB_SPEAKER;
	extern const uint16_t FLAG_HUB_LIGHT_55_MATRIX;
	extern const uint16_t FLAG_SENSOR_A;
	extern const uint16_t FLAG_SENSOR_B;
	extern const uint16_t FLAG_SENSOR_C;
	extern const uint16_t FLAG_SENSOR_D;
	extern const uint16_t FLAG_SENSOR_E;
	extern const uint16_t FLAG_SENSOR_F;

	extern const size_t SENSOR_DATA_SIZE_NONE;
	extern const size_t SENSOR_DATA_SIZE_HUB_IMU;
	extern const size_t SENSOR_DATA_SIZE_FORCE_SENSOR;
	extern const size_t SENSOR_DATA_SIZE_COLOR_SENSOR;
	extern const size_t SENSOR_DATA_SIZE_ULTRASONIC_SENSOR;
	extern const size_t SENSOR_DATA_SIZE_LIGHT_33_MATRIX;
	extern const size_t SENSOR_DATA_SIZE_MOTOR;
	extern const size_t SENSOR_DATA_SIZE_HUB_BUTTONS;
	extern const size_t SENSOR_DATA_SIZE_HUB_SYSTEM;
	extern const size_t SENSOR_DATA_SIZE_HUB_SPEAKER;
	extern const size_t SENSOR_DATA_SIZE_HUB_LIGHT_55_MATRIX;
}

namespace pybricksDriver::convert
{
	extern const double MM_S2_TO_M_S2;
	extern const double MRAD_TO_RAD;
	extern const double BYTES2_TO_2PI_RAD;
	extern const double BYTES1_TO_100_PERCENT;
	extern const double BYTES1_TO_1A;
	extern const double BYTES2_TO_11N;
	extern const double BYTES1_TO_0_008M;
	extern const double MM_TO_M;
	extern const double DEG_TO_RAD;
	extern const double RAD_TO_DEG;
}