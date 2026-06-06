#pragma once

#include <cstdint>
#include <string>

namespace pybricksDriver
{
	extern const uint8_t SENSOR_TYPE_NONE;
	extern const uint8_t SENSOR_TYPE_HUB_IMU;
	extern const uint8_t SENSOR_TYPE_FORCE_SENSOR;
	extern const uint8_t SENSOR_TYPE_COLOR_SENSOR;
	extern const uint8_t SENSOR_TYPE_ULTRASONIC_SENSOR;
	extern const uint8_t SENSOR_TYPE_LIGHT_33_MATRIX;
	extern const uint8_t SENSOR_TYPE_MEDIUM_MOTOR;
	extern const uint8_t SENSOR_TYPE_LARGE_MOTOR;
	extern const uint8_t SENSOR_TYPE_HUB_BUTTONS;
	extern const uint8_t SENSOR_TYPE_HUB_SYSTEM;
	extern const uint8_t SENSOR_TYPE_HUB_SPEAKER;
	extern const uint8_t SENSOR_TYPE_HUB_LIGHT_55_MATRIX;

	extern const uint8_t SENSOR_LOCATION_HUB;
	extern const uint8_t SENSOR_LOCATION_A;
	extern const uint8_t SENSOR_LOCATION_B;
	extern const uint8_t SENSOR_LOCATION_C;
	extern const uint8_t SENSOR_LOCATION_D;
	extern const uint8_t SENSOR_LOCATION_E;
	extern const uint8_t SENSOR_LOCATION_F;

	std::string getSensorLocationName(uint8_t sensorLocation);
	std::string getSensorTypeName(uint8_t sensorType);
	std::string getSensorTypeSafeName(uint8_t sensorType);
}