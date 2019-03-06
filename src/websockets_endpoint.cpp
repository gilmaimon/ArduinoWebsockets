#include <tiny_websockets/internals/websockets_endpoint.hpp>

namespace websockets { namespace internals {
    WebsocketsEndpoint::WebsocketsEndpoint(network::TcpClient& client, FragmentsPolicy fragmentsPolicy) : 
        _client(client),
        _fragmentsPolicy(fragmentsPolicy),
        _recvMode(RecvMode_Normal),
        _streamBuilder(fragmentsPolicy == FragmentsPolicy_Notify? true: false) {
        // Empty
    }

    bool WebsocketsEndpoint::poll() {
        return this->_client.poll();
    }

    Header readHeaderFromSocket(network::TcpClient& socket) {
        Header header;
        header.payload = 0;
        socket.read(reinterpret_cast<uint8_t*>(&header), 2);
        return header;
    }

    uint64_t readExtendedPayloadLength(network::TcpClient& socket, const Header& header) {
        uint64_t extendedPayload = header.payload;
        // in case of extended payload length
        if (header.payload == 126) {
            // read next 16 bits as payload length
            uint16_t tmp = 0;
            socket.read(reinterpret_cast<uint8_t*>(&tmp), 2);
            tmp = (tmp << 8) | (tmp >> 8);
            extendedPayload = tmp;
        }
        else if (header.payload == 127) {
            // TODO: read next 64 bits as payload length and handle such very long messages
        }

        return extendedPayload;
    }

    void readMaskingKey(network::TcpClient& socket, uint8_t* outputBuffer) {
        socket.read(reinterpret_cast<uint8_t*>(outputBuffer), 4);
    }

    WSString readData(network::TcpClient& socket, uint64_t extendedPayload) {
        const uint64_t BUFFER_SIZE = 64;

        WSString data = "";
        uint8_t buffer[BUFFER_SIZE];
        uint64_t done_reading = 0;
        while (done_reading < extendedPayload) {
            uint64_t to_read = extendedPayload - done_reading >= BUFFER_SIZE ? BUFFER_SIZE : extendedPayload - done_reading;
            socket.read(buffer, to_read);
            done_reading += to_read;

            for (uint64_t i = 0; i < to_read; i++) {
                data += static_cast<char>(buffer[i]);
            }
        }
        return data;
    }

    void remaskData(WSString& data, const uint8_t* const maskingKey, uint64_t payloadLength) {
        for (uint64_t i = 0; i < payloadLength; i++) {
            data[i] = data[i] ^ maskingKey[i % 4];
        }
    }

    WebsocketsFrame WebsocketsEndpoint::_recv() {
        auto header = readHeaderFromSocket(this->_client);
        uint64_t payloadLength = readExtendedPayloadLength(this->_client, header);

        uint8_t maskingKey[4];
        // if masking is set
        if (header.mask) {
            readMaskingKey(this->_client, maskingKey);
        }

        // read the message's payload (data) according to the read length
        WSString data = readData(this->_client, payloadLength);

        // if masking is set un-mask the message
        if (header.mask) {
            remaskData(data, maskingKey, payloadLength);
        }

        // Construct frame from data and header that was read
        WebsocketsFrame frame;
        frame.fin = header.fin;
        frame.mask = header.mask;

        frame.mask_buf[0] = maskingKey[0];
        frame.mask_buf[1] = maskingKey[1];
        frame.mask_buf[2] = maskingKey[2];
        frame.mask_buf[3] = maskingKey[3];

        frame.opcode = header.opcode;
        frame.payload_length = payloadLength;
        frame.payload = data;
        return frame;
    }

    WebsocketsMessage WebsocketsEndpoint::handleFrameInStreamingMode(WebsocketsFrame& frame) {
        if(frame.isControlFrame()) {
            auto msg = WebsocketsMessage::CreateFromFrame(std::move(frame));
            this->handleMessageInternally(msg);
            return std::move(msg);
        }
        else if(frame.isBeginningOfFragmentsStream()) {
            this->_recvMode = RecvMode_Streaming;

            if(this->_streamBuilder.isEmpty()) {
                this->_streamBuilder.first(frame);
                // if policy is set to notify, return the frame to the user
                if(this->_fragmentsPolicy == FragmentsPolicy_Notify) {
                    return WebsocketsMessage(this->_streamBuilder.type(), frame.payload, MessageRole::First);
                }
                else return {};
            }
        }
        else if(frame.isContinuesFragment()) {
            this->_streamBuilder.append(frame);
            if(this->_streamBuilder.isOk()) {
                // if policy is set to notify, return the frame to the user
                if(this->_fragmentsPolicy == FragmentsPolicy_Notify) {
                    return WebsocketsMessage(this->_streamBuilder.type(), frame.payload, MessageRole::Continuation);
                }
                else return {};
            }
        }
        else if(frame.isEndOfFragmentsStream()) {
            this->_recvMode = RecvMode_Normal;
            this->_streamBuilder.end(frame);
            if(this->_streamBuilder.isOk()) {
                // if policy is set to notify, return the frame to the user
                if(this->_fragmentsPolicy == FragmentsPolicy_Aggregate) {
                    auto completeMessage = this->_streamBuilder.build();
                    this->_streamBuilder = WebsocketsMessage::StreamBuilder(false);
                    this->handleMessageInternally(completeMessage);
                    return completeMessage;
                }
                else { // in case of notify policy
                    auto messageType = this->_streamBuilder.type();
                    this->_streamBuilder = WebsocketsMessage::StreamBuilder(true);
                    return WebsocketsMessage(messageType, frame.payload, MessageRole::Last);
                }                
            }
        } 
        
        // Error
        close();
        return {};
    }

