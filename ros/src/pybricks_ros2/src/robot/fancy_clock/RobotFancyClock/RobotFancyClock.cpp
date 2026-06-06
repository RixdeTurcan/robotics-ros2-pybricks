#include "RobotFancyClock.h"

#include <tf2/LinearMath/Quaternion.hpp>

#include <format>
#include <regex>
#include <string>

using namespace std::chrono_literals;


namespace pybricksDriverRos
{
	RobotFancyClock::RobotFancyClock()
		:Node("robot_simple_imu")
		,_tfBroadcaster(this)
	{
		RCLCPP_INFO(this->get_logger(), "Starting robot_simple_imu");

		_worldToHub_tf.header.frame_id = "world";
		_worldToHub_tf.child_frame_id = "hub";

		const std::string hubIMUTopic = "PybricksDriver/HUB/Hub_IMU";
		size_t tryCount = 0;

		RCLCPP_INFO(this->get_logger(), "Discovering topics..");
		while (true) {
			const auto& topics = this->get_topic_names_and_types();

			if (topics.contains("/" + hubIMUTopic)) {
				RCLCPP_INFO(this->get_logger(), "Topic /%s found", hubIMUTopic.c_str());
				break;
			}

			if (tryCount > 10) {
				const std::string errMessage = std::format("Topic /{} not found", hubIMUTopic);
				RCLCPP_ERROR(this->get_logger(), "%s", errMessage.c_str());
				throw std::runtime_error(errMessage);
			}

			++tryCount;
			rclcpp::sleep_for(200ms);
		}

		_sub_hubImu = this->create_subscription<IMUData>(
			hubIMUTopic,
			rclcpp::SensorDataQoS(),
			[this](const IMUData::UniquePtr& data){onHubIMUData(*data);}
		);
	}

	RobotFancyClock::~RobotFancyClock() {
		RCLCPP_INFO(this->get_logger(), "Ending robot_simple_imu");
	}

	void RobotFancyClock::onHubIMUData(const IMUData& data) {
		const auto tNow = get_clock()->now();
		_worldToHub_tf.header.stamp = tNow;

		tf2::Quaternion qRot;
		qRot.setRPY(data.rx, data.ry, data.rz);
		_worldToHub_tf.transform.rotation.x = qRot.x();
		_worldToHub_tf.transform.rotation.y = qRot.y();
		_worldToHub_tf.transform.rotation.z = qRot.z();
		_worldToHub_tf.transform.rotation.w = qRot.w();

		_tfBroadcaster.sendTransform(_worldToHub_tf);
	}
}