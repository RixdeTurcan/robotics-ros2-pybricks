#include <pybricks-driver/types.h>
#include <pybricks-driver/logging.h>

#include "constant_internal.h"
#include "tools/tools.h"

#include <format>



namespace pybricksDriver
{
	namespace
	{
		logging::Logger& logger = logging::registerLogger("Commands");
	}

	int Data_Base::logLevel() const {return PYBRICKS_LOG_LEVEL_ERROR;}
	void Data_Base::printTitle(std::string& print) const{}
	void Data_Base::printData(std::string& print) const{}
	std::string Data_Base::print() const{return std::string{};}
	void Data_Base::fromRawEvent(const Event& event){}
	CommandAndData Data_Base::computeCommandAndData() const{return CommandAndData{};}


	int Ping_OutData::logLevel() const {return PYBRICKS_LOG_LEVEL_TRACE;}
	void Ping_OutData::printTitle(std::string& print) const {
		print += "OUT Ping";
	}
	void Ping_OutData::printData(std::string& print) const {
		Data_Base::printData(print);
	}
	CommandAndData Ping_OutData::computeCommandAndData() const {
		return {messaging::MESSAGE_OUT_COMMAND_PING, {}};
	}


	int TimeSync_OutData::logLevel() const {return PYBRICKS_LOG_LEVEL_DEBUG;}
	TimeSync_OutData::TimeSync_OutData(const uint64_t _pcTimestampNs)
		:pcTimestampNs(_pcTimestampNs)
	{}
	TimeSync_OutData::TimeSync_OutData()
		:pcTimestampNs(0)
	{}
	void TimeSync_OutData::printTitle(std::string& print) const {
		print += "OUT Time sync";
	}
	void TimeSync_OutData::printData(std::string& print) const {
		Data_Base::printData(print);
		print += std::format("pc timestamp (ns) : {}", pcTimestampNs);
	}
	CommandAndData TimeSync_OutData::computeCommandAndData() const {
		return {messaging::MESSAGE_OUT_COMMAND_TIMESYNC, {tools::splitU64LE(pcTimestampNs)}};
	}


	int StartListening_OutData::logLevel() const {return PYBRICKS_LOG_LEVEL_INFO;}
	StartListening_OutData::StartListening_OutData(
			const uint8_t _loopId, const bool _withHubIMU, const bool _withHubButtons,
			const bool _withHubSystem, const bool _withHubSpeaker, const bool _withHubLight55Matrix,
			const bool _withSensorA, const bool _withSensorB, const bool _withSensorC, const bool _withSensorD,
			const bool _withSensorE, const bool _withSensorF, const uint8_t _loopPeriod10Ms)
		:loopId(_loopId)
		,withHubIMU(_withHubIMU)
		,withHubButtons(_withHubButtons)
		,withHubSystem(_withHubSystem)
		,withHubSpeaker(_withHubSpeaker)
		,withHubLight55Matrix(_withHubLight55Matrix)
		,withSensorA(_withSensorA)
		,withSensorB(_withSensorB)
		,withSensorC(_withSensorC)
		,withSensorD(_withSensorD)
		,withSensorE(_withSensorE)
		,withSensorF(_withSensorF)
		,loopPeriod10Ms(_loopPeriod10Ms)
	{}
	StartListening_OutData::StartListening_OutData()
		:loopId(0)
		,withHubIMU(false)
		,withHubButtons(false)
		,withHubSystem(false)
		,withHubSpeaker(false)
		,withHubLight55Matrix(false)
		,withSensorA(false)
		,withSensorB(false)
		,withSensorC(false)
		,withSensorD(false)
		,withSensorE(false)
		,withSensorF(false)
		,loopPeriod10Ms(1)
	{}
	void StartListening_OutData::printTitle(std::string& print) const {
		print += "OUT Start listening";
	}
	void StartListening_OutData::printData(std::string& print) const {
		Data_Base::printData(print);
		print += std::format("Loop {} - {}ms [", loopId, loopPeriod10Ms*10);

		if (withHubIMU) {
			print += "Hub IMU,";
		}
		if (withHubButtons) {
			print += "Hub buttons,";
		}
		if (withHubSystem) {
			print += "Hub system,";
		}
		if (withHubSpeaker) {
			print += "Hub speaker,";
		}
		if (withHubLight55Matrix) {
			print += "Hub light 55 matrix,";
		}
		if (withSensorA) {
			print += "Sensor A,";
		}
		if (withSensorB) {
			print += "Sensor B,";
		}
		if (withSensorC) {
			print += "Sensor C,";
		}
		if (withSensorD) {
			print += "Sensor D,";
		}
		if (withSensorE) {
			print += "Sensor E,";
		}
		if (withSensorF) {
			print += "Sensor F,";
		}

		if (print.back() == ',') {
			print.pop_back();
		}

		print += ']';
	}
	CommandAndData StartListening_OutData::computeCommandAndData() const{
		if (loopId > 3) {
			logger->warn("Loop ID should be between 0 and 3, but got: {}", loopId);
		}

		uint16_t flagsAndLoopId = loopId;

		if (withHubIMU) flagsAndLoopId |= messaging::FLAG_HUB_IMU;
		if (withHubButtons) flagsAndLoopId |= messaging::FLAG_HUB_BUTTONS;
		if (withHubSystem) flagsAndLoopId |= messaging::FLAG_HUB_SYSTEM;
		if (withHubSpeaker) flagsAndLoopId |= messaging::FLAG_HUB_SPEAKER;
		if (withHubLight55Matrix) flagsAndLoopId |= messaging::FLAG_HUB_LIGHT_55_MATRIX;
		if (withSensorA) flagsAndLoopId |= messaging::FLAG_SENSOR_A;
		if (withSensorB) flagsAndLoopId |= messaging::FLAG_SENSOR_B;
		if (withSensorC) flagsAndLoopId |= messaging::FLAG_SENSOR_C;
		if (withSensorD) flagsAndLoopId |= messaging::FLAG_SENSOR_D;
		if (withSensorE) flagsAndLoopId |= messaging::FLAG_SENSOR_E;
		if (withSensorF) flagsAndLoopId |= messaging::FLAG_SENSOR_F;

		auto bytes = tools::splitU16LE(flagsAndLoopId);
		bytes.emplace_back(loopPeriod10Ms);

		return {messaging::MESSAGE_OUT_COMMAND_START_LISTENING, bytes};
	}


