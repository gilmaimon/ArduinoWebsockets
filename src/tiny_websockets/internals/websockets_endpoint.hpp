#pragma once

#include <tiny_websockets/internals/ws_common.hpp>
#include <tiny_websockets/network/tcp_client.hpp>
#include <tiny_websockets/internals/data_frame.hpp>
#include <tiny_websockets/message.hpp>

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
        WebsocketsEndpoint(network::TcpClient* socket, FragmentsPolicy fragmentsPolicy = FragmentsPolicy_Aggregate);

        WebsocketsEndpoint(const WebsocketsEndpoint& other);
        WebsocketsEndpoint(const WebsocketsEndpoint&& other);
        
        WebsocketsEndpoint& operator=(const WebsocketsEndpoint& other);
        WebsocketsEndpoint& operator=(const WebsocketsEndpoint&& other);

        bool poll();
        WebsocketsMessage recv();
        bool send(const char* data, size_t len, uint8_t opcode, bool fin = true, bool mask = false, uint8_t maskingKey[4] = nullptr);    
        bool send(WSString data, uint8_t opcode, bool fin = true, bool mask = false, uint8_t maskingKey[4] = nullptr);    
        
        bool ping(WSString msg = "");
        bool pong(WSString msg = "");

        void close(CloseReason reason = CloseReason_NormalClosure);
        CloseReason getCloseReason();

        void setFragmentsPolicy(FragmentsPolicy newPolicy);
        FragmentsPolicy getFragmentsPolicy();

        virtual ~WebsocketsEndpoint();
    private:
        network::TcpClient* _client;
        FragmentsPolicy _fragmentsPolicy;
        enum RecvMode {
            RecvMode_Normal,
            RecvMode_Streaming
        } _recvMode;
        WebsocketsMessage::StreamBuilder _streamBuilder;
        CloseReason _closeReason;

        WebsocketsFrame _recv();
        void handleMessageInternally(WebsocketsMessage& msg);

        WebsocketsMessage handleFrameInStreamingMode(WebsocketsFrame& frame);
        WebsocketsMessage handleFrameInStandardMode(WebsocketsFrame& frame);

        bool sendHeader(uint64_t len, uint8_t opcode, bool fin, bool mask);
    };
}} // websockets::internals