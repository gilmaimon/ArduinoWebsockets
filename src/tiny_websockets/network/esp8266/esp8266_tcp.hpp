#pragma once

#ifdef ESP8266 

#include <tiny_websockets/internals/ws_common.hpp>
#include <tiny_websockets/network/tcp_client.hpp>
#include <tiny_websockets/network/tcp_server.hpp>
#include <tiny_websockets/network/generic_esp/generic_esp_clients.hpp>

#include <ESP8266WiFi.h>

namespace websockets { namespace network {
  typedef GenericEspTcpClient<WiFiClient> Esp8266TcpClient;
  
  class SecuredEsp8266TcpClient : public GenericEspTcpClient<WiFiClientSecure> {
  public:
    void setInsecure() {
      this->client.setInsecure();
    }

    void setFingerprint(const char* fingerprint) {
      this->client.setFingerprint(fingerprint);
    }

	void setKnownKey(const PublicKey *pk) {
		this->client.setKnownKey(pk);
	}

    void setTrustAnchors(const X509List *ta){
      this->client.setTrustAnchors(ta);
	}

    void setClientRSACert(const X509List *cert, const PrivateKey *sk) {
      this->client.setClientRSACert(cert, sk);
	}

	void setClientECCert(const X509List *cert, const PrivateKey *sk) {
      this->client.setClientECCert(cert, sk, 0xFFFF, 0);
	}
  };

  #define DUMMY_PORT 0

  class Esp8266TcpServer : public TcpServer {
  public:
    Esp8266TcpServer() : server(DUMMY_PORT) {}
    bool poll() override {
      yield();
      return server.hasClient();
    }

    bool listen(const uint16_t port) override {
      yield();
      server.begin(port);
      return available();
    }
    
    TcpClient* accept() override {
      while(available()) {
        yield();
        auto client = server.available();
        if(client) return new Esp8266TcpClient{client};
      }
      return new Esp8266TcpClient;
    }

    bool available() override {
      yield();
      return server.status() != CLOSED;
    }
    
    void close() override {
      yield();
      server.close();
    }

    virtual ~Esp8266TcpServer() {
      if(available()) close();
    }
  
  protected:
    int getSocket() const override {
      return -1;
    }

  private:
    WiFiServer server;
  };
}} // websockets::network

#endif // #ifdef ESP8266 
