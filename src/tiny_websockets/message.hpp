#pragma once

#include <tiny_websockets/internals/ws_common.hpp>
#include <tiny_websockets/internals/data_frame.hpp>

namespace websockets {
    enum MessageType {
        // Data opcdoes
        Text = 0x1,
        Binary = 0x2,

        // Control opcodes
        Close = 0x8,
        Ping = 0x9,
        Pong = 0xA
    };

    // The class the user will interact with as a message
    // This message can be partial (so practically this is a Frame and not a message)
    struct WebsocketsMessage {
        WebsocketsMessage(MessageType msgType, WSInterfaceString msgData, bool fragmented = false) : _type(msgType), _data(msgData), _fragmented(fragmented) {}
        static WebsocketsMessage CreateBinary(WSInterfaceString msgData, bool partial = false) {
            return WebsocketsMessage(MessageType::Binary, msgData, partial);
        }
        static WebsocketsMessage CreateText(WSInterfaceString msgData, bool partial = false) {
            return WebsocketsMessage(MessageType::Text, msgData, partial);
        }

        static WebsocketsMessage CreateFromFrame(internals::WebsocketsFrame frame) {
            return WebsocketsMessage(
                static_cast<MessageType>(frame.opcode),
                internals::fromInternalString(frame.payload),
                (!frame.fin && frame.opcode != 0) || (frame.fin && frame.opcode == 0)
            );
        }
        
        bool isText() const { return this->_type == MessageType::Text; }
        bool isBinary() const { return this->_type == MessageType::Binary; }

        MessageType type() const { return this->_type; }
        WSInterfaceString data() const { return this->_data; }

        bool isFragmented() const { return this->_fragmented; }

    private:
        MessageType _type;
        WSInterfaceString _data;
        bool _fragmented;
    };
}