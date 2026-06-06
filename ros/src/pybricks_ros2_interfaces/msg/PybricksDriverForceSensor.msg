std_msgs/Header header

int64	delay	# ns	# Data delay from realtime

float64	f		# N		# Measured force
float64	d		# m		# Measured distance
bool    pressed			# True with pressed at 3N (configurable)
bool    touched			# True with a contact is detected (f can be 0)