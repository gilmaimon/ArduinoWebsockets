#include <tiny_websockets/internals/websockets_endpoint.hpp>

namespace websockets { 

    CloseReason GetCloseReason(uint16_t reasonCode) {
        switch(reasonCode) {
            case CloseReason_NormalClosure: 
                return CloseReason_NormalClosure;
            
            case CloseReason_GoingAway: 
                return CloseReason_GoingAway;
            
            case CloseReason_ProtocolError: 
                return CloseReason_ProtocolError;
            
            case CloseReason_UnsupportedData: 
                return CloseReason_UnsupportedData;
            
            case CloseReason_AbnormalClosure: 
                return CloseReason_AbnormalClosure;
            
            case CloseReason_InvalidPayloadData: 
                return CloseReason_InvalidPayloadData;
            
            case CloseReason_PolicyViolation: 
                return CloseReason_PolicyViolation;
            
            case CloseReason_MessageTooBig: 
                return CloseReason_MessageTooBig;
            
            case CloseReason_NoStatusRcvd: 
                return CloseReason_NoStatusRcvd;
            
            case CloseReason_InternalServerError: 
                return CloseReason_InternalServerError;

            default: return CloseReason_None;
        }
    }    

namespace internals {

    uint32_t swapEndianess(uint32_t num) {
        uint32_t highest = (num >> 24);
        uint32_t second = (num << 8) >> 24;
        uint32_t third = (num << 16) >> 24;
        uint32_t lowest = (num << 24) >> 24;

        return highest | (second << 8) | (third << 16) | (lowest << 24);
    }

    uint64_t swapEndianess(uint64_t num) {
        uint32_t upper = (num >> 32);
        uint32_t lower = (num << 32) >> 32;
    
        upper = swapEndianess(upper);
        lower = swapEndianess(lower);
    
        uint64_t upperLong = upper;
        uint64_t lowerLong = lower;
        
        return upperLong | (lowerLong << 32);
    }

    WebsocketsEndpoint::WebsocketsEndpoint(std::shared_ptr<network::TcpClient> client, FragmentsPolicy fragmentsPolicy) : 
        _client(client),
        _fragmentsPolicy(fragmentsPolicy),
        _recvMode(RecvMode_Normal),
        _streamBuilder(fragmentsPolicy == FragmentsPolicy_Notify? true: false),
        _closeReason(CloseReason_None) {
        // Empty
    }

    WebsocketsEndpoint::WebsocketsEndpoint(const WebsocketsEndpoint& other) : 
        _client(other._client), 
        _fragmentsPolicy(other._fragmentsPolicy), 
        _recvMode(other._recvMode), 
        _streamBuilder(other._streamBuilder), 
        _closeReason(other._closeReason),
        _useMasking(other._useMasking) {

        const_cast<WebsocketsEndpoint&>(other)._client = nullptr;
    }
    WebsocketsEndpoint::WebsocketsEndpoint(const WebsocketsEndpoint&& other) : 
        _client(other._client), 
        _fragmentsPolicy(other._fragmentsPolicy), 
        _recvMode(other._recvMode), 
        _streamBuilder(other._streamBuilder), 
        _closeReason(other._closeReason),
        _useMasking(other._useMasking) {

        const_cast<WebsocketsEndpoint&>(other)._client = nullptr;
    }

    WebsocketsEndpoint& WebsocketsEndpoint::operator=(const WebsocketsEndpoint& other) {
        this->_client = other._client;
        this->_fragmentsPolicy = other._fragmentsPolicy;
        this->_recvMode = other._recvMode;
        this->_streamBuilder = other._streamBuilder;
        this->_closeReason = other._closeReason;
        this->_useMasking = other._useMasking;

        const_cast<WebsocketsEndpoint&>(other)._client = nullptr;

        return *this;
    }
    WebsocketsEndpoint& WebsocketsEndpoint::operator=(const WebsocketsEndpoint&& other) {
        this->_client = other._client;
        this->_fragmentsPolicy = other._fragmentsPolicy;
        this->_recvMode = other._recvMode;
        this->_streamBuilder = other._streamBuilder;
        this->_closeReason = other._closeReason;
        this->_useMasking = other._useMasking;

        const_cast<WebsocketsEndpoint&>(other)._client = nullptr;

        return *this;
    }

    void WebsocketsEndpoint::setInternalSocket(std::shared_ptr<network::TcpClient> socket) {
        this->_client = socket;
    }

    bool WebsocketsEndpoint::poll() {
        return this->_client->poll();
    }

    uint32_t readUntilSuccessfullOrError(network::TcpClient& socket, uint8_t* buffer, const uint32_t len) {
        auto numRead = socket.read(buffer, len);
        while(numRead == static_cast<uint32_t>(-1) && socket.available()) {
            numRead = socket.read(buffer, len);
        }
        return numRead;
    }

    Header readHeaderFromSocket(network::TcpClient& socket) {
        Header header;
        header.payload = 0;
        readUntilSuccessfullOrError(socket, reinterpret_cast<uint8_t*>(&header), 2);
        return header;
    }

