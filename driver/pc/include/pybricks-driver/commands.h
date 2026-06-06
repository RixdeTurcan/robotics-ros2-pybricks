#pragma once

#include "msgpack_compat.h"
#include "simpleble_compat.h"

#include <string>
// ReSharper disable once CppUnusedIncludeDirective
#include <ostream>

#define PYBRICKS_PRINT(T) \
	int logLevel() const override; \
	void printTitle(std::string& print) const override; \
	void printData(std::string& print) const override; \
	std::string print() const override { \
		std::string print; \
		printTitle(print); \
		print += " -> "; \
		printData(print); \
		return print; \
	} \
	friend std::ostream& operator<<(std::ostream& os, const T& self){ \
		os << self.print(); \
		return os; \
	}

namespace pybricksDriver
{
	// Base struct for Command In, Out and PubSub data
	struct Data_Base
	{
		explicit Data_Base() = default;
		virtual ~Data_Base() = default;

		virtual void fromRawEvent(const Event& event);
		[[nodiscard]] virtual CommandAndData computeCommandAndData() const;

		[[nodiscard]] virtual int logLevel() const;
		virtual void printTitle(std::string& print) const;
		virtual void printData(std::string& print) const;
		[[nodiscard]] virtual std::string print() const;
	};

	// Structs for Command Out data

	struct Ping_OutData : Data_Base
	{
		explicit Ping_OutData() = default;

		[[nodiscard]] CommandAndData computeCommandAndData() const override;

		PYBRICKS_PRINT(Ping_OutData);
		PYBRICKS_MSGPACK_DEFINE(
		);
	};

	struct TimeSync_OutData : Data_Base
	{
		uint64_t pcTimestampNs; // Timestamp in ns

		explicit TimeSync_OutData();
		explicit TimeSync_OutData(uint64_t _pcTimestampNs);

		[[nodiscard]] CommandAndData computeCommandAndData() const override;

		PYBRICKS_PRINT(TimeSync_OutData);
		PYBRICKS_MSGPACK_DEFINE(
			pcTimestampNs
		);
	};


	struct StartListening_OutData : Data_Base
	{
		uint8_t loopId;

		bool withHubIMU;
		bool withHubButtons;
		bool withHubSystem;
		bool withHubSpeaker;
		bool withHubLight55Matrix;
		bool withSensorA;
		bool withSensorB;
		bool withSensorC;
		bool withSensorD;
		bool withSensorE;
		bool withSensorF;

		uint8_t loopPeriod10Ms;

		explicit StartListening_OutData();
		explicit StartListening_OutData(
			uint8_t _loopId, bool _withHubIMU, bool _withHubButtons,
			bool _withHubSystem, bool _withHubSpeaker, bool _withHubLight55Matrix,
			bool _withSensorA, bool _withSensorB, bool _withSensorC, bool _withSensorD,
			bool _withSensorE, bool _withSensorF, uint8_t _loopPeriod10Ms);

		[[nodiscard]] CommandAndData computeCommandAndData() const override;

		PYBRICKS_PRINT(StartListening_OutData);
		PYBRICKS_MSGPACK_DEFINE(
			loopId,
			withHubIMU, withHubButtons, withHubSystem, withHubSpeaker,
			withHubLight55Matrix, withSensorA, withSensorB,
			withSensorC, withSensorD, withSensorE, withSensorF,
			loopPeriod10Ms
		);
	};

	struct StopListening_OutData : Data_Base
	{
		uint8_t loopId;

		explicit StopListening_OutData();
		explicit StopListening_OutData(uint8_t _loopId);

		[[nodiscard]] CommandAndData computeCommandAndData() const override;

		PYBRICKS_PRINT(StopListening_OutData);
		PYBRICKS_MSGPACK_DEFINE(
			loopId
		);
	};

	struct StopAllListening_OutData : Data_Base
	{
		explicit StopAllListening_OutData() = default;

		[[nodiscard]] CommandAndData computeCommandAndData() const override;

		PYBRICKS_PRINT(StopAllListening_OutData);
		PYBRICKS_MSGPACK_DEFINE(
		);
	};

	struct GetCapabilities_OutData : Data_Base
	{
		explicit GetCapabilities_OutData() = default;

		[[nodiscard]] CommandAndData computeCommandAndData() const override;

		PYBRICKS_PRINT(GetCapabilities_OutData);
		PYBRICKS_MSGPACK_DEFINE(
		);
	};

	struct Actuator_OutData : Data_Base
	{
		uint8_t sensorType;
		uint8_t sensorLocation;
		std::vector<uint8_t> commandData;

		explicit Actuator_OutData();
		explicit Actuator_OutData(
			uint8_t _sensorType,
			uint8_t _sensorLocation,
			const std::vector<uint8_t>& _commandData
		);

		[[nodiscard]] CommandAndData computeCommandAndData() const override;

		PYBRICKS_PRINT(Actuator_OutData);
		PYBRICKS_MSGPACK_DEFINE(
			sensorType, sensorLocation, commandData
		);
	};

}