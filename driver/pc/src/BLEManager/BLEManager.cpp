#include "./BLEManager.h"

#include "simpleble/Adapter.h"
#include "simpleble/Peripheral.h"

#ifdef __linux__
	#include "./tools_linux.cpp"
#endif

#include <condition_variable>
#include <stdexcept>
#include <mutex>
#include <chrono>
#include <ranges>
#include <set>
#include <optional>
#include <vector>



namespace pybricksDriver
{
	BLEManager::BLEManager()
		: _logger(logging::createLogger("BLEManager"))
		  , _adapter(SimpleBLE::Adapter())
		  , _connectedDevice(SimpleBLE::Peripheral())
		  , _disconnecting(false)
		  , _nextOnDisconnectCallbackId(0) {
		if (const auto bluetoothEnabled = SimpleBLE::Safe::Adapter::bluetooth_enabled(); !bluetoothEnabled.has_value()) {
			throw std::runtime_error("Bluetooth is not enabled on this system.");
		}

		const auto adapters = SimpleBLE::Safe::Adapter::get_adapters();

		if (!adapters.has_value() || adapters->empty()) {
			throw std::runtime_error("No Bluetooth adapters found.");
		}
		_adapter = adapters->front();

		_logger->trace("Initialized with adapter: {}", _adapter.identifier().value_or("unknown"));
	}

	BLEManager::~BLEManager() {
		_adapter.scan_stop();
		disconnectDevice();

		_logger->info("Released");
	}

	bool BLEManager::connectToDevice(const std::string& name, int timeout_ms) {
		std::mutex foundMutex;
		std::condition_variable foundCondition;
		std::unique_lock lock(foundMutex);
		bool deviceFound = false;
		std::set<std::string> foundDevices;

		auto handleDevice = [&](SimpleBLE::Safe::Peripheral device) {
			if (!device.identifier().has_value() || !device.address().has_value()) {
				return;
			}

			const std::string deviceName = device.identifier().value();

			if (!foundDevices.contains(deviceName)) {
				_logger->trace("Found device: {} ({})", deviceName, device.address().value());
				foundDevices.insert(deviceName);
				if (deviceName == name){
					{
						std::lock_guard lockGuard(foundMutex);
						_connectedDevice = device;
						deviceFound = true;
					}
					_adapter.scan_stop();
					foundCondition.notify_one();
				}
			}
		};

		_adapter.set_callback_on_scan_updated(handleDevice);
		_adapter.set_callback_on_scan_found(handleDevice);

		#ifdef __linux__
		_logger->debug("Disconnect eventually stalled devices");
		bluetooth::disconnect_device(name, _logger);
		#endif

		_logger->debug("Checking for already paired devices");

		for (const auto& device : _adapter.get_paired_peripherals().value_or(std::vector<SimpleBLE::Safe::Peripheral>{})) {
			handleDevice(device);
		}


		if (!deviceFound) {
			_logger->debug("Scanning for devices with name: {} for up to {} ms", name, timeout_ms);
			_adapter.scan_start();
		}


		if (foundCondition.wait_for(lock, std::chrono::milliseconds(timeout_ms), [&] { return deviceFound; })) {
			if (_connectedDevice.is_connectable() && _connectedDevice.identifier().has_value() && _connectedDevice.address().has_value()) {
				_logger->info("Device found: {} ({})", _connectedDevice.identifier().value(), _connectedDevice.address().value());
				_logger->debug("Stopping scan and connecting to device");

				_adapter.scan_stop();

				_connectedDevice.set_callback_on_disconnected([this] {
					_logger->info("Device disconnected");
					for (const auto& callback : _onDisconnectCallbacks | std::views::values) {
						callback();
					}
				});

				_connectedDevice.connect();

				if (!_connectedDevice.is_connected().value_or(false) || !_connectedDevice.services().has_value()) {
					_logger->error("Failed to connect to device after it was found.");
					return false;
				}

				_logger->info("Connected to device");

				return true;
			}
		}

		_logger->debug("Failed to find device with name: {} within timeout of {} ms", name, timeout_ms);
		_adapter.scan_stop();

		return false;
	}

	void BLEManager::disconnectDevice() {
		if (!_disconnecting) {
			_disconnecting = true;
			_connectedDevice.disconnect();
			_logger->info("Device disconnected");
			_disconnecting = false;
		}
	}

	bool BLEManager::isConnected() {
		return !_disconnecting && _connectedDevice.is_connected().value_or(false);
	}

	void BLEManager::subscribeToNotifications(const std::string& serviceUUID, const std::string& characteristicUUID, const std::function<void(SimpleBLE::ByteArray payload)>& callback) {
		_logger->debug("Subscribing to notifications for service {} and characteristic {}", serviceUUID, characteristicUUID);

		if (_disconnecting || !_connectedDevice.is_connected().value_or(false)) {
			_logger->error("Cannot subscribe to notifications because no device is connected.");
			return;
		}

		if (!_connectedDevice.notify(serviceUUID, characteristicUUID, callback)) {
			_logger->error("Failed to subscribe to notifications for service {} and characteristic {}", serviceUUID, characteristicUUID);
			disconnectDevice();
		}
	}

	void BLEManager::unsubscribeFromNotifications(const std::string& serviceUUID, const std::string& characteristicUUID) {
		_logger->debug("Unsubscribing from notifications for service {} and characteristic {}", serviceUUID, characteristicUUID);

		if (_disconnecting || _connectedDevice.is_connected().value_or(false)) {
			if (!_connectedDevice.unsubscribe(serviceUUID, characteristicUUID)) {
				_logger->error("Failed to unsubscribe from notifications for service {} and characteristic {}", serviceUUID, characteristicUUID);
				disconnectDevice();
			}
		}
	}


	void BLEManager::writeToCharacteristic(const std::string& serviceUUID, const std::string& characteristicUUID, const SimpleBLE::ByteArray& data) {
		_logger->trace("Writing to characteristic {} of service {}: {}", characteristicUUID, serviceUUID, data.toHex(true));

		if (_disconnecting || !_connectedDevice.is_connected().value_or(false)) {
			_logger->error("Cannot write to characteristic because no device is connected.");
			return;
		}

		if (!_connectedDevice.write_request(serviceUUID, characteristicUUID, data)) {
			_logger->error("Failed to write to characteristic {} of service {}", characteristicUUID, serviceUUID);
			disconnectDevice();
		}
	}

	size_t BLEManager::bindOnDisconnect(std::function<void()> callback) {
		const size_t callbackId = _nextOnDisconnectCallbackId++;
		_onDisconnectCallbacks.emplace(callbackId, callback);
		return callbackId;
	}

	void BLEManager::unbindOnDisconnect(const size_t callbackId) {
		_onDisconnectCallbacks.erase(callbackId);
	}
}