#pragma once

#include "ws_common.h"

namespace websockets { namespace network {
    struct TcpSocket {
        virtual bool available() = 0;
        virtual bool poll() = 0;
        virtual void send(WSString data) = 0;
        virtual void send(uint8_t* data, uint32_t len) = 0;
        virtual WSString readLine() = 0;
        virtual void read(uint8_t* buffer, uint32_t len) = 0;
        virtual void close() = 0;
        virtual ~TcpSocket() {}
    };
}} // websockets::network