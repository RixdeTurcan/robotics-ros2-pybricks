from launch import LaunchDescription
from launch.substitutions import PathJoinSubstitution

from launch_ros.actions import Node
from launch_ros.substitutions import FindPackageShare


def generate_launch_description():
	return LaunchDescription([
		Node(
			package='pybricks_ros2',
			executable='pybricks_driver_connector',
			name='pybricks_driver_connector',
			parameters=[
				PathJoinSubstitution([FindPackageShare('pybricks_ros2'), 'config', 'node_params.yaml'])
			],
		)
	])