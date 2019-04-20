#pragma once

#include <tiny_websockets/internals/ws_common.hpp>
#include <tiny_websockets/network/tcp_client.hpp>
#include <tiny_websockets/internals/data_frame.hpp>
#include <tiny_websockets/internals/websockets_endpoint.hpp>
#include <tiny_websockets/message.hpp>
#include <memory>
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

  class WebsocketsClient {
  public:
    WebsocketsClient();
    WebsocketsClient(std::shared_ptr<network::TcpClient> client);
    
    WebsocketsClient(const WebsocketsClient& other);
    WebsocketsClient(const WebsocketsClient&& other);
    
    WebsocketsClient& operator=(const WebsocketsClient& other);
    WebsocketsClient& operator=(const WebsocketsClient&& other);

    bool connect(const WSInterfaceString url);
    bool connect(const WSInterfaceString host, const int port, const WSInterfaceString path);
    
    void onMessage(const MessageCallback callback);
    void onMessage(const PartialMessageCallback callback);

    void onEvent(const EventCallback callback);
    void onEvent(const PartialEventCallback callback);

    bool poll();
    bool available(const bool activeTest = false);

    bool send(const WSInterfaceString&& data);
    bool send(const WSInterfaceString& data);
    bool send(const char* data);
    bool send(const char* data, const size_t len);

    bool sendBinary(const WSInterfaceString data);
    bool sendBinary(const char* data, const size_t len);

    // stream messages
    bool stream(const WSInterfaceString data = "");
    bool streamBinary(const WSInterfaceString data = "");
    bool end(const WSInterfaceString data = "");
    
    void setFragmentsPolicy(const FragmentsPolicy newPolicy);
    FragmentsPolicy getFragmentsPolicy() const;
    
    WebsocketsMessage readBlocking();

    bool ping(const WSInterfaceString data = "");
    bool pong(const WSInterfaceString data = "");

    void close(const CloseReason reason = CloseReason_NormalClosure);
    CloseReason getCloseReason() const;

  #ifdef ESP8266
    void setFingerprint(const char* fingerprint);
    void setInsecure();
  #elif defined(ESP32)
    void setCACert(const char* ca_cert);
  #endif

    virtual ~WebsocketsClient();

  private:
    std::shared_ptr<network::TcpClient> _client;
    internals::WebsocketsEndpoint _endpoint;
    bool _connectionOpen;
    MessageCallback _messagesCallback;
    EventCallback _eventsCallback;
    enum SendMode {
      SendMode_Normal,
      SendMode_Streaming
    } _sendMode;


  #ifdef ESP8266
    const char* _optional_ssl_fingerprint = nullptr;
  #elif defined(ESP32)
    const char* _optional_ssl_ca_cert = nullptr;
  #endif

    void _handlePing(WebsocketsMessage);
    void _handlePong(WebsocketsMessage);
    void _handleClose(WebsocketsMessage);

    void upgradeToSecuredConnection();
  };
}