    uint64_t readExtendedPayloadLength(network::TcpClient& socket, const Header& header) {
        uint64_t extendedPayload = header.payload;
        // in case of extended payload length
        if (header.payload == 126) {
            // read next 16 bits as payload length
            uint16_t tmp = 0;
            readUntilSuccessfullOrError(socket, reinterpret_cast<uint8_t*>(&tmp), 2);
            tmp = (tmp << 8) | (tmp >> 8);
            extendedPayload = tmp;
        }
        else if (header.payload == 127) {
            uint64_t tmp = 0;
            readUntilSuccessfullOrError(socket, reinterpret_cast<uint8_t*>(&tmp), 8);
            extendedPayload = swapEndianess(tmp);
        }

        return extendedPayload;
    }

    void readMaskingKey(network::TcpClient& socket, uint8_t* outputBuffer) {
        readUntilSuccessfullOrError(socket, reinterpret_cast<uint8_t*>(outputBuffer), 4);
    }

    WSString readData(network::TcpClient& socket, uint64_t extendedPayload) {
        const uint64_t BUFFER_SIZE = _WS_BUFFER_SIZE;

        WSString data(extendedPayload, '\0');
        uint8_t buffer[BUFFER_SIZE];
        uint64_t done_reading = 0;
        while (done_reading < extendedPayload && socket.available()) {
            uint64_t to_read = extendedPayload - done_reading >= BUFFER_SIZE ? BUFFER_SIZE : extendedPayload - done_reading;
            uint32_t numReceived = readUntilSuccessfullOrError(socket, buffer, to_read);

            // On failed reads, skip
            if(!socket.available()) break;

            for (uint64_t i = 0; i < numReceived; i++) {
                data[done_reading + i] = static_cast<char>(buffer[i]);
            }

            done_reading += numReceived;
        }
        return std::move(data);
    }

    void remaskData(WSString& data, const uint8_t* const maskingKey, uint64_t payloadLength) {
      for (uint64_t i = 0; i < payloadLength; i++) {
        data[i] = data[i] ^ maskingKey[i % 4];
      }
    }

