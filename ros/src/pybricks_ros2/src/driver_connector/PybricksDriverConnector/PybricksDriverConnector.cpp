#include "PybricksDriverConnector.h"

#include <chrono>
#include <regex>
#include <string>
#include <algorithm>
#include <format>
#include <optional>
#include <ranges>
#include <set>
#include <tuple>
#include <numbers>

using namespace std::chrono_literals;

namespace pybricksDriverRos
{

	PybricksDriverConnector::TimeSync::TimeSync()
		:hubToNodeOffset(0s)
		,hubToNodeDelay(0s)
		,lastHubTimeNs(0)
	{}

	PybricksDriverConnector::PybricksDriverConnector()
		:Node("pybricks_driver_connector")
		,_driverState(DriverState_disconnected)
		,_pingTimeout(get_clock()->now())
	{
		RCLCPP_INFO(get_logger(), "Starting pybricks_driver_connector");

		_timeSyncPublisher = create_publisher<pybricks_ros2_interfaces::msg::TimeSync>("PybricksDriver/TimeSync", rclcpp::SensorDataQoS());

		auto pubDesc = rcl_interfaces::msg::ParameterDescriptor{};
		pubDesc.description = "Pybricks driver PubSub publisher URI";
		declare_parameter("driver_pubsub_pub_uri", "tcp://127.0.0.1:5555", pubDesc);
		const auto driverPubSubPubURI = get_parameter("driver_pubsub_pub_uri").as_string();
		RCLCPP_INFO(get_logger(), "driver_pubsub_pub_uri: %s", driverPubSubPubURI.c_str());

		auto subDesc = rcl_interfaces::msg::ParameterDescriptor{};
		subDesc.description = "Pybricks driver PubSub subscriber URI";
		declare_parameter("driver_pubsub_sub_uri", "tcp://127.0.0.1:5556", subDesc);
		const auto driverPubSubSubURI = get_parameter("driver_pubsub_sub_uri").as_string();
		RCLCPP_INFO(get_logger(), "driver_pubsub_sub_uri: %s", driverPubSubSubURI.c_str());

		const auto now = get_clock()->now();
		_loops.emplace_back(LoopData{true, now, pybricksDriver::StartListening_OutData(
			0,
			true, false, false,
			false, false,
			true, true, true, true, true, true,
			1
			)});
		_loops.emplace_back(LoopData{true, now, pybricksDriver::StartListening_OutData(
			1,
			false, true, false,
			false, false,
			false, false, false, false, false, false,
			10
			)});
		_loops.emplace_back(LoopData{true, now, pybricksDriver::StartListening_OutData(
			2,
			false, false, true,
			false, false,
			false, false, false, false, false, false,
			10
			)});
		_loops.emplace_back(LoopData{false, now, pybricksDriver::StartListening_OutData(
			3,
			false, false, false,
			false, false,
			false, false, false, false, false, false,
			255
		)});

		_pubSub = std::make_unique<pybricksDriver::PubSub>(driverPubSubPubURI, driverPubSubSubURI);

		_pingTaskThread = std::make_unique<std::jthread>([this](const std::stop_token& stopToken) {
			while (!stopToken.stop_requested()) {
				processPingTask();
				std::this_thread::sleep_for(500ms);
			}
		});

		_testTaskThread = std::make_unique<std::jthread>([this](const std::stop_token& stopToken) {
			double angle = 0;
			while (!stopToken.stop_requested()) {
				_pubSub->sendData(pybricksDriver::Actuator_OutData{
					pybricksDriver::SENSOR_TYPE_MEDIUM_MOTOR,
					pybricksDriver::SENSOR_LOCATION_F,
					pybricksDriver::Actuator::MediumMotor::moveToAngle(angle, 20, true, false)
				});
				std::this_thread::sleep_for(1000ms);
				angle += 2*3.14/10;
			}
		});

		_timeSyncTaskThread = std::make_unique<std::jthread>([this](const std::stop_token& stopToken) {
			while (!stopToken.stop_requested()) {
				processTimeSyncTask();
				std::this_thread::sleep_for(300ms);
			}
		});

		_pubSubHandlerTokens.emplace_back(_pubSub->addHandler<pybricksDriver::Pong_PubSubData>([this](const auto& data){onPubSubPong(data);}));
		_pubSubHandlerTokens.emplace_back(_pubSub->addHandler<pybricksDriver::Data_InData>([this](const auto& data){onDriverInData(data);}));
		_pubSubHandlerTokens.emplace_back(_pubSub->addHandler<pybricksDriver::CommandResponse_GetCapabilities_InData>([this](const auto& data){onHubCapabilities(data);}));
		_pubSubHandlerTokens.emplace_back(_pubSub->addHandler<pybricksDriver::CommandResponse_TimeSync_InData>([this](const auto& data){onTimeSync(data);}));
	}

