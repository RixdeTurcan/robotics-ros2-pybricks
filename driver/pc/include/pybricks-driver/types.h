#pragma once

#include <tuple>
#include <variant>
#include <utility>

// ReSharper disable once CppUnusedIncludeDirective
#include "constant.h"
#include "commands.h"
#include "events.h"
#include "sensordata.h"
// ReSharper disable once CppUnusedIncludeDirective
#include "actuatordata.h"
#include "commands_pubsub.h"

#define PYBRICKS_LOG_LEVEL_TRACE 4
#define PYBRICKS_LOG_LEVEL_DEBUG 3
#define PYBRICKS_LOG_LEVEL_INFO 2
#define PYBRICKS_LOG_LEVEL_WARNING 1
#define PYBRICKS_LOG_LEVEL_ERROR 0

#ifndef PYBRICKS_LOG_LEVEL
	#define PYBRICKS_LOG_LEVEL 2
#endif

namespace pybricksDriver
{
	namespace detail
	{
		/* When adding a new command :
		 * - Add definition in events.h, commands.h or commands_pubsub.h
		 * - Implement it in events.cpp, commands.cpp or commands_pubsub.cpp
		 * - Add the Type in one of the following tuples: CommandInTypes, CommandOutTypes or CommandPubSubTypes
		 * - If it's a message IN or OUT:
		 *   - In the hub constants.py and driver constant.h, add the command number
		 *   - Implement the effect on the Hub in Python
		 * - If it's a message IN:
		 *   - Add the conversion case from Event raw data to the CommandInData Type in getCommandInDataFromEvent (messaging.cpp)
		 * - If it's not internal to the hub<->driver communication:
		 *   - Handle the command effect or forwarding in the constructor of DriverFSM.cpp
		 */
		using CommandInTypes = std::tuple<
			HubReady_InData,
			Data_InData,
			CommandResponse_Ping_InData,
			CommandResponse_StartListening_InData,
			CommandResponse_StopListening_InData,
			CommandResponse_StopAllListening_InData,
			CommandResponse_GetCapabilities_InData,
			CommandResponse_TimeSync_InData,
			CommandResponse_Actuator_InData
		>;

		using CommandOutTypes = std::tuple<
			Ping_OutData,
			StartListening_OutData,
			StopListening_OutData,
			StopAllListening_OutData,
			GetCapabilities_OutData,
			TimeSync_OutData,
			Actuator_OutData
		>;

		using CommandPubSubTypes = std::tuple<
			Ping_PubSubData,
			Pong_PubSubData
		>;

		using SensorDataTypes = std::tuple<
			SensorData_HubIMU,
			SensorData_HubButtons,
			SensorData_HubSystem,
			SensorData_HubSpeaker,
			SensorData_HubLight55Matrix,
			SensorData_ForceSensor,
			SensorData_ColorSensor,
			SensorData_UltrasonicSensor,
			SensorData_MediumMotor,
			SensorData_LargeMotor,
			SensorData_Light33Matrix
		>;

		template<typename Tuple>
		struct TupleToVariant;

		template<typename... Ts>
		struct TupleToVariant<std::tuple<Ts...>> {
			using type = std::variant<Ts...>;
		};

		template<typename Tuple>
		// ReSharper disable once CppRedundantTypenameKeyword
		using TupleToVariant_t = typename TupleToVariant<Tuple>::type;

		template <typename T, typename V>
		constexpr bool is_in_variant = false;

		template <typename T, typename... Ts>
		constexpr bool is_in_variant<T, std::variant<Ts...>> = (std::is_same_v<T, Ts> || ...);
	}

	using CommandInData = detail::TupleToVariant_t<detail::CommandInTypes>;
	using CommandOutData = detail::TupleToVariant_t<detail::CommandOutTypes>;
	using CommandPubSubData = detail::TupleToVariant_t<detail::CommandPubSubTypes>;
	using CommandData = detail::TupleToVariant_t<
		decltype(std::tuple_cat(
			std::declval<detail::CommandInTypes>(),
			std::declval<detail::CommandOutTypes>(),
			std::declval<detail::CommandPubSubTypes>()
		))
	>;
	using CommandInOutData = detail::TupleToVariant_t<
		decltype(std::tuple_cat(
			std::declval<detail::CommandInTypes>(),
			std::declval<detail::CommandOutTypes>()
		))
	>;
	using SensorData = detail::TupleToVariant_t<detail::SensorDataTypes>;


	template <typename T>
	constexpr bool is_CommandInType =
	detail::is_in_variant<T, CommandInData>;

	template <typename T>
	constexpr bool is_CommandOutType =
	detail::is_in_variant<T, CommandOutData>;

	template <typename T>
	constexpr bool is_CommandPubSubType =
		detail::is_in_variant<T, CommandPubSubData>;

	template <typename T>
	constexpr bool is_CommandInOutType =
	detail::is_in_variant<T, CommandInOutData>;

	template <typename T>
	constexpr bool is_CommandType =
	detail::is_in_variant<T, CommandData>;

	template <typename T>
	constexpr bool is_SensorData =
	detail::is_in_variant<T, SensorData>;
}
