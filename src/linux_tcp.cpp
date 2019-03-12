#ifdef __linux__

#include <tiny_websockets/network/linux/linux_tcp_client.hpp>

#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <netdb.h>
#include <memory.h>

// Client impl
namespace websockets { namespace network {
  int linuxTcpConnect(WSString host, int port) {
    int _socket = socket(AF_INET , SOCK_STREAM , 0);
    if(_socket == INVALID_SOCKET) {
      return INVALID_SOCKET;
    }

    struct sockaddr_in server;

    if(inet_addr(host.c_str())) {
        struct hostent *he;
        struct in_addr **addr_list;
        if ( (he = gethostbyname( host.c_str() ) ) == NULL) {
          //"Failed to resolve hostname\n"
          return INVALID_SOCKET;
        }
      
      addr_list = reinterpret_cast<in_addr **>(he->h_addr_list);
        for(int i = 0; addr_list[i] != NULL; i++) {
        server.sin_addr = *addr_list[i];
        break;
        }
    }
    else {
      server.sin_addr.s_addr = inet_addr( host.c_str() );
    }
    server.sin_family = AF_INET;
    server.sin_port = htons( port );
    if (connect(_socket, reinterpret_cast<sockaddr *>(&server), sizeof(server)) < 0) {
      return INVALID_SOCKET;
    }

    return _socket;
  }

  bool linuxTcpClose(int _socket) {
    return close( _socket );
  }

  bool linuxTcpSend(int _socket, uint8_t* data, size_t len) {
    auto res = send(_socket, data, len, MSG_NOSIGNAL );
    if(res < 0) {
      return false;
    }

    return true;
  }

  bool linuxTcpRead(int _socket, uint8_t* buffer, size_t len) {
    return read(_socket, buffer, len) > 0;
  }

  LinuxTcpClient::LinuxTcpClient(int socket) : _socket(socket) {
    // Empty
  }

  bool LinuxTcpClient::connect(WSString host, int port) {
    this->_socket = linuxTcpConnect(host, port);
    return available();
  }

  bool LinuxTcpClient::poll() {
    int count;
    ioctl(this->_socket, FIONREAD, &count);
    return count > 0;
  }
  bool LinuxTcpClient::available() {
    return this->_socket != INVALID_SOCKET;
  }
  void LinuxTcpClient::send(WSString data) {
    return this->send(
      reinterpret_cast<uint8_t*>(const_cast<char*>(data.c_str())),
      data.size()
    );
  }
  void LinuxTcpClient::send(uint8_t* data, uint32_t len) {
    if(!available()) return;// false;

    auto success = linuxTcpSend(
      this->_socket,
      data,
      len
    );

    if(!success) close();
    // return success;
  }
  
  WSString LinuxTcpClient::readLine() {
    uint8_t byte = '0';
    WSString line;
    read(&byte, 1);
    while (available()) {
      line += static_cast<char>(byte);
      if (byte == '\n') break;
      read(&byte, 1);
    }
    if(!available()) close();
    return line;
  }
  
  void LinuxTcpClient::read(uint8_t* buffer, uint32_t len) {
    auto success = linuxTcpRead(this->_socket, buffer, len);
    if(!success) close();
  }

  void LinuxTcpClient::close()  {
    linuxTcpClose(this->_socket);
    this->_socket = INVALID_SOCKET;
  }
  
  LinuxTcpClient::~LinuxTcpClient() {
    if(available()) close();
  }
}} // websockets::network

// Server Impl
namespace websockets { namespace network {

  int linuxTcpServerInit(const size_t backlog, int port) {
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;
    
    // socket init
    auto sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
      return INVALID_SOCKET;
    }
    
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port);

    // binding
    if (bind(sockfd, reinterpret_cast<struct sockaddr *>(&serv_addr), sizeof(serv_addr)) < 0) {
      return INVALID_SOCKET;
    }

    // listen
    listen(sockfd, backlog);
    return sockfd;
  }

  bool LinuxTcpServer::listen(uint16_t port) {
    this->_socket = linuxTcpServerInit(this->_num_backlog, port);
    return this->available();
  }

  int linuxAccept(int serverSocket) {
    struct sockaddr_in cli_addr;
    socklen_t clilen = sizeof(cli_addr);
    
    // accept
    auto clientSock = accept(serverSocket, reinterpret_cast<struct sockaddr *>(&cli_addr), &clilen);
    
    // return default in case of accept error
    if (clientSock < 0) return INVALID_SOCKET;

    return clientSock;
  }

  TcpClient* LinuxTcpServer::accept() {
    auto clientSock = linuxAccept(this->_socket);
    return new LinuxTcpClient(clientSock);
  }

  bool LinuxTcpServer::available() {
    return this->_socket != INVALID_SOCKET;
  }

  bool LinuxTcpServer::poll() {
    fd_set readSet;
    FD_ZERO(&readSet);
    FD_SET(this->_socket, &readSet);
    timeval timeout;
    timeout.tv_sec = 0;  // Zero timeout (poll)
    timeout.tv_usec = 0;
    return select(this->_socket + 1, &readSet, NULL, NULL, &timeout) == 1;
  }

  void linuxSockClose(int socket) {
    close(socket);
  }

  void LinuxTcpServer::close() {
    linuxSockClose(this->_socket);
    this->_socket = INVALID_SOCKET;
  }

  LinuxTcpServer::~LinuxTcpServer() {
    close();
  }
}} // websockets::network

#endif // #ifdef __linux__
