#pragma once

#include <pybricks-driver/types.h>

#include "tools/tools.h"


namespace pybricksDriver
{
	template<typename T> requires is_CommandInOutType<T>
	messaging::CommandHandler HubDriver::castToCommandHandler(const std::function<void(const T&)>& handler) {
		return [handler](const CommandInData& data) {
			if (std::holds_alternative<T>(data)) {
				handler(std::get<T>(data));
				return true;
			}
			return false;
		};
	}

	template<typename T> requires is_CommandInType<T>
	size_t HubDriver::addHandler(const std::function<void(const T&)>& handler) {
		const size_t handlerId = _nextHandlerId++;
		_handlers.emplace(handlerId, castToCommandHandler(handler));
		return handlerId;
	}

	template<typename T> requires is_CommandInType<T>
	void HubDriver::addOneTimeHandler(const std::function<void(const T&)>& handler) {
		_oneTimeHandlers.push_back(castToCommandHandler(handler));
	}


	template<typename T> requires is_CommandOutType<T>
	void HubDriver::sendCommand(const T& commandData) {
		if (PYBRICKS_LOG_LEVEL >= commandData.logLevel()) {
			_logger->debug(commandData.print());
		}
		const auto [command, data] = commandData.computeCommandAndData();


		SimpleBLE::ByteArray payload;
		payload.reserve(6 + data.size());
		payload.push_back(messaging::GATT_COMMAND_WRITE_TO_STDIN);
		payload.push_back(messaging::MESSAGE_BOUNDARY_START);
		payload.push_back(command);
		payload.push_back(static_cast<uint8_t>(0x00)); // Placeholder for payload length, will be updated after escaping

		for (const uint8_t byte : data) {
			if (byte == messaging::MESSAGE_BOUNDARY_START) {
				payload.push_back(messaging::MESSAGE_BOUNDARY_ESCAPE);
				payload.push_back(messaging::MESSAGE_BOUNDARY_ESCAPE_START);
			}else if (byte == messaging::MESSAGE_BOUNDARY_ESCAPE) {
				payload.push_back(messaging::MESSAGE_BOUNDARY_ESCAPE);
				payload.push_back(messaging::MESSAGE_BOUNDARY_ESCAPE);
			}else {
				payload.push_back(byte);
			}
		}

		if (payload.size() - 4 > 0xFE) {
			_logger->error("Payload size after escaping is too large: {}", payload.size() - 3);
			return;
		}

		payload[3] = static_cast<uint8_t>(payload.size() - 4);

		const uint8_t crc = tools::crc8(SimpleBLE::ByteArray(payload.begin() + 1, payload.end()));

		if (crc == messaging::MESSAGE_BOUNDARY_START) {
			payload.push_back(messaging::MESSAGE_BOUNDARY_ESCAPE);
			payload.push_back(messaging::MESSAGE_BOUNDARY_ESCAPE_START);
		}else if (crc == messaging::MESSAGE_BOUNDARY_ESCAPE) {
			payload.push_back(messaging::MESSAGE_BOUNDARY_ESCAPE);
			payload.push_back(messaging::MESSAGE_BOUNDARY_ESCAPE);
		}else {
			payload.push_back(crc);
		}

		_bleManager.writeToCharacteristic(_serviceUUID, _characteristicUUID, payload);
	}

	template<typename T> requires is_CommandInType<T>
	void HubDriver::applyEventModifier(T& event) {
		if constexpr (std::same_as<T, CommandResponse_TimeSync_InData> || std::same_as<T, Data_InData>) {
			std::lock_guard guard(_eventModifierLock);

			event.syncId = _timeSyncId;
			constexpr uint64_t limitMs = 0x10000;
			constexpr uint64_t limitNs = limitMs * 1000000;
			constexpr uint64_t halfLimitNs = limitNs / 2;
			if (event.hubTimestampNs + halfLimitNs < _lastHubTimestampNs) { //uint16 overflow happened
				const uint64_t delta = (_lastHubTimestampNs - halfLimitNs) - event.hubTimestampNs;
				const uint64_t nb = 1 + delta / limitNs;;
				event.hubTimestampNs += nb * limitNs;
			}
			_lastHubTimestampNs = std::max(_lastHubTimestampNs, event.hubTimestampNs);
		}
	}
}