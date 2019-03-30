#pragma once

#ifdef _WIN32 

#include <tiny_websockets/internals/ws_common.hpp>
#include <tiny_websockets/network/tcp_client.hpp>

#define WIN32_LEAN_AND_MEAN

#undef _WIN32_WINNT
#define _WIN32_WINNT 0x501

#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

#include <iostream>

namespace websockets { namespace network {
    class WinTcpClient : public TcpClient {
    public:
        WinTcpClient(const SOCKET s = INVALID_SOCKET);
        bool connect(const WSString& host, const int port) override;
        bool poll() override;
        bool available() override;
        void send(const WSString& data) override;
        void send(const WSString&& data) override;
        void send(const uint8_t* data, const uint32_t len) override;
        WSString readLine() override;
        void read(uint8_t* buffer, const uint32_t len) override;
        void close() override;
        virtual ~WinTcpClient();

    protected:
        int getSocket() const override;
    
    private:
        SOCKET socket;
    };
}} // websockets::network


#endif // #ifdef _WIN32 