#pragma once

#ifdef _WIN32 

#include <tiny_websockets/internals/ws_common.hpp>
#include <tiny_websockets/network/tcp_server.hpp>

#define WIN32_LEAN_AND_MEAN

#undef _WIN32_WINNT
#define _WIN32_WINNT 0x501

#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

namespace websockets { namespace network {
    class WinTcpServer : public TcpServer {
    public:
        bool listen(uint16_t port) override;
        TcpClient* accept() override;
        bool available() override;
        bool poll() override;
        void close() override;
        virtual ~WinTcpServer();

    protected:
        int getSocket() const override;

    private:
        SOCKET socket;
    };
}} // websockets::network


#endif // #ifdef _WIN32 