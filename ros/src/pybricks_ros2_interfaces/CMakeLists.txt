cmake_minimum_required(VERSION 3.10)
project(pybricks_ros2_interfaces)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")
set(CMAKE_LOG_PATH "")

find_package(ament_cmake REQUIRED)
find_package(rosidl_default_generators REQUIRED)
find_package(std_msgs REQUIRED)

set(msg_files
	"msg/PybricksDriverHubIMU.msg"
	"msg/PybricksDriverHubButtons.msg"
	"msg/PybricksDriverHubLight55Matrix.msg"
	"msg/PybricksDriverHubSpeaker.msg"
	"msg/PybricksDriverHubSystem.msg"
	"msg/PybricksDriverForceSensor.msg"
	"msg/PybricksDriverColorSensor.msg"
	"msg/PybricksDriverUltrasonicSensor.msg"
	"msg/PybricksDriverMediumMotor.msg"
	"msg/PybricksDriverLargeMotor.msg"
	"msg/PybricksDriverLight33Matrix.msg"
	"msg/TimeSync.msg"
)

rosidl_generate_interfaces(${PROJECT_NAME} ${msg_files}
	DEPENDENCIES
	std_msgs
)

ament_export_dependencies(rosidl_default_runtime)
ament_package()
