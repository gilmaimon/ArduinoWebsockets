#pragma once

#include "ws_common.h"
#include "network/tcp_client.h"
#include "websockets/data_frame.h"
#include "websockets/websockets_endpoint.h"
#include "websockets/message.h"
#include <functional>

namespace websockets {
	enum class WebsocketsEvent {
		ConnectionOpened,
		ConnectionClosed,
		GotPing, GotPong
	};
    typedef std::function<void(WebsocketsMessage)> MessageCallback;
    typedef std::function<void(WebsocketsEvent, WSString data)> EventCallback;

	class WebsocketsClient : private internals::WebsocketsEndpoint {
	public:
		WebsocketsClient(network::TcpClient* client);
		
		template <class TcpClientTy>
		static WebsocketsClient Create(TcpClientTy* clientPtr = new TcpClientTy) {
			return WebsocketsClient(clientPtr);
		}

		bool connect(WSString host, int port, WSString path);

		void onMessage(MessageCallback callback);
		void onEvent(EventCallback callback);

		void poll();
		bool available(bool activeTest = false);

		void send(WSString data);
		void sendBinary(WSString data);

		void ping(WSString data = "");
		void pong(WSString data = "");

		void close();

		~WebsocketsClient();

	private:
		network::TcpClient* _client;
		MessageCallback _messagesCallback;
		EventCallback _eventsCallback;
		bool _connectionOpen;

		void _handlePing(WebsocketsMessage);
		void _handlePong(WebsocketsMessage);
		void _handleClose(WebsocketsMessage);
	};
}