	PybricksDriverConnector::~PybricksDriverConnector() {
		RCLCPP_INFO(get_logger(), "Ending pybricks_driver_connector");

		for (const auto& token:_pubSubHandlerTokens) {
			_pubSub->removeHandler(token);
		}

		_pingTaskThread->request_stop();
	}

	void PybricksDriverConnector::processPingTask() {
		std::lock_guard guard(_lock);

		const auto now = get_clock()->now();

		if (_driverState > DriverState_disconnected && now >= _pingTimeout) {
			_driverState = DriverState_disconnected;
			RCLCPP_INFO(get_logger(), "Driver disconnected (timeout)");
		}

		if (_driverState == DriverState_registered) {
			_pubSub->sendData(pybricksDriver::GetCapabilities_OutData{});
		}

		if (_driverState == DriverState_gotCapabilities) {
			_driverState = DriverState_listening;

			for (auto& loop: _loops) {
				if (loop.active) {
					loop.timeout = now + std::max(loop.outData.loopPeriod10Ms * 10ms * 5, 500ms);
					_pubSub->sendData(loop.outData);
				}
			}

			RCLCPP_INFO(get_logger(), "Hub sending data");
		}

		if (_driverState == DriverState_listening) {
			for (auto& loop: _loops) {
				if (loop.active) {
					if (now >= loop.timeout) {
						_driverState = DriverState_registered;
						RCLCPP_WARN(get_logger(), "Loop %i timed out", loop.outData.loopId);
						_pubSub->sendData(pybricksDriver::GetCapabilities_OutData{});
						break;
					}
				}
			}
		}

		_pubSub->sendData(pybricksDriver::Ping_PubSubData{});
	}

	void PybricksDriverConnector::processTimeSyncTask() {

		const uint64_t nowNs = get_clock()->now().nanoseconds();
		_pubSub->sendData(pybricksDriver::TimeSync_OutData{nowNs});
	}

	void PybricksDriverConnector::onPubSubPong(const pybricksDriver::Pong_PubSubData& data) {
		std::lock_guard guard(_lock);
		if (_driverState == DriverState_disconnected) {
			_driverState = DriverState_registered;
			RCLCPP_INFO(get_logger(), "Driver connected");
		}
		_pingTimeout = get_clock()->now() + 3s;
	}

