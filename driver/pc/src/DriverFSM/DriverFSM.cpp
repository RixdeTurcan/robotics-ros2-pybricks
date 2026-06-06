#include "DriverFSM.h"

using namespace std::chrono_literals;

namespace pybricksDriver
{
	DriverFSM::DriverFSM(HubDriver& hub, PubSub& pubSub)
		:_hub(hub)
		,_pubSub(pubSub)
		,_logger(logging::createLogger("DriverFSM"))
		,_driverStatus(DriverStatus_inactive)
		,_pingTimeout(std::chrono::steady_clock::now())
	{
		_logger->info("Initializing");

		_pingTaskThread = std::make_unique<std::jthread>([this](const std::stop_token& stopToken) {
			while (!stopToken.stop_requested()) {
				processPingTask();
				std::this_thread::sleep_for(500ms);
			}
		});

		_pubSubHandlerTokens.emplace_back(_pubSub.addHandler<Ping_PubSubData>([this](const auto& data){onPubSubPing(data);}));

		_pubSubHandlerTokens.emplace_back(_pubSub.addHandler<StartListening_OutData>([this](const auto& data){forwardToHubIfActive(data);}));
		_pubSubHandlerTokens.emplace_back(_pubSub.addHandler<StopListening_OutData>([this](const auto& data){forwardToHubIfActive(data);}));
		_pubSubHandlerTokens.emplace_back(_pubSub.addHandler<StopAllListening_OutData>([this](const auto& data){forwardToHubIfActive(data);}));
		_pubSubHandlerTokens.emplace_back(_pubSub.addHandler<GetCapabilities_OutData>([this](const auto& data){forwardToHubIfActive(data);}));
		_pubSubHandlerTokens.emplace_back(_pubSub.addHandler<TimeSync_OutData>([this](const auto& data){forwardToHubIfActive(data);}));
		_pubSubHandlerTokens.emplace_back(_pubSub.addHandler<Actuator_OutData>([this](const auto& data){forwardToHubIfActive(data);}));

		_hubHandlerTokens.emplace_back(_hub.addHandler<Data_InData>([this](const auto& data){forwardToPubSubIfActive(data);}));
		_hubHandlerTokens.emplace_back(_hub.addHandler<CommandResponse_StartListening_InData>([this](const auto& data){forwardToPubSubIfActive(data);}));
		_hubHandlerTokens.emplace_back(_hub.addHandler<CommandResponse_StopListening_InData>([this](const auto& data){forwardToPubSubIfActive(data);}));
		_hubHandlerTokens.emplace_back(_hub.addHandler<CommandResponse_StopAllListening_InData>([this](const auto& data){forwardToPubSubIfActive(data);}));
		_hubHandlerTokens.emplace_back(_hub.addHandler<CommandResponse_GetCapabilities_InData>([this](const auto& data){forwardToPubSubIfActive(data);}));
		_hubHandlerTokens.emplace_back(_hub.addHandler<CommandResponse_TimeSync_InData>([this](const auto& data){forwardToPubSubIfActive(data);}));
		_hubHandlerTokens.emplace_back(_hub.addHandler<CommandResponse_Actuator_InData>([this](const auto& data){forwardToPubSubIfActive(data);}));

		_logger->info("Initialized");
	}

	DriverFSM::~DriverFSM() {
		_logger->info("Releasing");

		for (const auto& token:_hubHandlerTokens) {
			_hub.removeHandler(token);
		}
		for (const auto& token:_pubSubHandlerTokens) {
			_pubSub.removeHandler(token);
		}

		_pingTaskThread->request_stop();

		_logger->info("Released");
	}

	void DriverFSM::processPingTask() {
		std::lock_guard guard(_lock);

		if (std::chrono::steady_clock::now() >= _pingTimeout) {
			if (_driverStatus == DriverStatus_active) {
				_logger->info("PubSub disconnected (timeout)");
				_hub.sendCommand(StopAllListening_OutData{});
				_driverStatus = DriverStatus_waiting_for_connector;
			}
		}
	}

	void DriverFSM::onPubSubPing(const Ping_PubSubData& data) {
		if (_driverStatus != DriverStatus_inactive) {
			std::lock_guard guard(_lock);
			if (_driverStatus == DriverStatus_waiting_for_connector) {
				_driverStatus = DriverStatus_active;
				_logger->info("PubSub connected");
			}
			_pingTimeout = std::chrono::steady_clock::now() + 3s;
			_pubSub.sendData(Pong_PubSubData{});
		}
	}

	void DriverFSM::processFSM(std::optional<uint8_t> programId) {
		if (!_hub.isConnected()) {
			if (_driverStatus == DriverStatus_active){
				std::lock_guard guard(_lock);
				_driverStatus = DriverStatus_inactive;
			}
			_logger->info("Connect to hub");
			_hub.connect();
			if (!_hub.isConnected()) {
				_logger->debug("Failed to connect to hub, retrying in 1 second...");
				std::this_thread::sleep_for(1s);
			}

		}else if (!_hub.isProgramRunning()) {
			if (_driverStatus == DriverStatus_active){
				std::lock_guard guard(_lock);
				_driverStatus = DriverStatus_inactive;
			}
			if (programId.has_value()) {
				_logger->info("Start hub program ID: {}", programId.value());
			}else {
				_logger->info("Start hub program ID: Selected on Hub");
			}
			std::this_thread::sleep_for(250ms);
			_hub.startProgram(programId);
			std::this_thread::sleep_for(250ms);

		}else if (_hub.getRunningProgramId() != programId) {
			if (_driverStatus == DriverStatus_active){
				std::lock_guard guard(_lock);
				_driverStatus = DriverStatus_inactive;
			}

			_logger->info("Stop running program ID: {}", _hub.getRunningProgramId());
			_hub.stopProgram();
			std::this_thread::sleep_for(500ms);

		}else {
			if (_driverStatus == DriverStatus_inactive){
				std::lock_guard guard(_lock);
				_driverStatus = DriverStatus_waiting_for_connector;
			}

			std::this_thread::sleep_for(100ms);
		}
	}
}