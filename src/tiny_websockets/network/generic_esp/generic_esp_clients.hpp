#pragma once

#include <tiny_websockets/internals/ws_common.hpp>
#include <tiny_websockets/network/tcp_client.hpp>

namespace websockets { namespace network {
  template <class WifiClientImpl> 
  class GenericEspTcpClient : public TcpClient {
  public:
    GenericEspTcpClient(WifiClientImpl c) : client(c) {
      client.setNoDelay(true);
    }
    
    GenericEspTcpClient() {}

    bool connect(const WSString& host, const int port) {
      yield();
      auto didConnect = client.connect(host.c_str(), port);
      client.setNoDelay(true);
      return didConnect;
    }

    bool poll() {
      yield();
      return client.available();
    }

    bool available() override {
      return client.connected();
    }

    void send(const WSString& data) override {
      yield();
      client.write(reinterpret_cast<uint8_t*>(const_cast<char*>(data.c_str())), data.size());
      yield();
    }

    void send(const WSString&& data) override {
      yield();
      client.write(reinterpret_cast<uint8_t*>(const_cast<char*>(data.c_str())), data.size());
      yield();
    }

    void send(const uint8_t* data, const uint32_t len) override {
      yield();
      client.write(data, len);
      yield();
    }
    
    WSString readLine() override {
      WSString line = "";

      int ch = -1;

      const uint64_t millisBeforeReadingHeaders = millis();
      while( ch != '\n' && available()) {
        if (millis() - millisBeforeReadingHeaders > _CONNECTION_TIMEOUT) return "";
        ch = client.read();
        if (ch < 0) continue;
        line += (char) ch;
      }

      return line;
    }

    uint32_t read(uint8_t* buffer, const uint32_t len) override {
      yield();
      return client.read(buffer, len);
    }

    void close() override {
      yield();
      client.stop();
    }

    virtual ~GenericEspTcpClient() {
      client.stop();
    }

  protected:
    WifiClientImpl client;

    int getSocket() const override {
      return -1;
    }    
  };
}} // websockets::network
