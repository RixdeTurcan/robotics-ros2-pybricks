#include "messaging.h"

#include "constant_internal.h"
#include "tools/tools.h"
#include <pybricks-driver/logging.h>



namespace pybricksDriver::messaging
{
	namespace
	{
		logging::Logger& logger = logging::registerLogger("Messaging");
	}

	std::vector<std::string> consumeDataForPrintableContent(SimpleBLE::ByteArray& buffer, SimpleBLE::ByteArray& data) {
		for (size_t i = 0; i < data.size(); ++i) {
			const uint8_t byte = data[i];
			buffer.push_back(byte);
			const size_t bufferSize = buffer.size();

			if (bufferSize >= 2 && buffer[bufferSize-2] == MESSAGE_PRINT_END_0 && buffer[bufferSize-1] == MESSAGE_PRINT_END_1) {
				auto message = std::string(buffer.begin(), buffer.end() - 2);
				buffer.clear();
				SimpleBLE::ByteArray remainingData(data.begin() + i + 1, data.end());
				data = std::move(remainingData);
				return {message};
			}
		}
		data.clear();
		return {};
	}


	bool decodeDataMessage(Event& event, SimpleBLE::ByteArray& buffer) {
		logger->trace("Decoding message from buffer: {}", buffer.toHex(true));

		const size_t dataSize = buffer.size();

		event.command = buffer[1];

		const uint8_t payloadSize = buffer[2];
		uint8_t crc = buffer[dataSize - 1];

		//crc have been escaped
		if (dataSize == 3 + payloadSize + 2) {
			if (crc == MESSAGE_BOUNDARY_ESCAPE_START) {
				crc =  MESSAGE_BOUNDARY_START;
			}
		}

		uint8_t computedCrc = tools::crc8(buffer.slice(0, 3 + payloadSize));
		if (computedCrc != crc) {
			logger->warn("CRC mismatch: expected {:X}, computed {:X}", crc, computedCrc);
			buffer.clear();
			return false;
		}


		for (size_t i = 0; i < payloadSize; ++i) {
			const uint8_t byte = buffer[3 + i];
			if (byte == MESSAGE_BOUNDARY_ESCAPE) {
				if (i == payloadSize - 1) {
					logger->warn("Invalid escape byte at the end of the payload");
					buffer.clear();
					return false;
				}

				const uint8_t nextByte = buffer[3 + i + 1];
				if (nextByte == MESSAGE_BOUNDARY_ESCAPE_START) {
					event.data.push_back(MESSAGE_BOUNDARY_START);
				}else if (nextByte == MESSAGE_BOUNDARY_ESCAPE) {
					event.data.push_back(MESSAGE_BOUNDARY_ESCAPE);
				}else{
					logger->warn("Invalid escape sequence: {} {}", byte, nextByte);
					buffer.clear();
					return false;
				}
				++i;

			}else{
				event.data.push_back(byte);
			}
		}

		logger->trace("Decoded message - command: {:X}, size: {}, payload: {}", event.command, event.data.size(), event.data.toHex(true));
		buffer.clear();
		return true;
	}


