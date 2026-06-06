#pragma once

#include "msgpack_compat.h"
#include "simpleble_compat.h"
#include "commands.h"

#include <vector>
#include <tuple>


namespace pybricksDriver
{
	// Base struct for CommandResponse in data
	struct CommandResponse_InData : Data_Base
	{
		bool success;

		explicit CommandResponse_InData();

		void fromRawEvent(const Event& event) override;

		PYBRICKS_PRINT(CommandResponse_InData);
		PYBRICKS_MSGPACK_DEFINE(
			success
		);
	};



	// Structs for Command In data
	struct HubReady_InData : Data_Base
	{
		explicit HubReady_InData() = default;

		void fromRawEvent(const Event& event) override;

		PYBRICKS_PRINT(HubReady_InData);
		PYBRICKS_MSGPACK_DEFINE(
		);
	};


	struct Data_InData : StartListening_OutData
	{
		// Timestamp in ns
		uint64_t hubTimestampNs;
		uint16_t syncId;

		// Sensors payload
		std::vector<std::tuple<uint8_t, uint8_t, std::vector<uint8_t>>> sensorsPayload; //Sensor type, Sensor lcoation, sensor payload

		explicit Data_InData();

		void fromRawEvent(const Event& event) override;

		PYBRICKS_PRINT(Data_InData);
		PYBRICKS_MSGPACK_DEFINE(
			PYBRICKS_MSGPACK_BASE(StartListening_OutData),
			hubTimestampNs, syncId, sensorsPayload
		);
	};


	struct CommandResponse_Ping_InData : Ping_OutData, CommandResponse_InData
	{
		explicit CommandResponse_Ping_InData() = default;

		void fromRawEvent(const Event& event) override;

		PYBRICKS_PRINT(CommandResponse_Ping_InData);
		PYBRICKS_MSGPACK_DEFINE(
			PYBRICKS_MSGPACK_BASE(Ping_OutData),
			PYBRICKS_MSGPACK_BASE(CommandResponse_InData)
		);
	};


	struct CommandResponse_TimeSync_InData : TimeSync_OutData, CommandResponse_InData
	{
		uint64_t hubTimestampNs; // Timestamp in ns
		uint16_t syncId; // Id of the synchronisation (It changes when the hub is restarted, or the sensors are changed)

		explicit CommandResponse_TimeSync_InData();

		void fromRawEvent(const Event& event) override;

		PYBRICKS_PRINT(CommandResponse_TimeSync_InData);
		PYBRICKS_MSGPACK_DEFINE(
			PYBRICKS_MSGPACK_BASE(CommandResponse_InData),
			PYBRICKS_MSGPACK_BASE(TimeSync_OutData),
			hubTimestampNs, syncId
		);
	};


	struct CommandResponse_StartListening_InData : CommandResponse_InData
	{
		uint8_t loopId;

		explicit CommandResponse_StartListening_InData();

		void fromRawEvent(const Event& event) override;

		PYBRICKS_PRINT(CommandResponse_StartListening_InData);
		PYBRICKS_MSGPACK_DEFINE(
			PYBRICKS_MSGPACK_BASE(CommandResponse_InData),
			loopId
		);
	};


	struct CommandResponse_StopListening_InData : CommandResponse_InData, StopListening_OutData
	{
		explicit CommandResponse_StopListening_InData() = default;

		void fromRawEvent(const Event& event) override;

		PYBRICKS_PRINT(CommandResponse_StopListening_InData);
		PYBRICKS_MSGPACK_DEFINE(
			PYBRICKS_MSGPACK_BASE(CommandResponse_InData),
			PYBRICKS_MSGPACK_BASE(StopListening_OutData)
		);
	};

	struct CommandResponse_StopAllListening_InData : CommandResponse_InData
	{
		explicit CommandResponse_StopAllListening_InData() = default;

		void fromRawEvent(const Event& event) override;

		PYBRICKS_PRINT(CommandResponse_StopAllListening_InData);
		PYBRICKS_MSGPACK_DEFINE(
			PYBRICKS_MSGPACK_BASE(CommandResponse_InData)
		);
	};


	struct CommandResponse_GetCapabilities_InData : CommandResponse_InData
	{
		std::vector<std::tuple<uint8_t, uint8_t>> sensors; //type, location

		explicit CommandResponse_GetCapabilities_InData() = default;

		void fromRawEvent(const Event& event) override;

		PYBRICKS_PRINT(CommandResponse_GetCapabilities_InData);
		PYBRICKS_MSGPACK_DEFINE(
			PYBRICKS_MSGPACK_BASE(CommandResponse_InData),
			sensors
		);
	};

	struct CommandResponse_Actuator_InData : CommandResponse_InData
	{
		uint8_t errorId;

		explicit CommandResponse_Actuator_InData();
		explicit CommandResponse_Actuator_InData(uint8_t _errorId);

		void fromRawEvent(const Event& event) override;

		PYBRICKS_PRINT(CommandResponse_Actuator_InData);
		PYBRICKS_MSGPACK_DEFINE(
			PYBRICKS_MSGPACK_BASE(CommandResponse_InData)
		);
	};

}