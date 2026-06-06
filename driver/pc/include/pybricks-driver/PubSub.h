#pragma once

#include <pybricks-driver/types.h>

#include <spdlog/logger.h>
#include <zmq.hpp>
#include <msgpack.hpp>

#include <thread>
#include <optional>

namespace pybricksDriver
{
	class PubSub
	{
		public:
			using CommandHandler = std::function<bool(const CommandData&)>;

			static const std::string defaultPubURI;
			static const std::string defaultSubURI;

		public:
			struct MessageWrapper
			{
				uint8_t typeId = 0;
				std::vector<char> payload;

				template<uint8_t N = 0>
				[[nodiscard]] std::optional<CommandData> getCommandData() const;

				MSGPACK_DEFINE(typeId, payload);
			};

		public:
			explicit PubSub(std::string pubURI = defaultPubURI, std::string subURI = defaultSubURI);
			~PubSub();

			template<typename T> requires is_CommandType<T>
			void sendData(const T& data);

			template<typename T> requires is_CommandType<T>
			size_t addHandler(const std::function<void(const T&)>& handler);

			template<typename T> requires is_CommandType<T>
			void addOneTimeHandler(const std::function<void(const T&)>& handler);

			void removeHandler(size_t handlerId);

		private:
			bool processSubscriber();

			template<typename T> requires is_CommandType<T>
			CommandHandler castToCommandHandler(const std::function<void(const T&)>& handler);

		private:
			std::shared_ptr<spdlog::logger> _logger;
			zmq::context_t _context;

			zmq::socket_t _publisher;
			zmq::socket_t _subscriber;

			std::string _pubUri;
			std::string _subUri;

			std::unique_ptr<std::jthread> _subscriberThread;

			std::map<size_t, CommandHandler> _handlers;
			std::list<CommandHandler> _oneTimeHandlers;
			size_t _nextHandlerId;
	};
}

#include "PubSub.hpp"
