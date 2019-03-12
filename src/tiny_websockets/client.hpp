#pragma once

#include <tiny_websockets/internals/ws_common.hpp>
#include <tiny_websockets/network/tcp_client.hpp>
#include <tiny_websockets/internals/data_frame.hpp>
#include <tiny_websockets/internals/websockets_endpoint.hpp>
#include <tiny_websockets/message.hpp>
#include <functional>

namespace websockets {
  enum class WebsocketsEvent {
    ConnectionOpened,
    ConnectionClosed,
    GotPing, GotPong
  };

  class WebsocketsClient;
    typedef std::function<void(WebsocketsClient&, WebsocketsMessage)> MessageCallback;
    typedef std::function<void(WebsocketsMessage)> PartialMessageCallback;
    
    typedef std::function<void(WebsocketsClient&, WebsocketsEvent, WSInterfaceString)> EventCallback;
    typedef std::function<void(WebsocketsEvent, WSInterfaceString)> PartialEventCallback;

  class WebsocketsClient : private internals::WebsocketsEndpoint {
  public:
    WebsocketsClient(network::TcpClient* client = new WSDefaultTcpClient);
    
    WebsocketsClient(const WebsocketsClient& other);
    WebsocketsClient(const WebsocketsClient&& other);
    
    WebsocketsClient& operator=(const WebsocketsClient& other);
    WebsocketsClient& operator=(const WebsocketsClient&& other);

    bool connect(WSInterfaceString url);
    bool connect(WSInterfaceString host, int port, WSInterfaceString path);
    
    void onMessage(MessageCallback callback);
    void onMessage(PartialMessageCallback callback);

    void onEvent(EventCallback callback);
    void onEvent(PartialEventCallback callback);

    bool poll();
    bool available(bool activeTest = false);

    bool send(WSInterfaceString data);
    bool send(const char* data, size_t len);

    bool sendBinary(WSInterfaceString data);
    bool sendBinary(const char* data, size_t len);

    // stream messages
    bool stream(WSInterfaceString data = "");
    bool streamBinary(WSInterfaceString data = "");
    bool end(WSInterfaceString data = "");
    
    void setFragmentsPolicy(FragmentsPolicy newPolicy);
    FragmentsPolicy getFragmentsPolicy();
    
    WebsocketsMessage readBlocking();

    bool ping(WSInterfaceString data = "");
    bool pong(WSInterfaceString data = "");

    void close(CloseReason reason = CloseReason_NormalClosure);
    CloseReason getCloseReason();

    virtual ~WebsocketsClient();

  private:
    network::TcpClient* _client;
    bool _connectionOpen;
    MessageCallback _messagesCallback;
    EventCallback _eventsCallback;
    enum SendMode {
      SendMode_Normal,
      SendMode_Streaming
    } _sendMode;

    void _handlePing(WebsocketsMessage);
    void _handlePong(WebsocketsMessage);
    void _handleClose(WebsocketsMessage);
  };
}