#include <pybricks-driver/types.h>

#include "tools/tools.h"
#include "HubDriver/constant_internal.h"

#include <cmath>



namespace pybricksDriver::Actuator
{
	namespace detail
	{
		std::vector<uint8_t> moveToAngle(
			const double targetAngle,
			const double maxVelocity,
			const bool stopAtEnd,
			const bool lockAtEnd
		) {
			std::vector<uint8_t> commandData;
			commandData.reserve(6);
			auto targetAngleBytes = tools::splitU16LE(static_cast<int>(std::round(targetAngle * convert::RAD_TO_DEG)) % 360);
			auto maxVelocityBytes = tools::splitU16LE(static_cast<int>(std::round(maxVelocity * convert::RAD_TO_DEG)));

			uint8_t stopBehavior = 0x00;
			if (stopAtEnd) {
				if (lockAtEnd) {
					stopBehavior = 2;
				}else{
					stopBehavior = 0;
				}
			}else{
				stopBehavior = 3;
			}

			commandData.push_back(0x00);
			commandData.insert(commandData.end(), targetAngleBytes.begin(), targetAngleBytes.end());
			commandData.insert(commandData.end(), maxVelocityBytes.begin(), maxVelocityBytes.end());
			commandData.push_back(stopBehavior);

			return commandData;
		}
	}

	namespace MediumMotor
	{
		std::vector<uint8_t> moveToAngle(
			const double targetAngle,
			const double maxVelocity,
			const bool stopAtEnd,
			const bool lockAtEnd
		) {
			return detail::moveToAngle(targetAngle, maxVelocity, stopAtEnd, lockAtEnd);
		}
	}

	namespace LargeMotor
	{
		std::vector<uint8_t> moveToAngle(
			const double targetAngle,
			const double maxVelocity,
			const bool stopAtEnd,
			const bool lockAtEnd
		) {
			return detail::moveToAngle(targetAngle, maxVelocity, stopAtEnd, lockAtEnd);
		}
	}
}
