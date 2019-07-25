#pragma once

#include <tiny_websockets/internals/ws_common.hpp>
#include <tiny_websockets/network/tcp_socket.hpp>

namespace websockets { namespace network {
  struct TcpClient : public TcpSocket {
    virtual bool poll() = 0;
    virtual void send(const WSString& data) = 0;
    virtual void send(const WSString&& data) = 0;
    virtual void send(const uint8_t* data, const uint32_t len) = 0;
    virtual WSString readLine() = 0;
    virtual uint32_t read(uint8_t* buffer, const uint32_t len) = 0;
    virtual bool connect(const WSString& host, int port) = 0;
    virtual ~TcpClient() {}
  };
}} // websockets::network