#pragma once

#include "ws_common.h"
#include "network/tcp_client.h"
#include "websockets/data_frame.h"
#include "websockets/message.h"

namespace websockets { namespace internals {
    class WebsocketsEndpoint {
    public:
        WebsocketsEndpoint(network::TcpSocket& socket);
        bool poll();
        WebsocketsFrame recv();
        void send(WSString data, uint8_t opcode, bool mask = false, uint8_t maskingKey[4] = nullptr);    
        
        void ping(WSString msg = "");
        void pong(WSString msg = "");
        void close(bool sendCloseFrame);
        virtual ~WebsocketsEndpoint();
    private:
        network::TcpSocket& _socket;
    };
}} // websockets::internals