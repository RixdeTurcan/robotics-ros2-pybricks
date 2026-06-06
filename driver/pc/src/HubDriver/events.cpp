#include <pybricks-driver/types.h>

#include "constant_internal.h"
#include "tools/tools.h"
#include "../../include/pybricks-driver/logging.h"

#include <format>



namespace pybricksDriver
{
	namespace
	{
		logging::Logger& logger = logging::registerLogger("Events");
	}

	int CommandResponse_InData::logLevel() const {return PYBRICKS_LOG_LEVEL_DEBUG;}
	CommandResponse_InData::CommandResponse_InData()
		:success(false)
	{}
	void CommandResponse_InData::printTitle(std::string& print) const {
		print += "IN  Response";
	}
	void CommandResponse_InData::printData(std::string& print) const {
		Data_Base::printData(print);

		if (success) {
			print += "OK ";
		}else {
			print += "NOK";
		}
	}
	void CommandResponse_InData::fromRawEvent(const Event& event) {
		if (event.data.size() < 2) {
			logger->warn("Command response should contain at least 2 bytes of data, but received: {}", event.data.toHex(true));
			success = false;
			return;
		}
		success = event.data[1] == 0x00;
	}


	int HubReady_InData::logLevel() const {return PYBRICKS_LOG_LEVEL_TRACE;}
	void HubReady_InData::printTitle(std::string& print) const {
		print += "IN  Hub ready";
	}
	void HubReady_InData::printData(std::string& print) const {
		Data_Base::printData(print);
	}
	void HubReady_InData::fromRawEvent(const Event& event) {
	}


	int CommandResponse_Ping_InData::logLevel() const {return PYBRICKS_LOG_LEVEL_TRACE;}
	void CommandResponse_Ping_InData::printTitle(std::string& print) const {
		CommandResponse_InData::printTitle(print);
		print += " Ping";
	}
	void CommandResponse_Ping_InData::printData(std::string& print) const {
		CommandResponse_InData::printData(print);
		Ping_OutData::printData(print);
	}
	void CommandResponse_Ping_InData::fromRawEvent(const Event& event) {
		CommandResponse_InData::fromRawEvent(event);
	}


	int CommandResponse_TimeSync_InData::logLevel() const {return PYBRICKS_LOG_LEVEL_DEBUG;}
	CommandResponse_TimeSync_InData::CommandResponse_TimeSync_InData()
		:hubTimestampNs(0)
		,syncId(0)
	{}
	void CommandResponse_TimeSync_InData::printTitle(std::string& print) const {
		CommandResponse_InData::printTitle(print);
		print += " Time sync";
	}
	void CommandResponse_TimeSync_InData::printData(std::string& print) const {
		CommandResponse_InData::printData(print);
		TimeSync_OutData::printData(print);
		print += std::format(" / Sync id: {} / hub timestamp (ns) {}", syncId, hubTimestampNs);
	}
	void CommandResponse_TimeSync_InData::fromRawEvent(const Event& event) {
		CommandResponse_InData::fromRawEvent(event);

		if (event.data.size() < 12) {
			logger->warn("Time sync command response should contain at least 14 bytes of data, but received: {}", event.data.toHex(true));
			success = false;
			return;
		}

		pcTimestampNs = tools::readU64LE(event.data, 2);
		hubTimestampNs = static_cast<uint64_t>(tools::readU16LE(event.data, 10)) * 1000000;
	}


