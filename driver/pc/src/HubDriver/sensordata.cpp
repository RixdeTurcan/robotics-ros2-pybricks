#include <pybricks-driver/types.h>
#include <pybricks-driver/logging.h>

#include "constant_internal.h"
#include "tools/tools.h"

#include <string>



namespace pybricksDriver
{
	namespace
	{
		logging::Logger& logger = logging::registerLogger("SensorData");

	}

	SensorData_HubIMU::SensorData_HubIMU(const uint8_t& type, const std::vector<uint8_t>& payload) {
		if (payload.size() < messaging::SENSOR_DATA_SIZE_HUB_IMU) {
			logger->warn("Sensor payload is expected to contain at least {} bytes of data, but received: {}", messaging::SENSOR_DATA_SIZE_HUB_IMU, tools::toHexString(payload));
			return;
		}

		ax = tools::readS16LE(payload, 0)  * convert::MM_S2_TO_M_S2;
		ay = tools::readS16LE(payload, 2)  * convert::MM_S2_TO_M_S2;
		az = tools::readS16LE(payload, 4)  * convert::MM_S2_TO_M_S2;

		wx = tools::readS16LE(payload, 6)  * convert::MRAD_TO_RAD;
		wy = tools::readS16LE(payload, 8)  * convert::MRAD_TO_RAD;
		wz = tools::readS16LE(payload, 10) * convert::MRAD_TO_RAD;

		rx = tools::readU16LE(payload, 12) * convert::BYTES2_TO_2PI_RAD;
		ry = tools::readU16LE(payload, 14) * convert::BYTES2_TO_2PI_RAD;
		rz = tools::readU16LE(payload, 16) * convert::BYTES2_TO_2PI_RAD;
	}

	SensorData_ForceSensor::SensorData_ForceSensor(const uint8_t& type, const std::vector<uint8_t>& payload) {
		if (payload.size() < messaging::SENSOR_DATA_SIZE_FORCE_SENSOR) {
			logger->warn("Sensor payload is expected to contain at least {} bytes of data, but received: {}", messaging::SENSOR_DATA_SIZE_FORCE_SENSOR, tools::toHexString(payload));
			return;
		}

		f = tools::readU16LE(payload, 0) * convert::BYTES2_TO_11N;
		d = payload[2] * convert::BYTES1_TO_0_008M;
		pressed = (payload[3] & 0x02) != 0;
		touched = (payload[3] & 0x01) != 0;
	}

	SensorData_ColorSensor::SensorData_ColorSensor(const uint8_t& type, const std::vector<uint8_t>& payload) {
		if (payload.size() < messaging::SENSOR_DATA_SIZE_COLOR_SENSOR) {
			logger->warn("Sensor payload is expected to contain at least {} bytes of data, but received: {}", messaging::SENSOR_DATA_SIZE_COLOR_SENSOR, tools::toHexString(payload));
			return;
		}

		h = tools::readU16LE(payload, 0);
		s = payload[2];
		v = payload[3];
		reflection = payload[4];
		ambiant = payload[5];
	}

	SensorData_UltrasonicSensor::SensorData_UltrasonicSensor(const uint8_t& type, const std::vector<uint8_t>& payload) {
		if (payload.size() < messaging::SENSOR_DATA_SIZE_ULTRASONIC_SENSOR) {
			logger->warn("Sensor payload is expected to contain at least {} bytes of data, but received: {}", messaging::SENSOR_DATA_SIZE_ULTRASONIC_SENSOR, tools::toHexString(payload));
			return;
		}

		const uint16_t d_mm = tools::readU16LE(payload, 0);

		d = d_mm * convert::MM_TO_M;
		isInf = d_mm == 0xFFFF;
	}

	SensorData_Motor::SensorData_Motor(const uint8_t& type, const std::vector<uint8_t>& payload) {
		if (payload.size() < messaging::SENSOR_DATA_SIZE_MOTOR) {
			logger->warn("Sensor payload is expected to contain at least {} bytes of data, but received: {}", messaging::SENSOR_DATA_SIZE_MOTOR, tools::toHexString(payload));
			return;
		}

		r = tools::readU16LE(payload, 0) * convert::DEG_TO_RAD * 0.01;
		v = tools::readS16LE(payload, 2) * convert::DEG_TO_RAD * 0.01;
		t = tools::readS16LE(payload, 4) * convert::MM_TO_M * 0.01;
	}

	SensorData_MediumMotor::SensorData_MediumMotor(const uint8_t& type, const std::vector<uint8_t>& payload)
		:SensorData_Motor(type, payload)
	{}

	SensorData_LargeMotor::SensorData_LargeMotor(const uint8_t& type, const std::vector<uint8_t>& payload)
		:SensorData_Motor(type, payload)
	{}

	SensorData_HubButtons::SensorData_HubButtons(const uint8_t& type, const std::vector<uint8_t>& payload) {
		if (payload.size() < messaging::SENSOR_DATA_SIZE_HUB_BUTTONS) {
			logger->warn("Sensor payload is expected to contain at least {} bytes of data, but received: {}", messaging::SENSOR_DATA_SIZE_HUB_BUTTONS, tools::toHexString(payload));
			return;
		}

		center = (payload[0] & 0x01) != 0;
		left = (payload[0] & 0x02) != 0;
		right = (payload[0] & 0x04) != 0;
		bluetooth = (payload[0] & 0x08) != 0;
	}

	SensorData_HubSystem::SensorData_HubSystem(const uint8_t& type, const std::vector<uint8_t>& payload) {
		if (payload.size() < messaging::SENSOR_DATA_SIZE_HUB_SYSTEM) {
			logger->warn("Sensor payload is expected to contain at least {} bytes of data, but received: {}", messaging::SENSOR_DATA_SIZE_HUB_SYSTEM, tools::toHexString(payload));
			return;
		}

		capacity = payload[0] * convert::BYTES1_TO_100_PERCENT;
		current = payload[1] * convert::BYTES1_TO_1A;
	}

	SensorData_HubSpeaker::SensorData_HubSpeaker(const uint8_t& type, const std::vector<uint8_t>& payload) {}

	SensorData_HubLight55Matrix::SensorData_HubLight55Matrix(const uint8_t& type, const std::vector<uint8_t>& payload) {}

	SensorData_Light33Matrix::SensorData_Light33Matrix(const uint8_t& type, const std::vector<uint8_t>& payload) {}
}