	std::tuple<bool, bool, Event> processConsumeDataForEvent(SimpleBLE::ByteArray& buffer, SimpleBLE::ByteArray& data) {
		Event event;

		if (data.empty()) {
			return {false, false, event};
		}

		const size_t dataSize = data.size();
		size_t dataIndex = 0;


		/* Message format:
		 * - 1 byte: MESSAGE_BOUNDARY_START
		 * - 1 byte: command byte
		 * - 1 byte: payload length (N)
		 * - N bytes: payload
		 * - 1 byte: CRC8 checksum of the N+3 previous bytes (boundary, command, length, payload)
		 */

		// First there is a MESSAGE_BOUNDARY_START byte
		if (buffer.empty()) {
			const uint8_t byte = data[dataIndex];
			if (byte != MESSAGE_BOUNDARY_START) {
				data.clear();
				return {false, false, event};
			}
			buffer.reserve(3);
			buffer.push_back(byte);
			++dataIndex;
		}

		if (dataIndex >= dataSize) {
			data.clear();
			return {false, false, event};
		}

		// Second read the command byte
		if (buffer.size() == 1) {
			buffer.push_back(data[dataIndex]);
			++dataIndex;
		}

		if (dataIndex >= dataSize) {
			data.clear();
			return {false, false, event};
		}

		// Third read the payload length byte
		if (buffer.size() == 2) {
			const uint8_t payloadLength = data[dataIndex];

			if (payloadLength > 0xFE) {
				logger->warn("Invalid payload length: {}", payloadLength);
				buffer.clear();
				return {false, false, event};
			}

			buffer.reserve(3 + payloadLength + 2);
			buffer.push_back(payloadLength);
			++dataIndex;
		}

		if (dataIndex >= dataSize) {
			data.clear();
			return {false, false, event};
		}

		// Fourth read the payload bytes until we have the full payload in the buffer (or we run out of bytes in the current notification)
		const size_t sizeAfterPayload = 3 + buffer[2];
		const auto currentSize = buffer.size();
		if (currentSize < sizeAfterPayload) {
			// Now we are sure that the difference is >= 0, we can safely cast it to size_t
			const size_t nbBytesToRead = std::min(sizeAfterPayload - currentSize, dataSize - dataIndex);
			buffer.insert(buffer.end(), data.begin() + dataIndex, data.begin() + dataIndex + nbBytesToRead);
			dataIndex += nbBytesToRead;
		}

		if (dataIndex >= dataSize) {
			data.clear();
			return {false, false, event};
		}

		// Finally read the CRC8 checksum byte if we have the full payload in the buffer
		if (buffer.size() == sizeAfterPayload) {
			buffer.push_back(data[dataIndex]);
			++dataIndex;
		}

		if (buffer[sizeAfterPayload] == MESSAGE_BOUNDARY_ESCAPE) {
			if (dataIndex >= dataSize) {
				data.clear();
				return {false, false, event};
			}

			buffer.push_back(data[dataIndex]);
			++dataIndex;
		}

		if (!decodeDataMessage(event, buffer)) {
			return {false, false, event};
		}

		//Truncate the buffer to the next message if there is remaining data in the current notification, and process it in the next loop iteration
		if (dataIndex >= dataSize) {
			data.clear();
			return {false, true, event};
		}

		SimpleBLE::ByteArray remainingData(data.begin() + dataIndex, data.end());
	    data = std::move(remainingData);
	    return {true, true, event};
	}

	std::vector<Event> consumeDataForEvent(SimpleBLE::ByteArray& buffer, SimpleBLE::ByteArray& data) {
		std::vector<Event> events;
		while(!data.empty()) {
			auto [shouldRepeat, eventComplete, event] = processConsumeDataForEvent(buffer, data);
			if (eventComplete) {
				events.push_back(event);
			}
			if (!shouldRepeat) {
				break;
			}
		}
		return events;
	}

	std::optional<CommandInData> getCommandInDataFromEvent(const Event& event) {
		if (event.command == MESSAGE_IN_COMMAND_DATA) {
			return Data_InData{};
		}

		if (event.command == MESSAGE_IN_COMMAND_RESPONSE) {
			if (event.data.empty()) {
				logger->warn("Received command response with empty data");
				return std::nullopt;
			}
			uint8_t outCommand = event.data[0];

			if (outCommand == MESSAGE_OUT_COMMAND_PING) {
				return CommandResponse_Ping_InData{};
			}
			if (outCommand == MESSAGE_OUT_COMMAND_START_LISTENING) {
				return CommandResponse_StartListening_InData{};
			}
			if (outCommand == MESSAGE_OUT_COMMAND_STOP_LISTENING) {
				return CommandResponse_StopListening_InData{};
			}
			if (outCommand == MESSAGE_OUT_COMMAND_STOP_ALL_LISTENING) {
				return CommandResponse_StopAllListening_InData{};
			}
			if (outCommand == MESSAGE_OUT_COMMAND_GET_CAPABILITIES) {
				return CommandResponse_GetCapabilities_InData{};
			}
			if (outCommand == MESSAGE_OUT_COMMAND_TIMESYNC) {
				return CommandResponse_TimeSync_InData{};
			}
			if (outCommand == MESSAGE_OUT_COMMAND_ACTUATOR) {
				return CommandResponse_Actuator_InData{};
			}



			logger->warn("Received command response with unknown out command: {}", outCommand);
			return std::nullopt;
		}

		if (event.command == MESSAGE_IN_COMMAND_HUB_READY) {
			return HubReady_InData{};
		}

		logger->warn("Received event with unknown command: {}", event.command);
		return std::nullopt;
	}

}