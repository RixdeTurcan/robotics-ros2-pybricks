#pragma once

#include <rclcpp/rclcpp.hpp>
#include <tf2_ros/transform_broadcaster.h>
#include <geometry_msgs/msg/transform_stamped.hpp>

#include <pybricks_ros2_interfaces/msg/pybricks_driver_hub_imu.hpp>

namespace pybricksDriverRos
{
	class RobotSimpleIMU : public rclcpp::Node
	{
		public:
			using IMUData = pybricks_ros2_interfaces::msg::PybricksDriverHubIMU;
		public:
			explicit RobotSimpleIMU();
			~RobotSimpleIMU() override;

		private:
			void onHubIMUData(const IMUData& data);

		private:
			tf2_ros::TransformBroadcaster _tfBroadcaster;

			rclcpp::Subscription<IMUData>::SharedPtr _sub_hubImu;

			geometry_msgs::msg::TransformStamped _worldToHub_tf;
			geometry_msgs::msg::Vector3 _hubAcc;
			geometry_msgs::msg::Vector3 _hubVel;
			geometry_msgs::msg::Vector3 _hubPos;
	};
}