	void PybricksDriverConnector::onDriverInData(const pybricksDriver::Data_InData& data) {
		auto& loop = _loops.at(data.loopId);

		const auto tNow = get_clock()->now();

		if (loop.active) {
			loop.timeout = tNow + std::max(loop.outData.loopPeriod10Ms * 10ms * 5, 500ms);
		}

		auto tData = rclcpp::Time(data.hubTimestampNs, RCL_ROS_TIME) + _timeSync[data.syncId].hubToNodeOffset;
		if (tData > tNow){
			RCLCPP_DEBUG(get_logger(), "Sensor delay < 0 : %f ms", (tNow-tData).seconds()*1000);
			tData = tNow; // Measurement of sensor delay is off, so fix it at 0
		}
		auto delay = tNow-tData;

		for (const auto& [sensorType, sensorLocation, sensorPayload] : data.sensorsPayload) {
			if (sensorType == pybricksDriver::SENSOR_TYPE_HUB_IMU) {
				const pybricksDriver::SensorData_HubIMU dataSensor(sensorType, sensorPayload);

				auto message = pybricks_ros2_interfaces::msg::PybricksDriverHubIMU();
				message.header.stamp = tData;
				message.delay = delay.nanoseconds();
				message.ax = dataSensor.ax;
				message.ay = dataSensor.ay;
				message.az = dataSensor.az;
				message.wx = dataSensor.wx;
				message.wy = dataSensor.wy;
				message.wz = dataSensor.wz;
				message.rx = dataSensor.rx;
				message.ry = dataSensor.ry;
				message.rz = dataSensor.rz;

				if (message.rx > std::numbers::pi) {
					message.rx -= 2 * std::numbers::pi;
				}
				if (message.ry > std::numbers::pi) {
					message.ry -= 2 * std::numbers::pi;
				}
				if (message.rz > std::numbers::pi) {
					message.rz -= 2 * std::numbers::pi;
				}

				std::get<PublisherHubIMU>(getPublisher(sensorLocation, sensorType))->publish(message);

			}else if (sensorType == pybricksDriver::SENSOR_TYPE_HUB_BUTTONS) {
				const pybricksDriver::SensorData_HubButtons dataSensor(sensorType, sensorPayload);

				auto message = pybricks_ros2_interfaces::msg::PybricksDriverHubButtons();
				message.header.stamp = tData;
				message.delay = delay.nanoseconds();
				message.l = dataSensor.left;
				message.c = dataSensor.center;
				message.r = dataSensor.right;
				message.b = dataSensor.bluetooth;

				std::get<PublisherHubButtons>(getPublisher(sensorLocation, sensorType))->publish(message);

			}else if (sensorType == pybricksDriver::SENSOR_TYPE_HUB_SYSTEM) {
				const pybricksDriver::SensorData_HubSystem dataSensor(sensorType, sensorPayload);

				auto message = pybricks_ros2_interfaces::msg::PybricksDriverHubSystem();
				message.header.stamp = tData;
				message.delay = delay.nanoseconds();
				message.b = dataSensor.capacity;
				message.c = dataSensor.current;

				std::get<PublisherHubSystem>(getPublisher(sensorLocation, sensorType))->publish(message);

			}else if (sensorType == pybricksDriver::SENSOR_TYPE_HUB_SPEAKER) {
				//const pybricksDriver::SensorData_HubSpeaker dataSensor(sensorType, sensorPayload);

				auto message = pybricks_ros2_interfaces::msg::PybricksDriverHubSpeaker();
				message.header.stamp = tData;
				message.delay = delay.nanoseconds();

				std::get<PublisherHubSpeaker>(getPublisher(sensorLocation, sensorType))->publish(message);

			}else if (sensorType == pybricksDriver::SENSOR_TYPE_HUB_LIGHT_55_MATRIX) {
				//const pybricksDriver::SensorData_HubLight55Matrix dataSensor(sensorType, sensorPayload);

				auto message = pybricks_ros2_interfaces::msg::PybricksDriverHubLight55Matrix();
				message.header.stamp = tData;
				message.delay = delay.nanoseconds();

				std::get<PublisherHubLight55Matrix>(getPublisher(sensorLocation, sensorType))->publish(message);

			}else if (sensorType == pybricksDriver::SENSOR_TYPE_FORCE_SENSOR) {
				const pybricksDriver::SensorData_ForceSensor dataSensor(sensorType, sensorPayload);

				auto message = pybricks_ros2_interfaces::msg::PybricksDriverForceSensor();
				message.header.stamp = tData;
				message.delay = delay.nanoseconds();
				message.f = dataSensor.f;
				message.d = dataSensor.d;
				message.pressed = dataSensor.pressed;
				message.touched = dataSensor.touched;

				std::get<PublisherForceSensor>(getPublisher(sensorLocation, sensorType))->publish(message);

			}else if (sensorType == pybricksDriver::SENSOR_TYPE_COLOR_SENSOR) {
				const pybricksDriver::SensorData_ColorSensor dataSensor(sensorType, sensorPayload);

				auto message = pybricks_ros2_interfaces::msg::PybricksDriverColorSensor();
				message.header.stamp = tData;
				message.delay = delay.nanoseconds();
				message.h = dataSensor.h;
				message.s = dataSensor.s;
				message.v = dataSensor.v;
				message.r = dataSensor.reflection;
				message.a = dataSensor.ambiant;

				std::get<PublisherColorSensor>(getPublisher(sensorLocation, sensorType))->publish(message);

			}else if (sensorType == pybricksDriver::SENSOR_TYPE_ULTRASONIC_SENSOR) {
				const pybricksDriver::SensorData_UltrasonicSensor dataSensor(sensorType, sensorPayload);

				auto message = pybricks_ros2_interfaces::msg::PybricksDriverUltrasonicSensor();
				message.header.stamp = tData;
				message.delay = delay.nanoseconds();
				message.d = dataSensor.d;
				message.inf = dataSensor.isInf;

				std::get<PublisherUltrasonicSensor>(getPublisher(sensorLocation, sensorType))->publish(message);

			}else if (sensorType == pybricksDriver::SENSOR_TYPE_MEDIUM_MOTOR) {
				const pybricksDriver::SensorData_MediumMotor dataSensor(sensorType, sensorPayload);

				auto message = pybricks_ros2_interfaces::msg::PybricksDriverMediumMotor();
				message.header.stamp = tData;
				message.delay = delay.nanoseconds();
				message.r = dataSensor.r;
				message.v = dataSensor.v;
				message.l = dataSensor.t;

				if (message.r > std::numbers::pi) {
					message.r -= 2 * std::numbers::pi;
				}

				std::get<PublisherMediumMotor>(getPublisher(sensorLocation, sensorType))->publish(message);

			}else if (sensorType == pybricksDriver::SENSOR_TYPE_LARGE_MOTOR) {
				const pybricksDriver::SensorData_LargeMotor dataSensor(sensorType, sensorPayload);

				auto message = pybricks_ros2_interfaces::msg::PybricksDriverLargeMotor();
				message.header.stamp = tData;
				message.delay = delay.nanoseconds();
				message.r = dataSensor.r;
				message.v = dataSensor.v;
				message.l = dataSensor.t;

				if (message.r > std::numbers::pi) {
					message.r -= 2 * std::numbers::pi;
				}

				std::get<PublisherLargeMotor>(getPublisher(sensorLocation, sensorType))->publish(message);

			}else if (sensorType == pybricksDriver::SENSOR_TYPE_LIGHT_33_MATRIX) {
				//const pybricksDriver::SensorData_Light33Matrix dataSensor(sensorType, sensorPayload);

				auto message = pybricks_ros2_interfaces::msg::PybricksDriverLight33Matrix();
				message.header.stamp = tData;
				message.delay = delay.nanoseconds();

				std::get<PublisherLight33Matrix>(getPublisher(sensorLocation, sensorType))->publish(message);

			}
		}

		std::string s = data.print();
		const std::regex pattern{R"(\n\t*)"};
		s = std::regex_replace(s, pattern, "  /  ");
		RCLCPP_DEBUG(get_logger(), "Hub data: %s", s.c_str());
	}

