#pragma once

#ifdef _WIN32 

#include "ws_common.h"
#include "network/tcp_client.h"

#include <windows.h>

namespace websockets { namespace network {
    class WinTcpSocket : public TcpSocket {
    public:
        bool connect(WSString host, int port);
        bool poll() override;
        bool available() override;
        void send(WSString data) override;
        void send(uint8_t* data, uint32_t len) override;
        WSString readLine() override;
        void read(uint8_t* buffer, uint32_t len) override;
        void close() override;
        virtual ~WinTcpSocket();

    private:
        SOCKET socket;
    };
}} // websockets::network


#endif // #ifdef _WIN32 