	int Data_InData::logLevel() const {return PYBRICKS_LOG_LEVEL_DEBUG;}
	Data_InData::Data_InData()
		:hubTimestampNs(0)
		,syncId(0)
	{}
	void Data_InData::printTitle(std::string& print) const {
		print += " Hub data";
	}
	void Data_InData::printData(std::string& print) const {
		StartListening_OutData::printData(print);
		print += std::format(" - Sync id: {}\n", syncId);

		for (const auto& [sensorType, sensorLocation, sensorPayload] : sensorsPayload) {
			print += std::format("\t{}: {}\n", getSensorLocationName(sensorLocation), getSensorTypeName(sensorType));

			if (sensorType == SENSOR_TYPE_HUB_IMU) {
				const SensorData_HubIMU data(sensorType, sensorPayload);
				print += std::format("\t\tIMU acc (m/s²) : {:+.2f} {:+.2f} {:+.2f}\n", data.ax, data.ay, data.az);
				print += std::format("\t\tIMU gyr (rad/s): {:+.2f} {:+.2f} {:+.2f}\n", data.wx, data.wy, data.wz);
				print += std::format("\t\tIMU rot (rad)  : {:+.2f} {:+.2f} {:+.2f}\n", data.rx, data.ry, data.rz);

			}else if (sensorType == SENSOR_TYPE_HUB_SYSTEM) {
				const SensorData_HubSystem data(sensorType, sensorPayload);
				print += std::format("\t\tBattery (%) : {}", data.capacity);
				print += std::format("\t\tBattery (mA)  : {}", data.current*1000);

			}else if (sensorType == SENSOR_TYPE_HUB_BUTTONS) {
				const SensorData_HubButtons data(sensorType, sensorPayload);
				print += "\t\tButtons:";
				if (data.center) {
					print += " Center";
				}
				if (data.left) {
					print += " Left";
				}
				if (data.right) {
					print += " Right";
				}
				if (data.bluetooth) {
					print += " Bluetooth";
				}

			}else if (sensorType == SENSOR_TYPE_FORCE_SENSOR) {
				const SensorData_ForceSensor data(sensorType, sensorPayload);
				print += std::format("\t\tForce (N): {:.1f}\n", data.f);
				print += std::format("\t\tDistance (mm): {:.1f}\n", data.d*1000);
				print += std::format("\t\tStatus:");
				if (data.touched) {
					print += " Touched";
				}
				if (data.pressed) {
					print += " Pressed";
				}
				print += "\n";

			}else if (sensorType == SENSOR_TYPE_COLOR_SENSOR) {
				const SensorData_ColorSensor data(sensorType, sensorPayload);
				print += std::format("\t\tHue (°): {}\n", data.h);
				print += std::format("\t\tSaturation (%): {}\n", data.s);
				print += std::format("\t\tValue (%): {}\n", data.v);
				print += std::format("\t\tReflection (%): {}\n", data.reflection);
				print += std::format("\t\tAmbiant (%): {}\n", data.ambiant);

			}else if (sensorType == SENSOR_TYPE_ULTRASONIC_SENSOR) {
				const SensorData_UltrasonicSensor data(sensorType, sensorPayload);
				if (data.isInf) {
					print += "\t\tDistance (m): Infinity\n";
				}else {
					print += std::format("\t\tDistance (m): {:.2f}\n", data.d);
				}

			}else if (sensorType == SENSOR_TYPE_MEDIUM_MOTOR) {
				const SensorData_MediumMotor data(sensorType, sensorPayload);
				print += std::format("\t\tAngle (rad): {:.2f}\n", data.r);
				print += std::format("\t\tVelocity (rad/s): {:.2f}\n", data.v);
				print += std::format("\t\tTorque (mNm): {:.2f}\n", data.t*1000);

			}else if (sensorType == SENSOR_TYPE_LARGE_MOTOR) {
				const SensorData_LargeMotor data(sensorType, sensorPayload);
				print += std::format("\t\tAngle (rad): {:.2f}\n", data.r);
				print += std::format("\t\tVelocity (rad/s): {:.2f}\n", data.v);
				print += std::format("\t\tTorque (mNm): {:.2f}\n", data.t*1000);

			}
		}
	}
	void Data_InData::fromRawEvent(const Event& event) {
		if (event.data.size() < 7) {
			logger->warn("Data event should contain at least 6 bytes of data, but received: {}", event.data.toHex(true));
			return;
		}

		hubTimestampNs = static_cast<uint64_t>(tools::readU16LE(event.data, 0)) * 1000000;
		const uint16_t dataFlags = tools::readU16LE(event.data, 2);
		std::vector<uint8_t> pupSensorTypes;
		pupSensorTypes.emplace_back(event.data[6] & 0x0F);
		pupSensorTypes.emplace_back((event.data[6] >> 4) & 0x0F);
		pupSensorTypes.emplace_back(event.data[5] & 0x0F);
		pupSensorTypes.emplace_back((event.data[5] >> 4) & 0x0F);
		pupSensorTypes.emplace_back(event.data[4] & 0x0F);
		pupSensorTypes.emplace_back((event.data[4] >> 4) & 0x0F);

		loopId = dataFlags & 0x03;

		withHubIMU = (dataFlags & messaging::FLAG_HUB_IMU) != 0;
		withHubButtons = (dataFlags & messaging::FLAG_HUB_BUTTONS) != 0;
		withHubSystem = (dataFlags & messaging::FLAG_HUB_SYSTEM) != 0;
		withHubSpeaker = (dataFlags & messaging::FLAG_HUB_SPEAKER) != 0;
		withHubLight55Matrix = (dataFlags & messaging::FLAG_HUB_LIGHT_55_MATRIX) != 0;
		withSensorA = (dataFlags & messaging::FLAG_SENSOR_A) != 0;
		withSensorB = (dataFlags & messaging::FLAG_SENSOR_B) != 0;
		withSensorC = (dataFlags & messaging::FLAG_SENSOR_C) != 0;
		withSensorD = (dataFlags & messaging::FLAG_SENSOR_D) != 0;
		withSensorE = (dataFlags & messaging::FLAG_SENSOR_E) != 0;
		withSensorF = (dataFlags & messaging::FLAG_SENSOR_F) != 0;

		size_t offset = 7;
		if (withHubIMU) {
			sensorsPayload.emplace_back(SENSOR_TYPE_HUB_IMU, SENSOR_LOCATION_HUB, event.data.slice(offset, offset+messaging::SENSOR_DATA_SIZE_HUB_IMU));
			offset += messaging::SENSOR_DATA_SIZE_HUB_IMU;
		}

		if (withHubButtons) {
			sensorsPayload.emplace_back(SENSOR_TYPE_HUB_BUTTONS, SENSOR_LOCATION_HUB, event.data.slice(offset, offset+messaging::SENSOR_DATA_SIZE_HUB_BUTTONS));
			offset += messaging::SENSOR_DATA_SIZE_HUB_BUTTONS;
		}

		if (withHubSystem) {
			sensorsPayload.emplace_back(SENSOR_TYPE_HUB_SYSTEM, SENSOR_LOCATION_HUB, event.data.slice(offset, offset+messaging::SENSOR_DATA_SIZE_HUB_SYSTEM));
			offset += messaging::SENSOR_DATA_SIZE_HUB_SYSTEM;
		}

		if (withHubSpeaker) {
			sensorsPayload.emplace_back(SENSOR_TYPE_HUB_SPEAKER, SENSOR_LOCATION_HUB, event.data.slice(offset, offset+messaging::SENSOR_DATA_SIZE_HUB_SPEAKER));
			offset += messaging::SENSOR_DATA_SIZE_HUB_SPEAKER;
		}

		if (withHubLight55Matrix) {
			sensorsPayload.emplace_back(SENSOR_TYPE_HUB_LIGHT_55_MATRIX, SENSOR_LOCATION_HUB, event.data.slice(offset, offset+messaging::SENSOR_DATA_SIZE_HUB_LIGHT_55_MATRIX));
			offset += messaging::SENSOR_DATA_SIZE_HUB_LIGHT_55_MATRIX;
		}

		if (withSensorA) {
			const uint8_t sensorType = pupSensorTypes[0];
			const size_t dataSize = tools::getSensorDataSize(sensorType);
			sensorsPayload.emplace_back(pupSensorTypes[0], SENSOR_LOCATION_A, event.data.slice(offset, offset+dataSize));
			offset += dataSize;
		}

		if (withSensorB) {
			const uint8_t sensorType = pupSensorTypes[1];
			const size_t dataSize = tools::getSensorDataSize(sensorType);
			sensorsPayload.emplace_back(sensorType, SENSOR_LOCATION_B, event.data.slice(offset, offset+dataSize));
			offset += dataSize;
		}

		if (withSensorC) {
			const uint8_t sensorType = pupSensorTypes[2];
			const size_t dataSize = tools::getSensorDataSize(sensorType);
			sensorsPayload.emplace_back(sensorType, SENSOR_LOCATION_C, event.data.slice(offset, offset+dataSize));
			offset += dataSize;
		}

		if (withSensorD) {
			const uint8_t sensorType = pupSensorTypes[3];
			const size_t dataSize = tools::getSensorDataSize(sensorType);
			sensorsPayload.emplace_back(sensorType, SENSOR_LOCATION_D, event.data.slice(offset, offset+dataSize));
			offset += dataSize;
		}

		if (withSensorE) {
			const uint8_t sensorType = pupSensorTypes[4];
			const size_t dataSize = tools::getSensorDataSize(sensorType);
			sensorsPayload.emplace_back(sensorType, SENSOR_LOCATION_E, event.data.slice(offset, offset+dataSize));
			offset += dataSize;
		}

		if (withSensorF) {
			const uint8_t sensorType = pupSensorTypes[5];
			const size_t dataSize = tools::getSensorDataSize(sensorType);
			sensorsPayload.emplace_back(sensorType, SENSOR_LOCATION_F, event.data.slice(offset, offset+dataSize));
			// ReSharper disable once CppDFAUnusedValue
			offset += dataSize;
		}
	}