	void PybricksDriverConnector::onHubCapabilities(const pybricksDriver::CommandResponse_GetCapabilities_InData& data) {
		_hubCapabilities = data;
		std::set<std::tuple<uint8_t, uint8_t>> prevPublishers; //location, type
		for (const auto& [sensorLocation, publishers] : _rosPublisher) {
			for (const auto& sensorType : publishers | std::views::keys) {
				prevPublishers.emplace(sensorLocation, sensorType);
			}
		}

		RCLCPP_DEBUG(get_logger(), "%s", data.print().c_str());

		for (const auto& [sensorType, sensorLocation] : data.sensors) {
			if (sensorType != pybricksDriver::SENSOR_TYPE_NONE) {
				const auto nb = prevPublishers.erase({sensorLocation, sensorType});
				if (nb == 0) {
					RCLCPP_INFO(get_logger(), "Adding publisher /PybricksDriver/%s/%s", pybricksDriver::getSensorLocationName(sensorLocation).c_str(),
																						pybricksDriver::getSensorTypeSafeName(sensorType).c_str());
				}
				getPublisher(sensorLocation, sensorType);
			}
		}

		for (const auto& [sensorLocation, sensorType] : prevPublishers) {
			RCLCPP_INFO(get_logger(), "Removing publisher /PybricksDriver/%s/%s", pybricksDriver::getSensorLocationName(sensorLocation).c_str(),
																				  pybricksDriver::getSensorTypeSafeName(sensorLocation).c_str());
			_rosPublisher[sensorLocation].erase(sensorType);
		}

		if (_driverState == DriverState_registered) {
			_driverState = DriverState_gotCapabilities;
			RCLCPP_INFO(get_logger(), "Driver capabilities registered");
		}
	}

