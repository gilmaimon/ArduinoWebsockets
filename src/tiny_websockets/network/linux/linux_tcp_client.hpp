#pragma once

#ifdef __linux__ 

#include <tiny_websockets/internals/ws_common.hpp>
#include <tiny_websockets/network/tcp_client.hpp>
#include <tiny_websockets/network/tcp_socket.hpp>

#define INVALID_SOCKET -1

namespace websockets { namespace network {
  class LinuxTcpClient : public TcpClient {
    public:
        LinuxTcpClient(int socket = INVALID_SOCKET);
        bool connect(WSString host, int port) override;
        bool poll() override;
        bool available() override;
        void send(WSString data) override;
        void send(uint8_t* data, uint32_t len) override;
        WSString readLine() override;
        void read(uint8_t* buffer, uint32_t len) override;
        void close() override;
        virtual ~LinuxTcpClient();

    private:
        int _socket;
    };
}} // websockets::network

#endif // #ifdef __linux__ 