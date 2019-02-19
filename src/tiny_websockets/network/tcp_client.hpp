#pragma once

#include <tiny_websockets/internals/ws_common.hpp>
#include <tiny_websockets/network/tcp_socket.hpp>

namespace websockets { namespace network {
	struct TcpClient : public TcpSocket {
		virtual bool connect(WSString host, int port) = 0;
	};
}} // websockets::network