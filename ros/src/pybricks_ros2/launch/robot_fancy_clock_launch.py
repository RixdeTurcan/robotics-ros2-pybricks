from launch import LaunchDescription
from launch.substitutions import PathJoinSubstitution

from launch_ros.actions import Node
from launch_ros.substitutions import FindPackageShare


def generate_launch_description():
	return LaunchDescription([
		Node(
			package='pybricks_ros2',
			executable='robot_fancy_clock',
			name='robot_fancy_clock',
			parameters=[
				PathJoinSubstitution([FindPackageShare('pybricks_ros2'), 'config', 'node_params.yaml'])
			],
		),
		#Node(
		#	package='rviz2',
		#	executable='rviz2',
		#	name='rviz2',
		#	arguments=[
		#		'-d', PathJoinSubstitution([FindPackageShare('pybricks_ros2'), 'robot', 'fancy_clock', 'rviz', 'robot.rviz'])
		#	],
		#),
		#Node(
		#	package='plotjuggler',
		#	executable='plotjuggler',
		#	name='plotjuggler',
		#	arguments=[
		#		'-n',
		#		'-l', PathJoinSubstitution([FindPackageShare('pybricks_ros2'), 'robot', 'fancy_clock', 'rviz', 'robot_plot.xml'])
		#	],
		#),
	])