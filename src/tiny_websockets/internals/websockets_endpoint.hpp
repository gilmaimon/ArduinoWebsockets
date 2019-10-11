#pragma once

#include <tiny_websockets/internals/ws_common.hpp>
#include <tiny_websockets/network/tcp_client.hpp>
#include <tiny_websockets/internals/data_frame.hpp>
#include <tiny_websockets/message.hpp>
#include <memory>

#define __TINY_WS_INTERNAL_DEFAULT_MASK "\00\00\00\00"

namespace websockets {     
    enum FragmentsPolicy {
        FragmentsPolicy_Aggregate,
        FragmentsPolicy_Notify
    };

    enum CloseReason {
        CloseReason_None                =       -1,
        CloseReason_NormalClosure       =       1000,
        CloseReason_GoingAway           =       1001,
        CloseReason_ProtocolError       =       1002,
        CloseReason_UnsupportedData     =       1003,
        CloseReason_NoStatusRcvd        =       1005,
        CloseReason_AbnormalClosure     =       1006,
        CloseReason_InvalidPayloadData  =       1007,
        CloseReason_PolicyViolation     =       1008,
        CloseReason_MessageTooBig       =       1009,
        CloseReason_InternalServerError =       1011,
    };

    CloseReason GetCloseReason(uint16_t reasonCode);
    
    namespace internals {
    
    class WebsocketsEndpoint {
    public:
        WebsocketsEndpoint(std::shared_ptr<network::TcpClient> socket, FragmentsPolicy fragmentsPolicy = FragmentsPolicy_Aggregate);

        WebsocketsEndpoint(const WebsocketsEndpoint& other);
        WebsocketsEndpoint(const WebsocketsEndpoint&& other);
        
        WebsocketsEndpoint& operator=(const WebsocketsEndpoint& other);
        WebsocketsEndpoint& operator=(const WebsocketsEndpoint&& other);

        void setInternalSocket(std::shared_ptr<network::TcpClient> socket);

        bool poll();
        WebsocketsMessage recv();
        bool send(const char* data, const size_t len, const uint8_t opcode, const bool fin, const bool mask, const char* maskingKey = __TINY_WS_INTERNAL_DEFAULT_MASK);    
        bool send(const WSString& data, const uint8_t opcode, const bool fin, const bool mask, const char* maskingKey = __TINY_WS_INTERNAL_DEFAULT_MASK);
        
        bool send(const char* data, const size_t len, const uint8_t opcode, const bool fin);    
        bool send(const WSString& data, const uint8_t opcode, const bool fin);
        
        bool ping(const WSString& msg);
        bool ping(const WSString&& msg);

        bool pong(const WSString& msg);
        bool pong(const WSString&& msg);

        void close(const CloseReason reason = CloseReason_NormalClosure);
        CloseReason getCloseReason() const;

        void setFragmentsPolicy(const FragmentsPolicy newPolicy);
        FragmentsPolicy getFragmentsPolicy() const;

        void setUseMasking(bool useMasking) {
            _useMasking = useMasking;
        }

        virtual ~WebsocketsEndpoint();
    private:
        std::shared_ptr<network::TcpClient> _client;
        FragmentsPolicy _fragmentsPolicy;
        enum RecvMode {
            RecvMode_Normal,
            RecvMode_Streaming
        } _recvMode;
        WebsocketsMessage::StreamBuilder _streamBuilder;
        CloseReason _closeReason;
        bool _useMasking = true;

        WebsocketsFrame _recv();
        void handleMessageInternally(WebsocketsMessage& msg);

        WebsocketsMessage handleFrameInStreamingMode(WebsocketsFrame& frame);
        WebsocketsMessage handleFrameInStandardMode(WebsocketsFrame& frame);

        std::string getHeader(uint64_t len, uint8_t opcode, bool fin, bool mask);
    };
}} // websockets::internals