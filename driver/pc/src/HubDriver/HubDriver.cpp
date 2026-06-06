#include "./HubDriver.h"

#include <filesystem>
#include <utility>
#include <ranges>
#include <chrono>

#include "tools/tools.h"

using namespace std::chrono_literals;

namespace pybricksDriver
{
	HubDriver::HubDriver(std::string deviceName, std::string  serviceUUID, std::string  characteristicUUID)
		:_deviceName(std::move(deviceName))
		,_serviceUUID(std::move(serviceUUID))
		,_characteristicUUID(std::move(characteristicUUID))
		,_bleManager(BLEManager())
		,_logger(logging::createLogger("HubDriver"))
		,_status({0, 0, false, false, false, false, false, false, false, false, false, false, false})
		,_nextHandlerId(0)
		,_timeSyncId(0)
		,_lastHubTimestampNs(0)
	{
		_logger->trace("Initialized with device name: {}, service UUID: {}, characteristic UUID: {}", _deviceName, _serviceUUID, _characteristicUUID);


		addHandler<HubReady_InData>([this](const auto& data){
			_status.hubReady = true;
		});

		addHandler<CommandResponse_Ping_InData>([this](const auto& data){
			_pingTimeout = std::chrono::steady_clock::now() + 3s;
		});

		_pingTaskThread = std::make_unique<std::jthread>([this](const std::stop_token& stopToken){
			while (!stopToken.stop_requested()) {
				if (_bleManager.isConnected()) {
					const auto now = std::chrono::steady_clock::now();
					if (now > _pingTimeout) {
						_logger->warn("Hub ping timeout");
						disconnect();
					}else if (_status.programRunning && !_status.shuttingDown && !_status.shutdownRequested && !_status.batteryShuttingDown) {
						sendCommand(Ping_OutData{});
					}
				}
				std::this_thread::sleep_for(1000ms);
			}
		});

		_bleManager.bindOnDisconnect([this] {
			_logger->info("Handling disconnect - resetting status and timings");
			_status = {0, 0, false, false, false, false, false, false, false, false, false, false, false};
			_dataBuffer = {};
			_printBuffer = {};
		});
	}


	HubDriver::~HubDriver() {
		_logger->info("Releasing hub");
		disconnect();

		_logger->info("Releasing tasks");
		_pingTaskThread->request_stop();

		_logger->info("Released");
	}


	bool HubDriver::isHubReady() const {
		return _status.hubReady;
	}


	bool HubDriver::connect() {
		if (!_bleManager.connectToDevice(_deviceName)) {
			return false;
		}

		_pingTimeout = std::chrono::steady_clock::now() + 5s;

		_bleManager.subscribeToNotifications(_serviceUUID, _characteristicUUID, [this](const SimpleBLE::ByteArray& payload) {
			_onNotificationReceived(payload);
		});

		_logger->info("Connected to device: {}", _deviceName);

		return true;
	}


	void HubDriver::disconnect() {
		if (_bleManager.isConnected()) {
			_bleManager.unsubscribeFromNotifications(_serviceUUID, _characteristicUUID);
			_bleManager.disconnectDevice();
		}

		_logger->info("Disconnected from device: {}", _deviceName);
	}

	void HubDriver::startProgram(std::optional<uint8_t> programId){
		_logger->info("Starting hub program with ID: {}", programId.has_value() ? std::to_string(programId.value()) : "default");

		SimpleBLE::ByteArray payload;
		if (!programId.has_value()) {
			payload = SimpleBLE::ByteArray({messaging::GATT_COMMAND_START_PROGRAM});
		}else{
			payload = SimpleBLE::ByteArray({messaging::GATT_COMMAND_START_PROGRAM, programId.value()});
		}

		_status.hubReady = false;

		_bleManager.writeToCharacteristic(_serviceUUID, _characteristicUUID, payload);
	}

	void HubDriver::stopProgram() {
		_logger->info("Stopping hub program");

		const SimpleBLE::ByteArray payload({messaging::GATT_COMMAND_STOP_PROGRAM});

		_bleManager.writeToCharacteristic(_serviceUUID, _characteristicUUID, payload);
	}


