#include <string>
#include <vector>
#include <array>
#include <optional>

#include <pybricks-driver/logging.h>


namespace pybricksDriver::bluetooth
{
	struct BluetoothDevice {
		std::string mac;
		std::string name;
	};

	std::vector<std::string> run_command(const std::string& cmd) {
		std::vector<std::string> lines;
		std::array<char, 1024> buffer{};

		FILE* pipe = popen(cmd.c_str(), "r");
		if (!pipe) return lines;

		while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
			std::string line = buffer.data();
			if (!line.empty() && line.back() == '\n') line.pop_back();  // trim
			if (!line.empty()) lines.push_back(line);
		}
		pclose(pipe);

		return lines;
	}

	std::optional<std::string> find_mac_of_device(const std::string_view target_name) {
		const auto output = run_command("bluetoothctl devices");

		for (const std::string& line : output) {
			// Expect: "Device XX:XX:XX:XX:XX:XX Name"
			if (!line.starts_with("Device ")) continue;

			const auto first_space = line.find(' ');
			const auto second_space = line.find(' ', first_space + 1);
			if (second_space == std::string::npos) continue;

			std::string mac = line.substr(first_space + 1, second_space - first_space - 1);
			std::string name = line.substr(second_space + 1);

			if (name == target_name) {
				return mac;
			}
		}
		return std::nullopt;
	}

	bool disconnect_device(const std::string_view name, const logging::Logger& logger) {
		const auto mac = find_mac_of_device(name);
		if (!mac) {
			logger->trace("Device with name '{}' not found among paired devices.", name);
			return false;
		}

		std::string cmd = "bluetoothctl disconnect " + *mac + " > /dev/null 2>&1";
		int res = system(cmd.c_str());
		if (res != 0) {
			logger->trace("Failed to disconnect device '{}'. Command '{}' exited with code {}.", name, cmd, res);
			return false;
		}

		logger->trace("Disconnected device '{}'", name);

		return true;
	}
}
