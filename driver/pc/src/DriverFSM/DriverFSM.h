#pragma once

#include <pybricks-driver/PubSub.h>
#include <pybricks-driver/types.h>
#include <pybricks-driver/logging.h>

#include "HubDriver/HubDriver.h"

#include <mutex>
#include <vector>
#include <thread>
#include <chrono>


namespace pybricksDriver
{
	class DriverFSM
	{
		public:
			enum DriverStatus
			{
				DriverStatus_inactive,
				DriverStatus_waiting_for_connector,
				DriverStatus_active
			};

		public:
			explicit DriverFSM(HubDriver& hub, PubSub& pubSub);
			~DriverFSM();

			void processFSM(std::optional<uint8_t> programId);

		private:
			void processPingTask();

			void onPubSubPing(const Ping_PubSubData& data);

			template<typename T> requires is_CommandOutType<T>
			void forwardToHubIfActive(const T& commandOutData);

			template<typename T> requires is_CommandInType<T>
			void forwardToPubSubIfActive(const T& commandInData);

		private:
			HubDriver& _hub;
			PubSub& _pubSub;

			logging::Logger _logger;

			DriverStatus _driverStatus;
			std::chrono::steady_clock::time_point _pingTimeout;

			std::vector<size_t> _hubHandlerTokens;
			std::vector<size_t> _pubSubHandlerTokens;

			std::unique_ptr<std::jthread> _pingTaskThread;

			std::mutex _lock;
	};
}

#include "DriverFSM.hpp"
