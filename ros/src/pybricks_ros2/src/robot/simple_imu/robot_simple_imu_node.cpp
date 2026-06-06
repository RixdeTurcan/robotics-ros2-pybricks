#include "RobotSimpleIMU/RobotSimpleIMU.h"

#include <rclcpp/rclcpp.hpp>

int main(const int argc, char * argv[])
{
	rclcpp::init(argc, argv);
	rclcpp::spin(std::make_shared<pybricksDriverRos::RobotSimpleIMU>());
	rclcpp::shutdown();

	return 0;
}