	CommandResponse_StartListening_InData::CommandResponse_StartListening_InData()
		:loopId(0)
	{}
	int CommandResponse_StartListening_InData::logLevel() const {return PYBRICKS_LOG_LEVEL_DEBUG;}
	void CommandResponse_StartListening_InData::printTitle(std::string& print) const {
		CommandResponse_InData::printTitle(print);
		print += " Start listening";
	}
	void CommandResponse_StartListening_InData::printData(std::string& print) const {
		CommandResponse_InData::printData(print);
		print += std::format(" Loop {}", loopId);
	}
	void CommandResponse_StartListening_InData::fromRawEvent(const Event& event) {
		CommandResponse_InData::fromRawEvent(event);

		if (event.data.size() < 3) {
			logger->warn("Data event should contain at least 3 bytes of data, but received: {}", event.data.toHex(true));
			return;
		}

		const uint8_t dataFlags = event.data[2];
		loopId = dataFlags & 0x03;
	}


	int CommandResponse_StopListening_InData::logLevel() const {return PYBRICKS_LOG_LEVEL_DEBUG;}
	void CommandResponse_StopListening_InData::printTitle(std::string& print) const {
		CommandResponse_InData::printTitle(print);
		print += " Stop listening";
	}
	void CommandResponse_StopListening_InData::printData(std::string& print) const {
		CommandResponse_InData::printData(print);
		print += std::format(" Loop {}", loopId);
	}
	void CommandResponse_StopListening_InData::fromRawEvent(const Event& event) {
		CommandResponse_InData::fromRawEvent(event);

		if (event.data.size() < 3) {
			logger->warn("Data event should contain at least 3 bytes of data, but received: {}", event.data.toHex(true));
			return;
		}

		const uint8_t dataFlags = event.data[2];
		loopId = dataFlags & 0x03;
	}


