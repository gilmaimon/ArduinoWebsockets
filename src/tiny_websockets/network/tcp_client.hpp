#pragma once

#include <tiny_websockets/internals/ws_common.hpp>
#include <tiny_websockets/network/tcp_socket.hpp>

namespace websockets { namespace network {
  struct TcpClient : public TcpSocket {
    virtual bool poll() = 0;
        virtual void send(WSString data) = 0;
        virtual void send(uint8_t* data, uint32_t len) = 0;
        virtual WSString readLine() = 0;
        virtual void read(uint8_t* buffer, uint32_t len) = 0;
    virtual bool connect(WSString host, int port) = 0;
    virtual ~TcpClient() {}
  };
}} // websockets::network