	void PybricksDriverConnector::onTimeSync(const pybricksDriver::CommandResponse_TimeSync_InData& data) {
		auto& timeSync = _timeSync[data.syncId];

		const auto now = get_clock()->now();

		timeSync.data.push_back({
			rclcpp::Time(data.pcTimestampNs, RCL_ROS_TIME),
			rclcpp::Time(data.hubTimestampNs, RCL_ROS_TIME),
			now
		});

		if (timeSync.data.size() > 10) {
			timeSync.data.pop_front();
		}

		rclcpp::Duration avgDt(0s);
		rclcpp::Duration avgOffset(0s);
		const double s = 1. / timeSync.data.size();
		for (const auto& [nodeSend_nodeTime, hubRecv_hubTime, nodeRecv_nodeTime] : timeSync.data) {
			const auto dt = (nodeRecv_nodeTime - nodeSend_nodeTime) * 0.5;
			const auto midNodeTime = nodeSend_nodeTime + dt;
			avgDt += dt * s;
			avgOffset += (midNodeTime - hubRecv_hubTime) * s;
		}
		timeSync.hubToNodeDelay = avgDt;
		timeSync.hubToNodeOffset = avgOffset;
		timeSync.lastHubTimeNs = data.hubTimestampNs;

		auto message = pybricks_ros2_interfaces::msg::TimeSync();
		message.header.stamp = now;
		message.id = data.syncId;
		message.delay = avgDt.nanoseconds();
		message.offset = avgOffset.nanoseconds();
		_timeSyncPublisher->publish(message);
	}

	const PublisherVariant& PybricksDriverConnector::getPublisher(const uint8_t sensorLocation, const uint8_t sensorType){ //NOLINT
		auto& publishers = _rosPublisher[sensorLocation];

		const auto it = publishers.find(sensorType);
		if (it != publishers.end()) {
			return it->second;
		}

		const std::string topic = std::format("PybricksDriver/{}/{}", pybricksDriver::getSensorLocationName(sensorLocation), pybricksDriver::getSensorTypeSafeName(sensorType));

		std::optional<PublisherVariant> p;
		const auto qos = rclcpp::SensorDataQoS();

		if (sensorType == pybricksDriver::SENSOR_TYPE_HUB_IMU) {
			p.emplace(create_publisher<HubIMUData>(topic, qos));
		}else if (sensorType == pybricksDriver::SENSOR_TYPE_HUB_BUTTONS) {
			p.emplace(create_publisher<HubButtonsData>(topic, qos));
		}else if (sensorType == pybricksDriver::SENSOR_TYPE_HUB_SYSTEM) {
			p.emplace(create_publisher<HubSystemData>(topic, qos));
		}else if (sensorType == pybricksDriver::SENSOR_TYPE_HUB_SPEAKER) {
			p.emplace(create_publisher<HubSpeakerData>(topic, qos));
		}else if (sensorType == pybricksDriver::SENSOR_TYPE_HUB_LIGHT_55_MATRIX) {
			p.emplace(create_publisher<HubLight55MatrixData>(topic, qos));
		}else if (sensorType == pybricksDriver::SENSOR_TYPE_FORCE_SENSOR) {
			p.emplace(create_publisher<ForceSensorData>(topic, qos));
		}else if (sensorType == pybricksDriver::SENSOR_TYPE_COLOR_SENSOR) {
			p.emplace(create_publisher<ColorSensorData>(topic, qos));
		}else if (sensorType == pybricksDriver::SENSOR_TYPE_ULTRASONIC_SENSOR) {
			p.emplace(create_publisher<UltrasonicSensorData>(topic, qos));
		}else if (sensorType == pybricksDriver::SENSOR_TYPE_MEDIUM_MOTOR) {
			p.emplace(create_publisher<MediumMotorData>(topic, qos));
		}else if (sensorType == pybricksDriver::SENSOR_TYPE_LARGE_MOTOR) {
			p.emplace(create_publisher<LargeMotorData>(topic, qos));
		}else if (sensorType == pybricksDriver::SENSOR_TYPE_LIGHT_33_MATRIX) {
			p.emplace(create_publisher<Light33MatrixData>(topic, qos));
		}

		if (p) {
			auto [insertedIt, inserted] = publishers.emplace(sensorType, std::move(*p));
			return insertedIt->second;
		}

		const std::string errMessage = std::format("No publisher found for sensor type: {}", pybricksDriver::getSensorTypeSafeName(sensorType));
		RCLCPP_ERROR(get_logger(), "%s", errMessage.c_str());
		throw std::runtime_error(errMessage);
	}
}