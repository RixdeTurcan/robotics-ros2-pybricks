#pragma once

#include <mutex>

namespace pybricksDriver
{
	template<typename T> requires is_CommandOutType<T>
	void DriverFSM::forwardToHubIfActive(const T& commandOutData) {
		if (_driverStatus == DriverStatus_active) {
			std::lock_guard guard(_lock);
			_hub.sendCommand(commandOutData);
		}
	}

	template<typename T> requires is_CommandInType<T>
	void DriverFSM::forwardToPubSubIfActive(const T& commandInData) {
		if (_driverStatus == DriverStatus_active) {
			std::lock_guard guard(_lock);
			_pubSub.sendData(commandInData);
		}
	}
}