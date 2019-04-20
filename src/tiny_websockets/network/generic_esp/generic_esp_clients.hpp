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
