#pragma once

#include <cstdint>
#include <vector>


namespace pybricksDriver
{
	struct SensorData_base
	{
		SensorData_base() = default;
	};

	struct SensorData_HubIMU: SensorData_base
	{
		// IMU acceleration in m/s²
		double ax;
		double ay;
		double az;

		// IMU angular velocity in rad/s
		double wx;
		double wy;
		double wz;

		// IMU rotation in rad
		double rx;
		double ry;
		double rz;

		SensorData_HubIMU(const uint8_t& type, const std::vector<uint8_t>& payload);
	};

	struct SensorData_ForceSensor: SensorData_base
	{
		double f; // Measured force in N (max 10N)
		double d; // Measured distance in m (max 0.008m)
		bool pressed; //Trigger at default 3N of force
		bool touched; //Can trigger even if f = 0

		SensorData_ForceSensor(const uint8_t& type, const std::vector<uint8_t>& payload);
	};

	struct SensorData_ColorSensor: SensorData_base
	{
		int h; // Color hue in °
		int s; // Color saturation in %
		int v; // Color value in %
		int reflection; // Measuread reflection luminance in %
		int ambiant; // Measured ambiant luminance in %

		SensorData_ColorSensor(const uint8_t& type, const std::vector<uint8_t>& payload);
	};

	struct SensorData_UltrasonicSensor: SensorData_base
	{
		double d; // Measured distance in m
		bool isInf; //True if there is no measured distance (open field)

		SensorData_UltrasonicSensor(const uint8_t& type, const std::vector<uint8_t>& payload);
	};

	struct SensorData_Light33Matrix: SensorData_base
	{
		SensorData_Light33Matrix(const uint8_t& type, const std::vector<uint8_t>& payload);
	};

	struct SensorData_Motor: SensorData_base
	{
		double r; //Angular position of the motor in rad
		double v; //Angular velocity of the motor in rad/s
		double t; //Torque of the motor in Nm

		SensorData_Motor(const uint8_t& type, const std::vector<uint8_t>& payload);
	};

	struct SensorData_MediumMotor: SensorData_Motor
	{
		SensorData_MediumMotor(const uint8_t& type, const std::vector<uint8_t>& payload);
	};

	struct SensorData_LargeMotor: SensorData_Motor
	{
		SensorData_LargeMotor(const uint8_t& type, const std::vector<uint8_t>& payload);
	};

	struct SensorData_HubButtons: SensorData_base
	{
		bool left; // True if pressed
		bool center; // True if pressed
		bool right; // True if pressed
		bool bluetooth; // True if pressed

		SensorData_HubButtons(const uint8_t& type, const std::vector<uint8_t>& payload);
	};

	struct SensorData_HubSystem: SensorData_base
	{
		int capacity; // Remaining % of battery
		double current; // current load in A (max measured of 1A)

		SensorData_HubSystem(const uint8_t& type, const std::vector<uint8_t>& payload);
	};

	struct SensorData_HubSpeaker: SensorData_base
	{
		SensorData_HubSpeaker(const uint8_t& type, const std::vector<uint8_t>& payload);
	};
	struct SensorData_HubLight55Matrix: SensorData_base
	{
		SensorData_HubLight55Matrix(const uint8_t& type, const std::vector<uint8_t>& payload);
	};
}