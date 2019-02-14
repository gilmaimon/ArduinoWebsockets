#pragma once

#include "ws_common.h"
#include "network/tcp_client.h"
#include "websockets/data_frame.h"
#include "websockets/websockets_endpoint.h"
#include "websockets/message.h"
#include <functional>

namespace websockets {
    typedef std::function<void(WebsocketsMessage)> MessageCallback;

	class WebsocketsClient : private internals::WebsocketsEndpoint {
	public:
		WebsocketsClient(network::TcpClient* client);

		bool connect(WSString host, WSString path, int port);
		void onMessage(MessageCallback callback);
		void poll();
		bool available(bool activeTest = false);

		void send(WSString data);
		void sendBinary(WSString data);

		void close();

		~WebsocketsClient();

	private:
		network::TcpClient* _client;
		MessageCallback _callback;
		bool _connectionOpen;

		void _handlePing(WebsocketsMessage);
		void _handlePong(WebsocketsMessage);
		void _handleClose(WebsocketsMessage);
	};
}