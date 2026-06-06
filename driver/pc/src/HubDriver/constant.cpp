#include <pybricks-driver/constant.h>

namespace pybricksDriver
{
	const uint8_t SENSOR_TYPE_NONE = 0x0;
	const uint8_t SENSOR_TYPE_HUB_IMU = 0x1;
	const uint8_t SENSOR_TYPE_FORCE_SENSOR = 0x2;
	const uint8_t SENSOR_TYPE_COLOR_SENSOR = 0x3;
	const uint8_t SENSOR_TYPE_ULTRASONIC_SENSOR = 0x4;
	const uint8_t SENSOR_TYPE_LIGHT_33_MATRIX = 0x5;
	const uint8_t SENSOR_TYPE_MEDIUM_MOTOR = 0x6;
	const uint8_t SENSOR_TYPE_LARGE_MOTOR = 0x7;
	const uint8_t SENSOR_TYPE_HUB_BUTTONS = 0x8;
	const uint8_t SENSOR_TYPE_HUB_SYSTEM = 0x9;
	const uint8_t SENSOR_TYPE_HUB_SPEAKER = 0xA;
	const uint8_t SENSOR_TYPE_HUB_LIGHT_55_MATRIX = 0xB;

	const uint8_t SENSOR_LOCATION_HUB = 0x0;
	const uint8_t SENSOR_LOCATION_A = 0x1;
	const uint8_t SENSOR_LOCATION_B = 0x2;
	const uint8_t SENSOR_LOCATION_C = 0x3;
	const uint8_t SENSOR_LOCATION_D = 0x4;
	const uint8_t SENSOR_LOCATION_E = 0x5;
	const uint8_t SENSOR_LOCATION_F = 0x6;

	std::string getSensorLocationName(const uint8_t sensorLocation) {
		if (sensorLocation == SENSOR_LOCATION_HUB) return std::string{"HUB"};
		if (sensorLocation == SENSOR_LOCATION_A) return std::string{"A"};
		if (sensorLocation == SENSOR_LOCATION_B) return std::string{"B"};
		if (sensorLocation == SENSOR_LOCATION_C) return std::string{"C"};
		if (sensorLocation == SENSOR_LOCATION_D) return std::string{"D"};
		if (sensorLocation == SENSOR_LOCATION_E) return std::string{"E"};
		if (sensorLocation == SENSOR_LOCATION_F) return std::string{"F"};
		return std::string{"?"};
	}

	std::string getSensorTypeName(const uint8_t sensorType) {
		if (sensorType == SENSOR_TYPE_HUB_IMU) return std::string{"Hub IMU"};
		if (sensorType == SENSOR_TYPE_HUB_BUTTONS) return std::string{"Hub buttons"};
		if (sensorType == SENSOR_TYPE_HUB_SYSTEM) return std::string{"Hub system"};
		if (sensorType == SENSOR_TYPE_HUB_SPEAKER) return std::string{"Hub speaker"};
		if (sensorType == SENSOR_TYPE_HUB_LIGHT_55_MATRIX) return std::string{"Hub light 55 matrix"};
		if (sensorType == SENSOR_TYPE_FORCE_SENSOR) return std::string{"Force sensor"};
		if (sensorType == SENSOR_TYPE_COLOR_SENSOR) return std::string{"Color sensor"};
		if (sensorType == SENSOR_TYPE_ULTRASONIC_SENSOR) return std::string{"Ultrasonic sensor"};
		if (sensorType == SENSOR_TYPE_LIGHT_33_MATRIX) return std::string{"Light 33 matrix"};
		if (sensorType == SENSOR_TYPE_MEDIUM_MOTOR) return std::string{"Medium motor"};
		if (sensorType == SENSOR_TYPE_LARGE_MOTOR) return std::string{"Large motor"};
		if (sensorType == SENSOR_TYPE_NONE) return std::string{"/No sensor/"};
		return std::string{"/Unknown sensor/"};
	}

	std::string getSensorTypeSafeName(const uint8_t sensorType) {
		if (sensorType == SENSOR_TYPE_HUB_IMU) return std::string{"Hub_IMU"};
		if (sensorType == SENSOR_TYPE_HUB_BUTTONS) return std::string{"Hub_buttons"};
		if (sensorType == SENSOR_TYPE_HUB_SYSTEM) return std::string{"Hub_system"};
		if (sensorType == SENSOR_TYPE_HUB_SPEAKER) return std::string{"Hub_speaker"};
		if (sensorType == SENSOR_TYPE_HUB_LIGHT_55_MATRIX) return std::string{"Hub_light_55_matrix"};
		if (sensorType == SENSOR_TYPE_FORCE_SENSOR) return std::string{"Force_sensor"};
		if (sensorType == SENSOR_TYPE_COLOR_SENSOR) return std::string{"Color_sensor"};
		if (sensorType == SENSOR_TYPE_ULTRASONIC_SENSOR) return std::string{"Ultrasonic_sensor"};
		if (sensorType == SENSOR_TYPE_LIGHT_33_MATRIX) return std::string{"Light_33_matrix"};
		if (sensorType == SENSOR_TYPE_MEDIUM_MOTOR) return std::string{"Medium_motor"};
		if (sensorType == SENSOR_TYPE_LARGE_MOTOR) return std::string{"Large_motor"};
		if (sensorType == SENSOR_TYPE_NONE) return std::string{"No_sensor"};
		return std::string{"Unknown_sensor"};
	}
}