    WebsocketsMessage WebsocketsEndpoint::handleFrameInStandardMode(WebsocketsFrame& frame) {
        // Normal (unfragmented) frames are handled as a complete message 
        if(frame.isNormalUnfragmentedMessage() || frame.isControlFrame()) {
            auto msg = WebsocketsMessage::CreateFromFrame(std::move(frame));
            this->handleMessageInternally(msg);
            return std::move(msg);
        } 
        else if(frame.isBeginningOfFragmentsStream()) {
            return handleFrameInStreamingMode(frame);
        }

        // This is an error. a bad combination of opcodes and fin flag arrived.
        // Close the connectiong and TODO: indicate ERROR
        close();
        return {};
    }

    WebsocketsMessage WebsocketsEndpoint::recv() {        
        auto frame = _recv();

        if(this->_recvMode == RecvMode_Normal) {
            return handleFrameInStandardMode(frame);
        } 
        else /* this->_recvMode == RecvMode_Streaming */ {
            return handleFrameInStreamingMode(frame);
        }
    }

    void WebsocketsEndpoint::handleMessageInternally(WebsocketsMessage& msg) {
        if(msg.isPing()) {
            pong(internals::fromInterfaceString(msg.data()));
        } else if(msg.isClose()) {
            close();
        }
    }

    bool WebsocketsEndpoint::send(WSString data, uint8_t opcode, bool fin, bool mask, uint8_t maskingKey[4]) { 
        return send(data.c_str(), data.size(), opcode, fin, mask, maskingKey);
    }

    bool WebsocketsEndpoint::send(const char* data, size_t len, uint8_t opcode, bool fin, bool mask, uint8_t maskingKey[4]) {
        HeaderWithExtended header;
        header.fin = fin;
        header.flags = 0;
        header.opcode = opcode;
        header.mask = mask? 1: 0;

        size_t headerLen = 2;

        if(len < 126) {
            header.payload = len;
        } else if(len < 65536) {
            header.payload = 126;
            header.extendedPayload = (len << 8) | (len >> 8);
            headerLen = 4; // with 2 bytes of extra length
        } else {
            // TODO properly handle very long message
            // ?? header.extraExtenedePayload;
            header.payload = 127;
        }
        
        // send header
        this->_client.send(reinterpret_cast<uint8_t*>(&header), headerLen);

        // if masking is set, send the masking key
        if(mask) {
            this->_client.send(reinterpret_cast<uint8_t*>(maskingKey), 4);
        }

        if(len > 0) {
            this->_client.send(reinterpret_cast<uint8_t*>(const_cast<char*>(data)), len);
        }
        return true; // TODO dont assume success
    }

    void WebsocketsEndpoint::close() {
        if(this->_client.available()) {
            send(nullptr, 0, internals::ContentType::Close);
            this->_client.close();
        }
    }

    bool WebsocketsEndpoint::pong(WSString msg) {
        // Pong data must be shorter than 125 bytes
        if(msg.size() > 125)  {
            return false;
        }
        else {
            return send(msg, ContentType::Pong);
        }
    }

    bool WebsocketsEndpoint::ping(WSString msg) {
        // Ping data must be shorter than 125 bytes
        if(msg.size() > 125) {
            return false;
        }
        else {
            return send(msg, ContentType::Ping);
        }
    }

    void WebsocketsEndpoint::setFragmentsPolicy(FragmentsPolicy newPolicy) {
        this->_fragmentsPolicy = newPolicy;
    }

    FragmentsPolicy WebsocketsEndpoint::getFragmentsPolicy() {
        return this->_fragmentsPolicy;
    }

    WebsocketsEndpoint::~WebsocketsEndpoint() {}
}} // websockets::internals
