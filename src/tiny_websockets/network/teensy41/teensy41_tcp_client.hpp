#pragma once

#ifdef ARDUINO_TEENSY41

#include <tiny_websockets/internals/ws_common.hpp>
#include <tiny_websockets/network/tcp_client.hpp>
#include <tiny_websockets/network/tcp_server.hpp>

#include <NativeEthernet.h>

namespace websockets { namespace network {
  class Teensy41TcpClient : public TcpClient {
  public:
    Teensy41TcpClient(EthernetClient c) : client(c) {}

    Teensy41TcpClient() {}

    bool connect(const WSString& host, const int port) {
      yield();
      const char* hostStr = host.c_str();
      // Teensy's NativeEthernet library doesn't accept a char buffer
      // as an IP (it will try to resolve it). So we have to convert
      // it if necessary.
      IPAddress ip;
      return (ip.fromString(hostStr)
        ? client.connect(ip, port)
        : client.connect(hostStr, port)
      );
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
      while( ch != '\n' && available()) {
        // It is important to call `client.available()`. Otherwise no data can be read.
        if (client.available()) {
          ch = client.read();
          if (ch >= 0) {
            line += (char) ch;
          }
        }
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

    virtual ~Teensy41TcpClient() {
      client.stop();
    }

  protected:
    EthernetClient client;

    int getSocket() const override {
      return -1;
    }    
  };  
}} // websockets::network

#endif // #ifdef ARDUINO_TEENSY41 