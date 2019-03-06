#pragma once

#include <tiny_websockets/internals/ws_common.hpp>
#include <tiny_websockets/client.hpp>
#include <functional>

namespace websockets {
  class WebsocketsServer {
  public:
    WebsocketsServer(network::TcpServer* server = new WSDefaultTcpServer);
    
    WebsocketsServer(const WebsocketsServer& other) = delete;
    WebsocketsServer(const WebsocketsServer&& other) = delete;
    
    WebsocketsServer& operator=(const WebsocketsServer& other) = delete;
    WebsocketsServer& operator=(const WebsocketsServer&& other) = delete;

    bool available();
    void listen(uint16_t port);
    bool poll();
    WebsocketsClient accept();

    virtual ~WebsocketsServer();

  private:
    network::TcpServer* _server;
  };
}