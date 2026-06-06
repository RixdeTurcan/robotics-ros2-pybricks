#pragma once

#include "BLEManager/BLEManager.h"
#include <pybricks-driver/logging.h>
#include "constant_internal.h"
#include "messaging.h"

#include <string>
#include <map>
#include <list>
#include <functional>
#include <thread>
#include <memory>
#include <mutex>

namespace pybricksDriver
{
	class HubDriver
	{
		public:
			struct Status {
				uint8_t selectedProgramId{};
				uint8_t runningProgramId{};

				bool batteryLowVoltage;
				bool batteryShuttingDown;
				bool batteryHighCurrent;
				bool bleDiscoverable;
				bool bleLowSignal;
				bool powerButtonPressed;
				bool programRunning;
				bool shuttingDown;

				bool shutdownRequested;
				bool bleConnected;

				bool hubReady;
			};

		public:
			explicit HubDriver(std::string deviceName = messaging::DEFAULT_DEVICE_NAME,
							   std::string serviceUUID = messaging::DEFAULT_SERVICE_UUID,
							   std::string characteristicUUID = messaging::DEFAULT_CHARACTERISTIC_UUID);
			~HubDriver();

			bool connect();
			void disconnect();

			void startProgram(std::optional<uint8_t> programId = std::nullopt);
			void stopProgram();

			template<typename T> requires is_CommandInType<T>
			size_t addHandler(const std::function<void(const T&)>& handler);

			template<typename T> requires is_CommandInType<T>
			void addOneTimeHandler(const std::function<void(const T&)>& handler);

			void removeHandler(size_t handlerId);

			template<typename T> requires is_CommandOutType<T>
			void sendCommand(const T& commandData);

			[[nodiscard]] bool isHubReady() const;

			size_t bindOnDisconnect(std::function<void()>& callback) ;
			void unbindOnDisconnect(size_t callbackId);

			bool isConnected();
			[[nodiscard]] bool isProgramRunning() const;
			[[nodiscard]] uint8_t getRunningProgramId() const;

			template<typename T> requires is_CommandInType<T>
			void applyEventModifier(T&);


		private:
			void _onNotificationReceived(const SimpleBLE::ByteArray& data);

			template<typename T> requires is_CommandInOutType<T>
			messaging::CommandHandler castToCommandHandler(const std::function<void(const T&)>& handler);


		private:
			std::string _deviceName;
			std::string _serviceUUID;
			std::string _characteristicUUID;

			BLEManager _bleManager;

			logging::Logger _logger;

			Status _status;

			SimpleBLE::ByteArray _dataBuffer;
			SimpleBLE::ByteArray _printBuffer;

			std::map<size_t, messaging::CommandHandler> _handlers;
			std::list<messaging::CommandHandler> _oneTimeHandlers;
			size_t _nextHandlerId;

			uint16_t _timeSyncId;
			uint64_t _lastHubTimestampNs;

			std::mutex _eventModifierLock;

			std::chrono::steady_clock::time_point _pingTimeout;

			std::unique_ptr<std::jthread> _pingTaskThread;
	};
}

#include "HubDriver.hpp"