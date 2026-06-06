#pragma once

#include <cstdint>
#include <vector>


namespace pybricksDriver::Actuator
{
	namespace MediumMotor
	{
		std::vector<uint8_t> moveToAngle(
			double targetAngle,
			double maxVelocity,
			bool stopAtEnd,
			bool lockAtEnd
		);
	}

	namespace LargeMotor
	{
		std::vector<uint8_t> moveToAngle(
			double targetAngle,
			double maxVelocity,
			bool stopAtEnd,
			bool lockAtEnd
		);
	}
}