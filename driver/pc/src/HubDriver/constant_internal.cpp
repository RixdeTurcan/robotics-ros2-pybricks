#include "constant_internal.h"

#include <cmath>


namespace pybricksDriver::messaging
{
	const std::string DEFAULT_DEVICE_NAME = "Pybricks Hub";
	const std::string DEFAULT_SERVICE_UUID = "c5f50001-8280-46da-89f4-6d8051e4aeef";
	const std::string DEFAULT_CHARACTERISTIC_UUID = "c5f50002-8280-46da-89f4-6d8051e4aeef";

	const uint8_t GATT_COMMAND_STOP_PROGRAM = 0x00;
	const uint8_t GATT_COMMAND_START_PROGRAM = 0x01;
	const uint8_t GATT_COMMAND_WRITE_TO_STDIN = 0x06;

	const uint8_t GATT_MESSAGE_STATUS = 0x00;
	const uint8_t GATT_MESSAGE_READ_FROM_STDOUT = 0x01;
	const uint8_t GATT_MESSAGE_READ_FROM_DATA = 0x02;

	const uint8_t MESSAGE_BOUNDARY_START = 0xFF;
	const uint8_t MESSAGE_BOUNDARY_ESCAPE = 0xFE;
	const uint8_t MESSAGE_BOUNDARY_ESCAPE_START = 0xFD;

	const uint8_t MESSAGE_PRINT_END_0 = 0x0D;
	const uint8_t MESSAGE_PRINT_END_1 = 0x0A;

	const uint8_t MESSAGE_OUT_COMMAND_PING = 0x00;
	const uint8_t MESSAGE_OUT_COMMAND_START_LISTENING = 0x01;
	const uint8_t MESSAGE_OUT_COMMAND_STOP_LISTENING = 0x02;
	const uint8_t MESSAGE_OUT_COMMAND_STOP_ALL_LISTENING = 0x03;
	const uint8_t MESSAGE_OUT_COMMAND_GET_CAPABILITIES = 0x04;
	const uint8_t MESSAGE_OUT_COMMAND_TIMESYNC = 0x05;
	const uint8_t MESSAGE_OUT_COMMAND_ACTUATOR = 0x06;


	const uint8_t MESSAGE_IN_COMMAND_RESPONSE = 0x00;
	const uint8_t MESSAGE_IN_COMMAND_DATA = 0x01;
	const uint8_t MESSAGE_IN_COMMAND_HUB_READY = 0x02;

	const uint16_t FLAG_HUB_IMU = 0x0004;
	const uint16_t FLAG_HUB_BUTTONS = 0x0008;
	const uint16_t FLAG_HUB_SYSTEM = 0x0010;
	const uint16_t FLAG_HUB_SPEAKER = 0x0020;
	const uint16_t FLAG_HUB_LIGHT_55_MATRIX = 0x0040;
	const uint16_t FLAG_SENSOR_A = 0x0080;
	const uint16_t FLAG_SENSOR_B = 0x0100;
	const uint16_t FLAG_SENSOR_C = 0x0200;
	const uint16_t FLAG_SENSOR_D = 0x0400;
	const uint16_t FLAG_SENSOR_E = 0x0800;
	const uint16_t FLAG_SENSOR_F = 0x1000;

	const size_t SENSOR_DATA_SIZE_NONE = 0;
	const size_t SENSOR_DATA_SIZE_HUB_IMU = 2*3*3;
	const size_t SENSOR_DATA_SIZE_FORCE_SENSOR = 2+1+1;
	const size_t SENSOR_DATA_SIZE_COLOR_SENSOR = 2+1+1+1+1;
	const size_t SENSOR_DATA_SIZE_ULTRASONIC_SENSOR = 2;
	const size_t SENSOR_DATA_SIZE_LIGHT_33_MATRIX = 0;
	const size_t SENSOR_DATA_SIZE_MOTOR = 2*3;
	const size_t SENSOR_DATA_SIZE_HUB_BUTTONS = 1;
	const size_t SENSOR_DATA_SIZE_HUB_SYSTEM = 1+1;
	const size_t SENSOR_DATA_SIZE_HUB_SPEAKER = 0;
	const size_t SENSOR_DATA_SIZE_HUB_LIGHT_55_MATRIX = 0;
}

namespace pybricksDriver::convert
{
	const double MM_S2_TO_M_S2 = 0.001;
	const double MRAD_TO_RAD = 0.001;
	const double BYTES2_TO_2PI_RAD = 2. * M_PI / 65535.0;
	const double BYTES1_TO_100_PERCENT = 100.0 / 255.0;
	const double BYTES1_TO_1A = 1. / 255.0;
	const double BYTES2_TO_11N = 11. / 65535.0;
	const double BYTES1_TO_0_008M = 0.008 / 255.0;
	const double MM_TO_M = 0.001;
	const double DEG_TO_RAD = 2. * M_PI / 360.;
	const double RAD_TO_DEG = 360. / (2. * M_PI);
}