    WebsocketsFrame WebsocketsEndpoint::_recv() {
        auto header = readHeaderFromSocket(*this->_client);
        if(!_client->available()) return WebsocketsFrame(); // In case of faliure

        uint64_t payloadLength = readExtendedPayloadLength(*this->_client, header);
        if(!_client->available()) return WebsocketsFrame(); // In case of faliure

#ifdef _WS_CONFIG_MAX_MESSAGE_SIZE
        if(payloadLength > _WS_CONFIG_MAX_MESSAGE_SIZE) {
            return WebsocketsFrame();
        }
#endif

        uint8_t maskingKey[4];
        // if masking is set
        if (header.mask) {
            readMaskingKey(*this->_client, maskingKey);
            if(!_client->available()) return WebsocketsFrame(); // In case of faliure
        }

        WebsocketsFrame frame;
        // read the message's payload (data) according to the read length
        frame.payload = readData(*this->_client, payloadLength);
        if(!_client->available()) return WebsocketsFrame(); // In case of faliure

        // if masking is set un-mask the message
        if (header.mask) {
            remaskData(frame.payload, maskingKey, payloadLength);
        }

        // Construct frame from data and header that was read
        frame.fin = header.fin;
        frame.mask = header.mask;

        frame.mask_buf[0] = maskingKey[0];
        frame.mask_buf[1] = maskingKey[1];
        frame.mask_buf[2] = maskingKey[2];
        frame.mask_buf[3] = maskingKey[3];

        frame.opcode = header.opcode;
        frame.payload_length = payloadLength;

        return std::move(frame);
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
                    return WebsocketsMessage(this->_streamBuilder.type(), std::move(frame.payload), MessageRole::First);
                }
                else return {};
            }
        }
        else if(frame.isContinuesFragment()) {
            this->_streamBuilder.append(frame);
            if(this->_streamBuilder.isOk()) {
                // if policy is set to notify, return the frame to the user
                if(this->_fragmentsPolicy == FragmentsPolicy_Notify) {
                    return WebsocketsMessage(this->_streamBuilder.type(), std::move(frame.payload), MessageRole::Continuation);
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
                    return WebsocketsMessage(messageType, std::move(frame.payload), MessageRole::Last);
                }                
            }
        } 
        
        // Error
        close(CloseReason_ProtocolError);
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
        close(CloseReason_ProtocolError);
        return {};
    }

    WebsocketsMessage WebsocketsEndpoint::recv() {        
        auto frame = _recv();
        if (frame.isEmpty()) {
            return {};
        }

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
            // is there a reason field
            if(internals::fromInterfaceString(msg.data()).size() >= 2) {
                uint16_t reason = *(reinterpret_cast<const uint16_t*>(msg.data().c_str()));
                reason = reason >> 8 | reason << 8;
                this->_closeReason = GetCloseReason(reason);
            } else {
                this->_closeReason = CloseReason_GoingAway;
            }
            close(this->_closeReason);
        }
    }

    bool WebsocketsEndpoint::send(const char* data, const size_t len, const uint8_t opcode, const bool fin) {
        return this->send(data, len, opcode, fin, this->_useMasking);
    }

    bool WebsocketsEndpoint::send(const WSString& data, const uint8_t opcode, const bool fin) {
        return this->send(data, opcode, fin, this->_useMasking);
    }

    bool WebsocketsEndpoint::send(const WSString& data, const uint8_t opcode, const bool fin, const bool mask, const char* maskingKey) { 
        return send(data.c_str(), data.size(), opcode, fin, mask, maskingKey);
    }

    std::string WebsocketsEndpoint::getHeader(uint64_t len, uint8_t opcode, bool fin, bool mask) {
      std::string header_data;
      
        if(len < 126) {
            auto header = MakeHeader<Header>(len, opcode, fin, mask);
            header_data = std::string(reinterpret_cast<char*>(&header), 2 + 0);
        } else if(len < 65536) {
            auto header = MakeHeader<HeaderWithExtended16>(len, opcode, fin, mask);
            header.extendedPayload = (len << 8) | (len >> 8);
            header_data = std::string(reinterpret_cast<char*>(&header), 2 + 2);
        } else {
            auto header = MakeHeader<HeaderWithExtended64>(len, opcode, fin, mask);
            // header.extendedPayload = swapEndianess(len);
            header.extendedPayload = swapEndianess(len);

            header_data = std::string(reinterpret_cast<char*>(&header), 2);
            header_data += std::string(reinterpret_cast<char*>(&header.extendedPayload), 8);
        }

        return header_data;
    }

    void remaskData(WSString& data, const char* const maskingKey, size_t first, size_t len) {
      for (size_t i = first; i < first + len; i++) {
        data[i] = data[i] ^ maskingKey[i % 4];
      }
    }

    bool WebsocketsEndpoint::send(const char* data, const size_t len, const uint8_t opcode, const bool fin, const bool mask, const char* maskingKey) {

#ifdef _WS_CONFIG_MAX_MESSAGE_SIZE
        if(len > _WS_CONFIG_MAX_MESSAGE_SIZE) {
            return false;
        }
#endif
        // send the header
        std::string message_data = getHeader(len, opcode, fin, mask);

        if (mask) {
          message_data += std::string(maskingKey, 4);
        }

        size_t data_start = message_data.size();
        message_data += std::string(data, len);

        if (mask && memcmp(maskingKey, __TINY_WS_INTERNAL_DEFAULT_MASK, 4) != 0) {
          remaskData(message_data, maskingKey, data_start, len);
        }

        this->_client->send(reinterpret_cast<const uint8_t*>(message_data.c_str()), message_data.size());
        return true; // TODO dont assume success
    }

    void WebsocketsEndpoint::close(CloseReason reason) {
        this->_closeReason = reason;
        
        if(!this->_client->available()) return;

        if(reason == CloseReason_None) {
            send(nullptr, 0, internals::ContentType::Close, true, this->_useMasking);
        } else {
            uint16_t reasonNum = static_cast<uint16_t>(reason);
            reasonNum = (reasonNum >> 8) | (reasonNum << 8);
            send(reinterpret_cast<const char*>(&reasonNum), 2, internals::ContentType::Close, true, this->_useMasking);
        }
        this->_client->close();
    }

    CloseReason WebsocketsEndpoint::getCloseReason() const {
        return _closeReason;
    }

    bool WebsocketsEndpoint::ping(const WSString& msg) {
        // Ping data must be shorter than 125 bytes
        if(msg.size() > 125) {
            return false;
        }
        else {
            return send(msg, ContentType::Ping, true, this->_useMasking);
        }
    }
    bool WebsocketsEndpoint::ping(const WSString&& msg) {
        // Ping data must be shorter than 125 bytes
        if(msg.size() > 125) {
            return false;
        }
        else {
            return send(msg, ContentType::Ping, true, this->_useMasking);
        }
    }

    bool WebsocketsEndpoint::pong(const WSString& msg) {
        // Pong data must be shorter than 125 bytes
        if(msg.size() > 125)  {
            return false;
        }
        else {
            return this->send(msg, ContentType::Pong, true, this->_useMasking);
        }
    }
    bool WebsocketsEndpoint::pong(const WSString&& msg) {
        // Pong data must be shorter than 125 bytes
        if(msg.size() > 125)  {
            return false;
        }
        else {
            return this->send(msg, ContentType::Pong, true, this->_useMasking);
        }
    }

    void WebsocketsEndpoint::setFragmentsPolicy(FragmentsPolicy newPolicy) {
        this->_fragmentsPolicy = newPolicy;
    }

    FragmentsPolicy WebsocketsEndpoint::getFragmentsPolicy() const {
        return this->_fragmentsPolicy;
    }

    WebsocketsEndpoint::~WebsocketsEndpoint() {}
}} // websockets::internals
