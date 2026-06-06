#pragma once

#include "msgpack_compat.h"
#include "commands.h"

namespace pybricksDriver
{
	struct Ping_PubSubData : Data_Base
	{
		explicit Ping_PubSubData() = default;

		PYBRICKS_PRINT(Ping_PubSubData);
		PYBRICKS_MSGPACK_DEFINE(
		);
	};

	struct Pong_PubSubData : Data_Base
	{
		explicit Pong_PubSubData() = default;

		PYBRICKS_PRINT(Pong_PubSubData);
		PYBRICKS_MSGPACK_DEFINE(
		);
	};

}