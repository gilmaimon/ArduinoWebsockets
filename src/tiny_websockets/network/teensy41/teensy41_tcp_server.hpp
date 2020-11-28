#pragma once

#ifdef ARDUINO_TEENSY41 

#include <tiny_websockets/internals/ws_common.hpp>
#include <tiny_websockets/network/tcp_client.hpp>
#include <tiny_websockets/network/tcp_server.hpp>
#include <tiny_websockets/network/teensy41/teensy41_tcp_client.hpp>

#include <NativeEthernet.h>

namespace websockets { namespace network {
  class Teensy41TcpServer : public TcpServer {
  public:
    Teensy41TcpServer() {}

    bool poll() override {
      yield();
      return server.available();
    }

    bool listen(const uint16_t port) override {
      yield();
      server = EthernetServer(port);
      server.begin(port);
      return available();
    }
    
    TcpClient* accept() override {
      auto client = server.accept();
      return new Teensy41TcpClient(client);
    }

    bool available() override {
      yield();
      return static_cast<bool>(server);
    }
    
    void close() override {
      // Not Implemented
    }

    virtual ~Teensy41TcpServer() {
      // Not Implemented
    }

  protected:
    int getSocket() const override {
      return -1; // Not Implemented
    }
  
  private:
    EthernetServer server;
  };
}} // websockets::network

#endif // #ifdef ARDUINO_TEENSY41 
