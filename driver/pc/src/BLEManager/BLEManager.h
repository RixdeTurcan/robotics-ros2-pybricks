#pragma once

#include <pybricks-driver/logging.h>

#include <simpleble/Types.h>
#include <simpleble/AdapterSafe.h>
#include <simpleble/PeripheralSafe.h>

#include <string>
#include <functional>
#include <map>

namespace pybricksDriver
{
	class BLEManager
	{
		public:
			BLEManager();
			~BLEManager();

			bool connectToDevice(const std::string& name, int timeout_ms = 5000);
			void disconnectDevice();

			bool isConnected();

			void subscribeToNotifications(const std::string& serviceUUID, const std::string& characteristicUUID, const std::function<void(SimpleBLE::ByteArray payload)>& callback);
			void unsubscribeFromNotifications(const std::string& serviceUUID, const std::string& characteristicUUID);

			void writeToCharacteristic(const std::string& serviceUUID, const std::string& characteristicUUID, const SimpleBLE::ByteArray& data);

			size_t bindOnDisconnect(std::function<void()> callback) ;
			void unbindOnDisconnect(size_t callbackId);

		private:
			logging::Logger _logger;

			SimpleBLE::Safe::Adapter _adapter;
			SimpleBLE::Safe::Peripheral _connectedDevice;

			bool _disconnecting;

			std::map<size_t, std::function<void()>> _onDisconnectCallbacks;
			size_t _nextOnDisconnectCallbackId;
	};
}