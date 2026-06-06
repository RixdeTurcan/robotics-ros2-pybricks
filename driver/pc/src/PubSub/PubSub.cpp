#include <pybricks-driver/PubSub.h>
#include <pybricks-driver/logging.h>


#include <ranges>



namespace pybricksDriver
{
	const std::string PubSub::defaultPubURI = "tcp://127.0.0.1:5555";
	const std::string PubSub::defaultSubURI = "tcp://127.0.0.1:5556";


	PubSub::PubSub(std::string pubURI, std::string subURI)
		:_logger(logging::createLogger("PubSub"))
		,_context(1)
		,_publisher(_context, zmq::socket_type::pub)
		,_subscriber(_context, zmq::socket_type::sub)
		,_pubUri(std::move(pubURI))
		,_subUri(std::move(subURI))
		,_nextHandlerId(0)
	{
		_logger->info("Init publisher with URI: {}, subscriber with URI: {}", _pubUri, _subUri);
		_publisher.bind(_pubUri);
		_subscriber.connect(_subUri);
		_subscriber.set(zmq::sockopt::subscribe, "");
		_subscriber.set(zmq::sockopt::rcvtimeo, 100); // ms

		_subscriberThread = std::make_unique<std::jthread>([this](const std::stop_token& stopToken) {
			while (!stopToken.stop_requested()) {
				if (!processSubscriber()) {
					break;
				}
			}
		});
	}

	PubSub::~PubSub() {
		_logger->info("Shutting down subscriber thread");

		_subscriberThread->request_stop();

		_logger->info("Released");
	}

	bool PubSub::processSubscriber() {
		try {
			zmq::message_t message;
			const auto result = _subscriber.recv(message, zmq::recv_flags::none);
			if (result.has_value()) {
				const auto wrapperBuffer = msgpack::unpack(static_cast<const char*>(message.data()), message.size());
				const auto wrapper = wrapperBuffer.get().as<MessageWrapper>();
				const auto commandDataOpt = wrapper.getCommandData();

				_logger->trace("Received message of size {} with type id: {}", message.size(), wrapper.typeId);

				if (commandDataOpt.has_value()) {
					auto commandData = commandDataOpt.value();

					std::visit([this](auto&& c) {
						if (PYBRICKS_LOG_LEVEL >= c.logLevel()) {
							_logger->debug(c.print());
						}
					}, commandData);

					for (const auto& handler : _handlers | std::views::values) {
						// ReSharper disable once CppExpressionWithoutSideEffects
						handler(commandData);
					}

				}else{
					_logger->warn("Unknown message type id: {}", wrapper.typeId);
				}

			}

		}catch (const zmq::error_t& e) {
			if (e.num() == ETERM) {
				_logger->info("Context terminated, stopping subscriber loop");
				return false;
			}
			_logger->error("Error receiving message: {}", e.what());
		}

		return true;
	}

	void PubSub::removeHandler(const size_t handlerId) {
		_handlers.erase(handlerId);
	}

}