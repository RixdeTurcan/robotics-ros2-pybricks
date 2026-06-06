#pragma once

#include <msgpack.hpp>

namespace pybricksDriver
{
	template<typename T> requires is_CommandType<T>
	void PubSub::sendData(const T& data) {
		msgpack::sbuffer buffer;
		msgpack::pack(buffer, data);

		auto commandData = static_cast<CommandData>(data);

		MessageWrapper wrapper;
		wrapper.typeId = commandData.index();
		wrapper.payload.assign(buffer.data(), buffer.data() + buffer.size());

		msgpack::sbuffer wrapperBuffer;
		msgpack::pack(wrapperBuffer, wrapper);

		if (PYBRICKS_LOG_LEVEL >= data.logLevel()) {
			_logger->trace("Send message of size {} with type id: {}", wrapperBuffer.size(), wrapper.typeId);
			std::visit([this](auto&& c) {
				_logger->debug(c.print());
			}, commandData);
		}
		_publisher.send(zmq::buffer(wrapperBuffer.data(), wrapperBuffer.size()), zmq::send_flags::none);
	}

	template<uint8_t N>
	std::optional<CommandData> PubSub::MessageWrapper::getCommandData() const {
		static_assert(N <= std::variant_size_v<CommandData>);

		if constexpr (N >= std::variant_size_v<CommandData>) {
			return std::nullopt;

		}else{
			if (typeId == N) {
				const auto dataBuffer = msgpack::unpack(payload.data(), payload.size());
				return dataBuffer.get().as<std::variant_alternative_t<N, CommandData>>();
			}

			return getCommandData<N+1>();
		}
	}

	template<typename T> requires is_CommandType<T>
	PubSub::CommandHandler PubSub::castToCommandHandler(const std::function<void(const T&)>& handler) {
		return [handler](const CommandData& data) {
			if (std::holds_alternative<T>(data)) {
				handler(std::get<T>(data));
				return true;
			}
			return false;
		};
	}

	template<typename T> requires is_CommandType<T>
	size_t PubSub::addHandler(const std::function<void(const T&)>& handler) {
		const size_t handlerId = _nextHandlerId++;
		_handlers.emplace(handlerId, castToCommandHandler(handler));
		return handlerId;
	}

	template<typename T> requires is_CommandType<T>
	void PubSub::addOneTimeHandler(const std::function<void(const T&)>& handler) {
		_oneTimeHandlers.push_back(castToCommandHandler(handler));
	}

}