#include "HubDriver/HubDriver.h"
#include <pybricks-driver/PubSub.h>
#include <pybricks-driver/logging.h>
#include "DriverFSM/DriverFSM.h"

#include <thread>
#include <chrono>
#include <csignal>
#include <format>
#include <stdexcept>
#include <regex>
#include <filesystem>


using namespace std::chrono_literals;

#ifndef PYBRICKS_DRIVER_LOG_PATH
	#define PYBRICKS_DRIVER_LOG_PATH "./log/pybricks-driver.log"
#endif

static volatile std::sig_atomic_t stopping = 0;

void signalHandler(const int signum) {
	std::string signalName;
	switch (signum) {
		case SIGINT: signalName = "SIGINT"; break;
		case SIGTERM: signalName = "SIGTERM"; break;
		default: signalName = "Unknown"; break;
	}

	pybricksDriver::logging::createLogger("Signal")->warn("Received signal {}, initiating shutdown", signalName);

	stopping = 1;
}

enum DriverStatus
{
	DriverStatus_inactive,
	DriverStatus_waiting_for_connector,
	DriverStatus_active
};

enum MainParameter
{
	MainParameter_none,
	MainParameter_device_d, //Device name: Pybricks Hub
	MainParameter_gattServ_e, //GATT service UUID: c5f50001-8280-46da-89f4-6d8051e4aeef
	MainParameter_gattChar_c, //GATT characteristic UUID: c5f50002-8280-46da-89f4-6d8051e4aeef
	MainParameter_programId_i, //Program id: 0
	MainParameter_subUri_s, //PubSub driver subscribe URI: tcp://127.0.0.1:5555
	MainParameter_pubUri_p, //PubSub driver publish URI: tcp://127.0.0.1:5556
	MainParameter_log_l, //Log path: PYBRICKS_DRIVER_LOG_PATH | ./log/pybricks-driver.log
};


