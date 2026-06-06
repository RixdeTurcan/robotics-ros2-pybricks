#pragma once

#include <rclcpp/rclcpp.hpp>

#include <pybricks-driver/PubSub.h>

#include <pybricks_ros2_interfaces/msg/pybricks_driver_hub_imu.hpp>
#include <pybricks_ros2_interfaces/msg/pybricks_driver_hub_buttons.hpp>
#include <pybricks_ros2_interfaces/msg/pybricks_driver_hub_system.hpp>
#include <pybricks_ros2_interfaces/msg/pybricks_driver_hub_light55_matrix.hpp>
#include <pybricks_ros2_interfaces/msg/pybricks_driver_hub_speaker.hpp>
#include <pybricks_ros2_interfaces/msg/pybricks_driver_force_sensor.hpp>
#include <pybricks_ros2_interfaces/msg/pybricks_driver_color_sensor.hpp>
#include <pybricks_ros2_interfaces/msg/pybricks_driver_ultrasonic_sensor.hpp>
#include <pybricks_ros2_interfaces/msg/pybricks_driver_large_motor.hpp>
#include <pybricks_ros2_interfaces/msg/pybricks_driver_medium_motor.hpp>
#include <pybricks_ros2_interfaces/msg/pybricks_driver_light33_matrix.hpp>
#include <pybricks_ros2_interfaces/msg/time_sync.hpp>

#include <vector>
#include <map>
#include <mutex>
#include <deque>


namespace pybricksDriverRos
{
	using HubIMUData = pybricks_ros2_interfaces::msg::PybricksDriverHubIMU;
	using HubButtonsData = pybricks_ros2_interfaces::msg::PybricksDriverHubButtons;
	using HubSystemData = pybricks_ros2_interfaces::msg::PybricksDriverHubSystem;
	using HubSpeakerData = pybricks_ros2_interfaces::msg::PybricksDriverHubSpeaker;
	using HubLight55MatrixData = pybricks_ros2_interfaces::msg::PybricksDriverHubLight55Matrix;
	using ForceSensorData = pybricks_ros2_interfaces::msg::PybricksDriverForceSensor;
	using ColorSensorData = pybricks_ros2_interfaces::msg::PybricksDriverColorSensor;
	using UltrasonicSensorData = pybricks_ros2_interfaces::msg::PybricksDriverUltrasonicSensor;
	using MediumMotorData = pybricks_ros2_interfaces::msg::PybricksDriverMediumMotor;
	using LargeMotorData = pybricks_ros2_interfaces::msg::PybricksDriverLargeMotor;
	using Light33MatrixData = pybricks_ros2_interfaces::msg::PybricksDriverLight33Matrix;

	using PublisherHubIMU = rclcpp::Publisher<HubIMUData>::SharedPtr;
	using PublisherHubButtons = rclcpp::Publisher<HubButtonsData>::SharedPtr;
	using PublisherHubSystem = rclcpp::Publisher<HubSystemData>::SharedPtr;
	using PublisherHubSpeaker = rclcpp::Publisher<HubSpeakerData>::SharedPtr;
	using PublisherHubLight55Matrix = rclcpp::Publisher<HubLight55MatrixData>::SharedPtr;
	using PublisherForceSensor = rclcpp::Publisher<ForceSensorData>::SharedPtr;
	using PublisherColorSensor = rclcpp::Publisher<ColorSensorData>::SharedPtr;
	using PublisherUltrasonicSensor = rclcpp::Publisher<UltrasonicSensorData>::SharedPtr;
	using PublisherMediumMotor = rclcpp::Publisher<MediumMotorData>::SharedPtr;
	using PublisherLargeMotor = rclcpp::Publisher<LargeMotorData>::SharedPtr;
	using PublisherLight33Matrix = rclcpp::Publisher<Light33MatrixData>::SharedPtr;

	using PublisherVariant = std::variant<
		PublisherHubIMU, PublisherHubButtons, PublisherHubSystem, PublisherHubSpeaker, PublisherHubLight55Matrix,
		PublisherForceSensor, PublisherColorSensor, PublisherUltrasonicSensor,
		PublisherMediumMotor, PublisherLargeMotor, PublisherLight33Matrix
	>;

	class PybricksDriverConnector : public rclcpp::Node
	{
		public:
			enum DriverState
			{
				DriverState_disconnected = 0,
				DriverState_registered = 1,
				DriverState_gotCapabilities = 2,
				DriverState_listening = 3,
			};

			struct LoopData
			{
				bool active{false};
				rclcpp::Time timeout;
				pybricksDriver::StartListening_OutData outData;
			};

			struct TimeSyncData
			{
				rclcpp::Time nodeSend_nodeTime;
				rclcpp::Time hubRecv_hubTime;
				rclcpp::Time nodeRecv_nodeTime;
			};

			struct TimeSync
			{
				explicit TimeSync();

				std::deque<TimeSyncData> data;
				rclcpp::Duration hubToNodeOffset;
				rclcpp::Duration hubToNodeDelay;
				uint16_t lastHubTimeNs; //Used to handle uint16 overflow
			};

		public:
			explicit PybricksDriverConnector();
			~PybricksDriverConnector() override;

		private:
			void processPingTask();
			void processTimeSyncTask();

			void onPubSubPong(const pybricksDriver::Pong_PubSubData& data);

			void onDriverInData(const pybricksDriver::Data_InData& data);

			void onHubCapabilities(const pybricksDriver::CommandResponse_GetCapabilities_InData& data);

			void onTimeSync(const pybricksDriver::CommandResponse_TimeSync_InData& data);

			const PublisherVariant& getPublisher(uint8_t sensorLocation, uint8_t sensorType);

		private:
			std::map<uint8_t, std::map<uint8_t, PublisherVariant>> _rosPublisher; // sensor location -> sensor type -> publisher
			rclcpp::Publisher<pybricks_ros2_interfaces::msg::TimeSync>::SharedPtr _timeSyncPublisher;

			std::unique_ptr<pybricksDriver::PubSub> _pubSub;
			pybricksDriver::CommandResponse_GetCapabilities_InData _hubCapabilities;

			std::vector<size_t> _pubSubHandlerTokens;
			DriverState _driverState;

			std::unique_ptr<std::jthread> _pingTaskThread;
			std::unique_ptr<std::jthread> _testTaskThread;
			std::unique_ptr<std::jthread> _timeSyncTaskThread;

			rclcpp::Time _pingTimeout;

			std::vector<LoopData> _loops;

			std::map<uint16_t, TimeSync> _timeSync;

			std::mutex _lock;
	};
}

