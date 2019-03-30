#pragma once

#include <tiny_websockets/internals/ws_common.hpp>

namespace websockets { namespace network {
    struct TcpSocket {
    public:
        virtual bool available() = 0;
        virtual void close() = 0;
        virtual ~TcpSocket() {}
    protected:
        virtual int getSocket() const = 0;
    };
}} // websockets::network