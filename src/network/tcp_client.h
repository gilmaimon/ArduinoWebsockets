#pragma once

#include "ws_common.h"
#include "network/tcp_socket.h"

namespace websockets { namespace network {
	struct TcpClient : public TcpSocket {
		virtual bool connect(WSString host, int port) = 0;
	};
}} // websockets::network