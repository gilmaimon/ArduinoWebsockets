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
        WebsocketsMessage recv();
        bool send(WSString data, uint8_t opcode, bool mask = false, uint8_t maskingKey[4] = nullptr);    
        
        bool ping(WSString msg = "");
        bool pong(WSString msg = "");
        void close();
        virtual ~WebsocketsEndpoint();
    private:
        network::TcpSocket& _socket;
        WebsocketsFrame _recv();
    };
}} // websockets::internals