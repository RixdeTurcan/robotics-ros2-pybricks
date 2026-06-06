std_msgs/Header header

int64	delay	# ns	# Data delay from realtime

#RPY rotations
float64	rx	# rad		# Roll angle [0->2pi]
float64	ry	# rad		# Pich angle [0->2pi]
float64	rz	# rad		# Yaw angle [0->2pi]

#Gyro local frame velocities
float64	wx	# rad/s		# local x velocity
float64	wy	# rad/s		# local y velocity
float64	wz	# rad/s		# local z velocity

#Accelerometer accelerations
float64	ax	# m/s²		# Forward acceleration
float64	ay	# m/s²		# Left acceleration
float64	az	# m/s²		# Up acceleration