	int StopListening_OutData::logLevel() const {return PYBRICKS_LOG_LEVEL_INFO;}
	StopListening_OutData::StopListening_OutData(const uint8_t _loopId)
		:loopId(_loopId)
	{}
	StopListening_OutData::StopListening_OutData()
		:loopId(0)
	{}
	void StopListening_OutData::printTitle(std::string& print) const {
		print += "OUT Stop listening";
	}
	void StopListening_OutData::printData(std::string& print) const {
		Data_Base::printData(print);
		print += std::format("Loop {}", loopId);
	}
	CommandAndData StopListening_OutData::computeCommandAndData() const {
		return {messaging::MESSAGE_OUT_COMMAND_STOP_LISTENING, {loopId}};
	}


	int StopAllListening_OutData::logLevel() const {return PYBRICKS_LOG_LEVEL_INFO;}
	void StopAllListening_OutData::printTitle(std::string& print) const {
		print += "OUT Stop all listening";
	}
	void StopAllListening_OutData::printData(std::string& print) const {
		Data_Base::printData(print);
	}
	CommandAndData StopAllListening_OutData::computeCommandAndData() const {
		return {messaging::MESSAGE_OUT_COMMAND_STOP_ALL_LISTENING, {}};
	}


	int GetCapabilities_OutData::logLevel() const {return PYBRICKS_LOG_LEVEL_INFO;}
	void GetCapabilities_OutData::printTitle(std::string& print) const {
		print += "OUT Get capabilities";
	}
	void GetCapabilities_OutData::printData(std::string& print) const {
		Data_Base::printData(print);
	}
	CommandAndData GetCapabilities_OutData::computeCommandAndData() const {
		return {messaging::MESSAGE_OUT_COMMAND_GET_CAPABILITIES, {}};
	}


	Actuator_OutData::Actuator_OutData(
			const uint8_t _sensorType,
			const uint8_t _sensorLocation,
			const std::vector<uint8_t>& _commandData)
		:sensorType(_sensorType)
		,sensorLocation(_sensorLocation)
		,commandData(_commandData)
	{}
	Actuator_OutData::Actuator_OutData()
		:sensorType(0)
		,sensorLocation(0)
	{}
	int Actuator_OutData::logLevel() const {return PYBRICKS_LOG_LEVEL_DEBUG;}
	void Actuator_OutData::printTitle(std::string& print) const {
		print += "OUT Actuator command";
	}
	void Actuator_OutData::printData(std::string& print) const {
		Data_Base::printData(print);
		print += std::format("Sensor {} / {} - Command {} - Data {}",
							 getSensorLocationName(sensorLocation),
							 getSensorTypeName(sensorType),
							 commandData[0],
							 tools::toHexString(commandData)
							);
	}
	CommandAndData Actuator_OutData::computeCommandAndData() const {
		std::vector<uint8_t> bytes;
		bytes.reserve(2 + commandData.size());

		const uint8_t flags = (sensorType & 0x0F) | ((sensorLocation << 4) & 0xF0);
		bytes.push_back(flags);
		bytes.insert(bytes.end(), commandData.begin(), commandData.end());

		return {messaging::MESSAGE_OUT_COMMAND_ACTUATOR, bytes};
	}

}