	int CommandResponse_StopAllListening_InData::logLevel() const {return PYBRICKS_LOG_LEVEL_DEBUG;}
	void CommandResponse_StopAllListening_InData::printTitle(std::string& print) const {
		CommandResponse_InData::printTitle(print);
		print += " Stop all listening";
	}
	void CommandResponse_StopAllListening_InData::printData(std::string& print) const {
		CommandResponse_InData::printData(print);
	}
	void CommandResponse_StopAllListening_InData::fromRawEvent(const Event& event) {
		CommandResponse_InData::fromRawEvent(event);
	}


	int CommandResponse_GetCapabilities_InData::logLevel() const {return PYBRICKS_LOG_LEVEL_INFO;}
	void CommandResponse_GetCapabilities_InData::printTitle(std::string& print) const {
		CommandResponse_InData::printTitle(print);
		print += " Get capabilities";
	}
	void CommandResponse_GetCapabilities_InData::printData(std::string& print) const {
		CommandResponse_InData::printData(print);
		print += " Sensors:\n";
		for (const auto& [sensorType, sensorLocation] : sensors) {
			print += std::format("\t- {:3}: {}\n", getSensorLocationName(sensorLocation), getSensorTypeName(sensorType));
		}
	}
	void CommandResponse_GetCapabilities_InData::fromRawEvent(const Event& event) {
		CommandResponse_InData::fromRawEvent(event);

		for (auto it = event.data.begin() + 2; it != event.data.end(); ++it) {
			const uint8_t byte = *it;
			const uint8_t sensorType = byte & 0x0F;
			const uint8_t sensorLocation = (byte >> 4) & 0x0F;
			sensors.emplace_back(sensorType, sensorLocation);
		}
	}


	CommandResponse_Actuator_InData::CommandResponse_Actuator_InData()
		:errorId(0x00)
	{}
	CommandResponse_Actuator_InData::CommandResponse_Actuator_InData(const uint8_t _errorId)
		:errorId(_errorId)
	{}
	int CommandResponse_Actuator_InData::logLevel() const {return PYBRICKS_LOG_LEVEL_DEBUG;}
	void CommandResponse_Actuator_InData::printTitle(std::string& print) const {
		CommandResponse_InData::printTitle(print);
		print += " Actuator command";
	}
	void CommandResponse_Actuator_InData::printData(std::string& print) const {
		CommandResponse_InData::printData(print);
		print += std::format(" - Error Id: {}", errorId);
	}
	void CommandResponse_Actuator_InData::fromRawEvent(const Event& event) {
		CommandResponse_InData::fromRawEvent(event);

		if (event.data.size() < 3) {
			logger->warn("Data event should contain at least 3 bytes of data, but received: {}", event.data.toHex(true));
			return;
		}

		errorId = event.data[2];
	}

}
