#pragma once

#ifdef ESP8266 

#include <tiny_websockets/internals/ws_common.hpp>
#include <tiny_websockets/network/tcp_client.hpp>
#include <tiny_websockets/network/tcp_server.hpp>

#include <ESP8266WiFi.h>

namespace websockets { namespace network {
  class Esp8266TcpClient : public TcpClient {
  public:
    Esp8266TcpClient(WiFiClient c) : client(c) {
      client.setNoDelay(true);
    }
    
    Esp8266TcpClient() {}

    bool connect(const WSString& host, const int port) {
      auto didConnect = client.connect(host.c_str(), port);
      client.setNoDelay(true);
      return didConnect;
    }

    bool poll() {
      return client.available();
    }

    bool available() override {
      return client.connected();
    }

    void send(const WSString& data) override {
      client.write(reinterpret_cast<uint8_t*>(const_cast<char*>(data.c_str())), data.size());
    }

    void send(const WSString&& data) override {
      client.write(reinterpret_cast<uint8_t*>(const_cast<char*>(data.c_str())), data.size());
    }

    void send(const uint8_t* data, const uint32_t len) override {
      client.write(data, len);
    }
    
    WSString readLine() override {
      int val;
      WSString line;
      do {
        val = client.read();
        if(val < 0) continue;
        line += (char)val;
      } while(val != '\n');
      if(!available()) close();
      return line;
    }

    void read(uint8_t* buffer, const uint32_t len) override {
      client.read(buffer, len);
    }

    void close() override {
      client.stop();
    }

    virtual ~Esp8266TcpClient() {
      client.stop();
    }

  protected:
    int getSocket() const override {
      return -1;
    }
    
  private:
    WiFiClient client;
  };

  class SecuredEsp8266TcpClient : public TcpClient {
  public:
    SecuredEsp8266TcpClient(WiFiClientSecure c) : client(c) {
      client.setNoDelay(true);
    }
    
    SecuredEsp8266TcpClient() {
    }

    bool connect(const WSString& host, const int port) {
      auto didConnect = client.connect(host.c_str(), port);
      client.setNoDelay(true);
      return didConnect;
    }

    bool poll() {
      return client.available();
    }

    bool available() override {
      return client.connected();
    }

    void send(const WSString& data) override {
      client.write(reinterpret_cast<uint8_t*>(const_cast<char*>(data.c_str())), data.size());
    }

    void send(const WSString&& data) override {
      client.write(reinterpret_cast<uint8_t*>(const_cast<char*>(data.c_str())), data.size());
    }

    void send(const uint8_t* data, const uint32_t len) override {
      client.write(data, len);
    }
    
    WSString readLine() override {
      int val;
      WSString line;
      do {
        val = client.read();
        if(val < 0) continue;
        line += (char)val;
      } while(val != '\n');
      if(!available()) close();
      return line;
    }

    void read(uint8_t* buffer, const uint32_t len) override {
      client.read(buffer, len);
    }

    void close() override {
      client.stop();
    }

    void setInsecure() {
      client.setInsecure();
    }

    void setFingerprint(const char* fingerprint) {
      client.setFingerprint(fingerprint);
    }
    
    virtual ~SecuredEsp8266TcpClient() {
      client.stop();
    }

  protected:
    int getSocket() const override {
      return -1;
    }
    
  private:
    WiFiClientSecure client;
  };

  #define DUMMY_PORT 0

  class Esp8266TcpServer : public TcpServer {
  public:
    Esp8266TcpServer() : server(DUMMY_PORT) {}
    bool poll() override {
      return server.hasClient();
    }

    bool listen(const uint16_t port) override {
      server.begin(port);
      return available();
    }
    
    TcpClient* accept() override {
      while(available()) {
        auto client = server.available();
        if(client) return new Esp8266TcpClient{client};
      }
      return new Esp8266TcpClient;
    }

    bool available() override {
      return server.status() != CLOSED;
    }
    
    void close() override {
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