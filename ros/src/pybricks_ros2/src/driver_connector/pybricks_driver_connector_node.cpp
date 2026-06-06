#include "PybricksDriverConnector/PybricksDriverConnector.h"

#include <rclcpp/rclcpp.hpp>
#include <pybricks-driver/logging.h>

#ifndef PYBRICKS_ROS2_LOG_PATH
	#define PYBRICKS_ROS2_LOG_PATH "./log/pybricks_ros2.log"
#endif


int main(const int argc, char * argv[])
{
	pybricksDriver::logging::initLogging(PYBRICKS_ROS2_LOG_PATH);

	const auto logger = pybricksDriver::logging::createLogger("Main");

	logger->info("Start Pybricks_driver_connector Node");

	rclcpp::init(argc, argv);
	rclcpp::spin(std::make_shared<pybricksDriverRos::PybricksDriverConnector>());
	rclcpp::shutdown();

	logger->info("Stop Pybricks_driver_connector Node");

	return 0;
}
