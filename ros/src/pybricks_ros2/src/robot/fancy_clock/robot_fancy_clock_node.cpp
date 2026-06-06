#include "RobotFancyClock/RobotFancyClock.h"

#include <rclcpp/rclcpp.hpp>

int main(const int argc, char * argv[])
{
	rclcpp::init(argc, argv);
	rclcpp::spin(std::make_shared<pybricksDriverRos::RobotFancyClock>());
	rclcpp::shutdown();

	return 0;
}