	void HubDriver::_onNotificationReceived(const SimpleBLE::ByteArray& data) {
		_logger->trace("Notification received: {}", data.toHex(true));

		if (data.empty()) {
			_logger->warn("Received empty notification");
			return;
		}

		const uint8_t messageType = data[0];

		if (messageType == messaging::GATT_MESSAGE_STATUS) {
			if (data.size() < 5) {
				_logger->warn("Received status message with insufficient length: {}", data.size());
				return;
			}

			const uint32_t flags = tools::readU32LE(data, 1);

			const Status prevStatus = _status;

			_status.batteryLowVoltage = (flags & 0x01) != 0;
			_status.batteryShuttingDown = (flags & 0x02) != 0;
			_status.batteryHighCurrent = (flags & 0x04) != 0;
			_status.bleDiscoverable = (flags & 0x08) != 0;
			_status.bleLowSignal = (flags & 0x10) != 0;
			_status.powerButtonPressed = (flags & 0x20) != 0;
			_status.programRunning = (flags & 0x40) != 0;
			_status.shuttingDown = (flags & 0x80) != 0;
			_status.shutdownRequested = (flags & 0x100) != 0;
			_status.bleConnected = (flags & 0x200) != 0;

			if (data.size() >= 7) {
				_status.runningProgramId = data[5];
				_status.selectedProgramId = data[6];
			}

			if (_status.batteryLowVoltage != prevStatus.batteryLowVoltage) {
				_logger->warn("Battery low voltage : {}", _status.batteryLowVoltage);
			}
			if (_status.batteryShuttingDown != prevStatus.batteryShuttingDown) {
				_logger->warn("Battery shutting down : {}", _status.batteryShuttingDown);
			}
			if (_status.batteryHighCurrent != prevStatus.batteryHighCurrent) {
				_logger->warn("Battery high current : {}", _status.batteryHighCurrent);
			}
			if (_status.bleDiscoverable != prevStatus.bleDiscoverable) {
				_logger->info("BLE discoverable : {}", _status.bleDiscoverable);
			}
			if (_status.bleLowSignal != prevStatus.bleLowSignal) {
				_logger->warn("BLE low signal : {}", _status.bleLowSignal);
			}
			if (_status.powerButtonPressed != prevStatus.powerButtonPressed) {
				_logger->info("Power button pressed : {}", _status.powerButtonPressed);
			}
			if (_status.programRunning != prevStatus.programRunning) {
				if (_status.programRunning) {
					_logger->info("Program started");
					++_timeSyncId;
					_lastHubTimestampNs = 0;
				}else {
					_logger->info("Program stopped");
					_status.hubReady = false;
				}
			}
			if (_status.shuttingDown != prevStatus.shuttingDown) {
				_logger->warn("Shutting down : {}", _status.shuttingDown);
			}
			if (_status.shutdownRequested != prevStatus.shutdownRequested) {
				_logger->warn("Shutdown requested : {}", _status.shutdownRequested);
			}
			if (_status.bleConnected != prevStatus.bleConnected) {
				_logger->info("BLE connected : {}", _status.bleConnected);
			}
			if (_status.runningProgramId != prevStatus.runningProgramId) {
				_logger->info("Running program ID : {}", _status.runningProgramId);
			}
			if (_status.selectedProgramId != prevStatus.selectedProgramId) {
				_logger->info("Selected program ID : {}", _status.selectedProgramId);
			}

		}else if (messageType == messaging::GATT_MESSAGE_READ_FROM_STDOUT) {
			if (data.size() < 2) {
				return;
			}
			SimpleBLE::ByteArray remainingData(data.begin() + 1, data.end());
			while(!remainingData.empty()) {
				const auto messages = messaging::consumeDataForPrintableContent(_printBuffer, remainingData);

				for (const auto& message : messages) {
					_logger->debug("Hub: {}", message);
				}
			}

		}else if (messageType == messaging::GATT_MESSAGE_READ_FROM_DATA) {
			if (data.size() < 2) {
				return;
			}

			SimpleBLE::ByteArray remainingData(data.begin() + 1, data.end());

			while(!remainingData.empty()){
				const auto events = messaging::consumeDataForEvent(_dataBuffer, remainingData);

				for (const auto& event : events) {
					auto commandDataOpt = messaging::getCommandInDataFromEvent(event);
					if (commandDataOpt.has_value()) {
						auto commandData = commandDataOpt.value();

						std::visit([event, this](auto&& c) {
							c.fromRawEvent(event);
							applyEventModifier(c);
							if (PYBRICKS_LOG_LEVEL >= c.logLevel()) {
								_logger->debug(c.print());
							}
						}, commandData);


						for (const auto& handler : _handlers | std::views::values) {
							// ReSharper disable once CppExpressionWithoutSideEffects
							handler(commandData);
						}

						for (auto it = _oneTimeHandlers.begin(); it != _oneTimeHandlers.end(); ) {
							const messaging::CommandHandler& handler = *it;
							if (handler(commandData)) {
								it = _oneTimeHandlers.erase(it);
							}else {
								++it;
							}
						}
					}
				}
			}
		}
	}


	void HubDriver::removeHandler(const size_t handlerId) {
		_handlers.erase(handlerId);
	}


	// ReSharper disable once CppParameterMayBeConstPtrOrRef
	size_t HubDriver::bindOnDisconnect(std::function<void()>& callback) {
		return _bleManager.bindOnDisconnect(callback);
	}

	void HubDriver::unbindOnDisconnect(const size_t callbackId) {
		_bleManager.unbindOnDisconnect(callbackId);
	}


	bool HubDriver::isConnected(){
		return _bleManager.isConnected();
	}

	bool HubDriver::isProgramRunning() const {
		return _status.programRunning;
	}

	uint8_t HubDriver::getRunningProgramId() const {
		return _status.selectedProgramId;
	}
}