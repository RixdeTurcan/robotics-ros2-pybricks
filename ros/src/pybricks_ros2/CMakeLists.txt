cmake_minimum_required(VERSION 3.10)
project(pybricks_ros2)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SRC_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../..)
set(INSTALL_PATH ${CMAKE_INSTALL_PREFIX})
set(WORKSPACE_NAME "src")
set(LOG_PATH "${CMAKE_LOG_PATH}/pybricks_ros2/")

find_package(pybricks-driver CONFIG REQUIRED)
find_package(pybricks_ros2_interfaces REQUIRED)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(tf2 REQUIRED)
find_package(tf2_ros REQUIRED)
find_package(std_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(std_msgs REQUIRED)

add_compile_definitions(
    PYBRICKS_ROS2_LOG_PATH="${LOG_PATH}/pybricks_driver_connector.log"
    PYBRICKS_LOG_LEVEL=3
)

set(executables "")
set(robots "")

####Executable pybricks_driver_connector####

list(APPEND executables pybricks_driver_connector)

add_executable(pybricks_driver_connector
    ${CMAKE_CURRENT_SOURCE_DIR}/src/driver_connector/pybricks_driver_connector_node.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/driver_connector/PybricksDriverConnector/PybricksDriverConnector.cpp
)

target_link_libraries(pybricks_driver_connector
    PRIVATE
    pybricks-driver::pubsub
    ${rclcpp_TARGETS}
    ${pybricks_ros2_interfaces_TARGETS}
)

####Executable robot_simple_imu####

list(APPEND executables robot_simple_imu)
list(APPEND robots simple_imu)

add_executable(robot_simple_imu
    ${CMAKE_CURRENT_SOURCE_DIR}/src/robot/simple_imu/robot_simple_imu_node.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/robot/simple_imu/RobotSimpleIMU/RobotSimpleIMU.cpp
)

target_link_libraries(robot_simple_imu
    PRIVATE
    ${rclcpp_TARGETS}
    ${std_msgs_TARGETS}
    ${geometry_msgs_TARGETS}
    ${tf2_TARGETS}
    ${tf2_ros_TARGETS}
    ${pybricks_ros2_interfaces_TARGETS}
)

####Executable robot_fancy_clock####

list(APPEND executables robot_fancy_clock)
list(APPEND robots fancy_clock)

add_executable(robot_fancy_clock
    ${CMAKE_CURRENT_SOURCE_DIR}/src/robot/fancy_clock/robot_fancy_clock_node.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/robot/fancy_clock/RobotFancyClock/RobotFancyClock.cpp
)

target_link_libraries(robot_fancy_clock
    PRIVATE
    ${rclcpp_TARGETS}
    ${std_msgs_TARGETS}
    ${geometry_msgs_TARGETS}
    ${tf2_TARGETS}
    ${tf2_ros_TARGETS}
    ${pybricks_ros2_interfaces_TARGETS}
)

####Install all executables and launch scripts####

foreach(exe ${executables})
  set(EXECUTABLE_NAME ${exe})

  if(TARGET ${exe})
    install(TARGETS ${exe} DESTINATION lib/${PROJECT_NAME})
  endif()

  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/../../etc/run_node.zsh.in
    ${CMAKE_CURRENT_BINARY_DIR}/run_${exe}.zsh
    @ONLY
  )
  install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/run_${exe}.zsh
    DESTINATION lib/${PROJECT_NAME}
  )
endforeach()

install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/launch
    DESTINATION share/${PROJECT_NAME}/
)

install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/config
    DESTINATION share/${PROJECT_NAME}/
)


####Install rviz robots####

foreach(robot ${robots})

  install(
      DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src/robot/${robot}/rviz/meshes
      DESTINATION share/${PROJECT_NAME}/robot/${robot}/rviz
  )

  set(GENERATED_URDF_IN  ${CMAKE_CURRENT_SOURCE_DIR}/src/robot/${robot}/rviz/robot.urdf)
  set(GENERATED_URDF_OUT ${CMAKE_CURRENT_BINARY_DIR}/robot/${robot}/rviz/robot.urdf)
  set(INSTALLED_URDF_PATH "${CMAKE_INSTALL_PREFIX}/share/${PROJECT_NAME}/robot/${robot}/rviz/robot.urdf")
  file(READ "${GENERATED_URDF_IN}" URDF_CONTENTS)
  string(REPLACE
      "<mesh filename=\"meshes"
      "<mesh filename=\"package://${PROJECT_NAME}/robot/${robot}/rviz/meshes"
      URDF_CONTENTS
      "${URDF_CONTENTS}"
  )
  file(WRITE "${GENERATED_URDF_OUT}" "${URDF_CONTENTS}")
  install(
      FILES "${GENERATED_URDF_OUT}"
      DESTINATION share/${PROJECT_NAME}/robot/${robot}/rviz
  )

  set(GENERATED_RVIZ_IN  ${CMAKE_CURRENT_SOURCE_DIR}/src/robot/${robot}/rviz/robot.rviz.in)
  set(GENERATED_RVIZ_OUT ${CMAKE_CURRENT_BINARY_DIR}/robot/${robot}/rviz/robot.rviz)
  file(READ "${GENERATED_RVIZ_IN}" RVIZ_CONFIG_CONTENTS)
  string(REGEX REPLACE
      "([ ]*Description File: )[^\n]+"
        "\\1${INSTALLED_URDF_PATH}"
      RVIZ_CONFIG_CONTENTS
        "${RVIZ_CONFIG_CONTENTS}"
  )
  file(WRITE "${GENERATED_RVIZ_OUT}" "${RVIZ_CONFIG_CONTENTS}")
  install(
      FILES "${GENERATED_RVIZ_OUT}"
      DESTINATION share/${PROJECT_NAME}/robot/${robot}/rviz
  )

  install(
      FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/robot/${robot}/rviz/robot_plot.xml
      DESTINATION share/${PROJECT_NAME}/robot/${robot}/rviz
  )
endforeach()

ament_package()
