#pragma once

#include <tiny_websockets/internals/ws_common.hpp>
#include <tiny_websockets/network/tcp_client.hpp>
#include <tiny_websockets/network/tcp_server.hpp>

namespace websockets { namespace network {
  struct TcpServer : public TcpSocket {
    virtual bool poll() = 0;
    virtual bool listen(const uint16_t port) = 0;
    virtual TcpClient* accept() = 0;
    virtual ~TcpServer() {}
  };
}} // websockets::network