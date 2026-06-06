#include <pybricks-driver/types.h>

#include "constant_internal.h"


namespace pybricksDriver
{
	int Ping_PubSubData::logLevel() const {return PYBRICKS_LOG_LEVEL_TRACE;}
	void Ping_PubSubData::printTitle(std::string& print) const {
		print += "PUB Ping";
	}
	void Ping_PubSubData::printData(std::string& print) const {
		Data_Base::printData(print);
	}


	int Pong_PubSubData::logLevel() const {return PYBRICKS_LOG_LEVEL_TRACE;}
	void Pong_PubSubData::printTitle(std::string& print) const {
		print += "PUB Pong";
	}
	void Pong_PubSubData::printData(std::string& print) const {
		Data_Base::printData(print);
	}
}