int main(int argc, char* argv[]) {
	signal(SIGINT, signalHandler);
	signal(SIGTERM, signalHandler);

	std::string deviceName{pybricksDriver::messaging::DEFAULT_DEVICE_NAME};
	std::string gattServiceUUID{pybricksDriver::messaging::DEFAULT_SERVICE_UUID};
	std::string gattCharacteristicUUID{pybricksDriver::messaging::DEFAULT_CHARACTERISTIC_UUID};
	uint8_t programId{0x0};
	std::string subURI{pybricksDriver::PubSub::defaultPubURI};
	std::string pubURI{pybricksDriver::PubSub::defaultSubURI};
	std::string logPath{PYBRICKS_DRIVER_LOG_PATH};

	if (argc > 1) {
		MainParameter param{MainParameter_none};
		std::vector<std::string> args = {&argv[1], &argv[argc]};
		for (const auto& arg : args) {
			if (!arg.empty() && arg[0] == '-') {
				if (param != MainParameter_none) {
					throw std::invalid_argument(std::format("Expected argument value: {}", arg));
				}

				if (arg == "--device" || arg == "-d") {
					param = MainParameter_device_d;

				}else if (arg == "--gattServ" || arg == "-e") {
					param = MainParameter_gattServ_e;

				}else if (arg == "--gattChar" || arg == "-c") {
					param = MainParameter_gattChar_c;

				}else if (arg == "--programId" || arg == "-i") {
					param = MainParameter_programId_i;

				}else if (arg == "--subUri" || arg == "-s") {
					param = MainParameter_subUri_s;

				}else if (arg == "--pubUri" || arg == "-p") {
					param = MainParameter_pubUri_p;

				}else if (arg == "--log" || arg == "-l") {
					param = MainParameter_log_l;

				}else{
					throw std::invalid_argument(std::format("Unknown argument: {}", arg));
				}

			}else if (param != MainParameter_none) {
				if (param == MainParameter_device_d) {
					deviceName = arg;

				}else if (param == MainParameter_gattServ_e) {
					static const std::regex regex{R"(^[0-9A-Fa-f]{8}-[0-9A-Fa-f]{4}-[0-9A-Fa-f]{4}-[0-9A-Fa-f]{4}-[0-9A-Fa-f]{12}$)"};
					if (!std::regex_match(arg, regex)) {
						throw std::invalid_argument(std::format("Invalid GATT service UUID: {}", arg));
					}
					gattServiceUUID = arg;

				}else if (param == MainParameter_gattChar_c) {
					static const std::regex regex{R"(^[0-9A-Fa-f]{8}-[0-9A-Fa-f]{4}-[0-9A-Fa-f]{4}-[0-9A-Fa-f]{4}-[0-9A-Fa-f]{12}$)"};
					if (!std::regex_match(arg, regex)) {
						throw std::invalid_argument(std::format("Invalid GATT characteristic UUID: {}", arg));
					}
					gattCharacteristicUUID = arg;

				}else if (param == MainParameter_programId_i) {
					static const std::regex regex{"^[0-4]$"};
					if (!std::regex_match(arg, regex)) {
						throw std::invalid_argument(std::format("Invalid Program Id: {}", arg));
					}
					int value = std::stoi(arg);
					if (value < 0 || value > 4) {
						throw std::invalid_argument(std::format("Invalid Program Id: {}", arg));
					}
					programId = static_cast<uint8_t>(value);

				}else if (param == MainParameter_subUri_s) {
					static const std::regex regex{R"(^tcp://(\*|[0-9a-zA-Z._-]+|\[[0-9a-fA-F:]+\]):([1-9]\d{0,3}|[1-5]\d{4}|6[0-4]\d{3}|65[0-4]\d{2}|655[0-2]\d|6553[0-5]|\*))"};
					if (!std::regex_match(arg, regex)) {
						throw std::invalid_argument(std::format("Invalid PubSub subscribe URI: {}", arg));
					}
					subURI = arg;

				}else if (param == MainParameter_pubUri_p) {
					static const std::regex regex{R"(^tcp://(\*|[0-9a-zA-Z._-]+|\[[0-9a-fA-F:]+\]):([1-9]\d{0,3}|[1-5]\d{4}|6[0-4]\d{3}|65[0-4]\d{2}|655[0-2]\d|6553[0-5]|\*))"};
					if (!std::regex_match(arg, regex)) {
						throw std::invalid_argument(std::format("Invalid PubSub publis URI: {}", arg));
					}
					pubURI = arg;

				}else if (param == MainParameter_log_l) { //NOLINT
					std::string path = arg;
					std::filesystem::path fsPath(path);
					std::filesystem::path parent = fsPath.parent_path();

					if (!parent.empty()) {
						if (!std::filesystem::exists(parent)) {
							throw std::invalid_argument(std::format("Log path directory does not exist: {}", arg));
						}
						if (!std::filesystem::is_directory(parent)) {
							throw std::invalid_argument(std::format("Log path directory is not a directory: {}", arg));
						}
					}
					if (std::filesystem::exists(fsPath)) {
						if (std::filesystem::is_directory(fsPath)) {
							throw std::invalid_argument(std::format("Log path is a directory: {}", arg));
						}
					}
					logPath = arg;

				}
				param = MainParameter_none;

			}else{
				throw std::invalid_argument(std::format("Invalid argument: {}", arg));
			}
		}

		if (param != MainParameter_none) {
			throw std::invalid_argument("Missing value for last argument");
		}
	}


	pybricksDriver::logging::initLogging(logPath);

	const auto logger = pybricksDriver::logging::createLogger("Main");

	{
		logger->info("Start program");

		pybricksDriver::HubDriver hub(deviceName, gattServiceUUID, gattCharacteristicUUID);

		{
			pybricksDriver::PubSub pubSub(pubURI, subURI);
			pybricksDriver::DriverFSM driverDSM(hub, pubSub);

			while (!stopping) {
				driverDSM.processFSM(programId);
			}
		}

		logger->info("Stop hub program");

		hub.stopProgram();

		int timeoutCount = 0;
		while (hub.isProgramRunning()) {
			std::this_thread::sleep_for(100ms);
			if (timeoutCount >= 30) {
				logger->warn("Timeout while stopping program {}", programId);
				break;
			}
			timeoutCount += 1;
		}

		logger->info("Disconnect from hub");

		hub.disconnect();

		logger->info("Stop program");
	}

	logger->info("Resource cleanup complete, exiting");

	pybricksDriver::logging::deleteAllLoggers();

	return 0;
}
