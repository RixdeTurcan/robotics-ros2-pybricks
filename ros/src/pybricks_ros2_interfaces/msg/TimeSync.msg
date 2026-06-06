std_msgs/Header header

uint16	id				# Synchronisation Id (It can change if there is multiple hubs or a hub reset)
uint64	delay	# ns	# Measured hub -> pybricksDriverConnector node time average delay
uint64	offset	# ns	# Offset from hub local time to node time