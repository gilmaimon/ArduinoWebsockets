#pragma once

#ifdef ESP32 

#include <tiny_websockets/internals/ws_common.hpp>
#include <tiny_websockets/network/tcp_client.hpp>
#include <tiny_websockets/network/tcp_server.hpp>
#include <Arduino.h>

#include <WiFi.h>

namespace websockets { namespace network {
  class Esp32TcpClient : public TcpClient {
  public:
    Esp32TcpClient() {}
    Esp32TcpClient(WiFiClient c) : client(c) {}

    bool connect(WSString host, int port) {
      return client.connect(host.c_str(), port);
    }

    bool poll() {
      return client.available();
    }

    bool available() override {
      return static_cast<bool>(client);
    }

    void send(WSString data) override {
      client.write(reinterpret_cast<uint8_t*>(const_cast<char*>(data.c_str())), data.size());
    }

    void send(uint8_t* data, uint32_t len) override {
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

    void read(uint8_t* buffer, uint32_t len) override {
      client.read(buffer, len);
    }

    void close() override {
      client.stop();
    }

    virtual ~Esp32TcpClient() {
      close();
    }
  private:
    WiFiClient client;
  };

  class Esp32TcpServer : public TcpServer {
  public:
    Esp32TcpServer() {}
    bool poll() override {
      return server.hasClient();
    }

    bool listen(uint16_t port) override {
      server = WiFiServer(port);
      server.begin(port);
      return available();
    }
    
    TcpClient* accept() override {
      while(available()) {
        auto client = server.available();
        if(client) {
          return new Esp32TcpClient{client};
        }
      }
      return new Esp32TcpClient;
    }

    bool available() override {
      return static_cast<bool>(server);
    }
    
    void close() override {
      server.close();
    }

    virtual ~Esp32TcpServer() {
      if(available()) close();
    }
  private:
    WiFiServer server;
  };
}} // websockets::network

#endif // #ifdef ESP32 