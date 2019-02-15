#pragma once

#include "ws_common.h"
#include "websockets/data_frame.h"

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
        WebsocketsMessage(MessageType type, WSString data, bool fragmented = false) : _type(type), _data(data), _fragmented(fragmented) {}
        static WebsocketsMessage CreateBinary(WSString data, bool partial = false) {
            return WebsocketsMessage(MessageType::Binary, data, partial);
        }
        static WebsocketsMessage CreateText(WSString data, bool partial = false) {
            return WebsocketsMessage(MessageType::Text, data, partial);
        }

        static WebsocketsMessage CreateFromFrame(internals::WebsocketsFrame frame) {
            return WebsocketsMessage(
                static_cast<MessageType>(frame.opcode),
                frame.payload,
                (!frame.fin && frame.opcode != 0) || (frame.fin && frame.opcode == 0)
            );
        }
        
        bool isText() const { return this->_type == MessageType::Text; }
        bool isBinary() const { return this->_type == MessageType::Binary; }

        MessageType type() const { return this->_type; }
        WSString data() const { return this->_data; }

        bool isFragmented() const { return this->_fragmented; }

    private:
        MessageType _type;
        WSString _data;
        bool _fragmented;
    };
}