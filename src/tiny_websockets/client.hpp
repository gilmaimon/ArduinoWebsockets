#pragma once

#include <tiny_websockets/internals/ws_common.hpp>
#include <tiny_websockets/network/tcp_client.hpp>
#include <tiny_websockets/internals/data_frame.hpp>
#include <tiny_websockets/internals/websockets_endpoint.hpp>
#include <tiny_websockets/message.hpp>
#include <memory>
#include <functional>
#include <vector>

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

    void addHeader(const WSInterfaceString key, const WSInterfaceString value);

    bool connect(const WSInterfaceString url);
    bool connect(const WSInterfaceString host, const int port, const WSInterfaceString path);
    bool connectSecure(const WSInterfaceString host, const int port, const WSInterfaceString path);
      
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

    void setUseMasking(bool useMasking) {
      _endpoint.setUseMasking(useMasking);
    }

    void setInsecure();
  #ifdef ESP8266
    void setFingerprint(const char* fingerprint);
    void setClientRSACert(const X509List *cert, const PrivateKey *sk);
	void setClientECCert(const X509List *cert, const PrivateKey *sk);
    void setTrustAnchors(const X509List *ta);
	void setKnownKey(const PublicKey *pk);
  #elif defined(ESP32)
    void setCACert(const char* ca_cert);
    void setCertificate(const char* client_ca);
    void setPrivateKey(const char* private_key);
  #endif

    virtual ~WebsocketsClient();

  private:
    std::shared_ptr<network::TcpClient> _client;
    std::vector<std::pair<WSString, WSString>> _customHeaders;
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
    const X509List* _optional_ssl_trust_anchors = nullptr;
	const PublicKey* _optional_ssl_known_key = nullptr;
    const X509List* _optional_ssl_rsa_cert = nullptr;
    const PrivateKey* _optional_ssl_rsa_private_key = nullptr;
	const X509List* _optional_ssl_ec_cert = nullptr;
    const PrivateKey* _optional_ssl_ec_private_key = nullptr;
  #elif defined(ESP32)
    const char* _optional_ssl_ca_cert = nullptr;
    const char* _optional_ssl_client_ca = nullptr;
    const char* _optional_ssl_private_key = nullptr;
  #endif

    void _handlePing(WebsocketsMessage);
    void _handlePong(WebsocketsMessage);
    void _handleClose(WebsocketsMessage);

    void upgradeToSecuredConnection();
  };
}
