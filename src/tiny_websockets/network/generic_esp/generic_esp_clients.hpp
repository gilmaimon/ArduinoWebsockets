#pragma once

#include <tiny_websockets/internals/ws_common.hpp>
#include <tiny_websockets/network/tcp_client.hpp>

namespace websockets { namespace network {
  template <class WifiClientImpl> 
  class GenericEspTcpClient : public TcpClient {
  public:
    GenericEspTcpClient(WifiClientImpl c) : client(c) {
      client.setNoDelay(true);
      yield();
    }
    
    GenericEspTcpClient() {}

    bool connect(const WSString& host, const int port) {
      yield();
      auto didConnect = client.connect(host.c_str(), port);
      client.setNoDelay(true);
      yield();
      return didConnect;
    }

    bool poll() {
      yield();
      return client.available();
    }

    bool available() override {
      yield();
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
      int val;
      WSString line;
      do {
        yield();
        val = client.read();
        if(val < 0) continue;
        line += (char)val;
      } while(val != '\n');
      if(!available()) close();
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
