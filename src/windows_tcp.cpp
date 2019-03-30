#include <tiny_websockets/network/windows/win_tcp_client.hpp>
#include <tiny_websockets/network/windows/win_tcp_server.hpp>

#ifdef _WIN32

// Taken from: https://stackoverflow.com/questions/1869689/is-it-possible-to-tell-if-wsastartup-has-been-called-in-a-process
bool isWinsockInitialized() {
    SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (s == INVALID_SOCKET && WSAGetLastError() == WSANOTINITIALISED){
        return false;
    }

    closesocket(s);
    return true;
}

// Client Impl
namespace websockets { namespace network {
  /*
    Note: Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
    With MSVC:
    #pragma comment (lib, "Ws2_32.lib")
    #pragma comment (lib, "Mswsock.lib")
    #pragma comment (lib, "AdvApi32.lib")
  */
  SOCKET windowsTcpConnect(WSString host, int port) {
    WSADATA wsaData;
    SOCKET connectSocket = INVALID_SOCKET;
    struct addrinfo *result = NULL,
      *ptr = NULL,
      hints;

    // Initialize Winsock if not already initialized
    if(isWinsockInitialized() == false) {
      int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
      if (iResult != 0) {
        return INVALID_SOCKET;
      }
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // Resolve the server address and port
    int iResult = getaddrinfo(host.c_str(), std::to_string(port).c_str(), &hints, &result);
    if (iResult != 0) {
      return INVALID_SOCKET;
    }

    // Attempt to connect to an address until one succeeds
    for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

      // Create a SOCKET for connecting to server
      connectSocket = socket(ptr->ai_family, ptr->ai_socktype,
        ptr->ai_protocol);
      if (connectSocket == INVALID_SOCKET) {
        return INVALID_SOCKET;
      }

      // Connect to server.
      iResult = connect(connectSocket, ptr->ai_addr, static_cast<int>(ptr->ai_addrlen));
      if (iResult == SOCKET_ERROR) {
        closesocket(connectSocket);
        connectSocket = INVALID_SOCKET;
        continue;
      }
      break;
    }

    freeaddrinfo(result);

    return connectSocket;
  }

  // Returns true if an error occured
  bool windowsTcpSend(const uint8_t* buffer, const uint32_t len, const SOCKET socket) {
    // Send an initial buffer
    const char* cBuffer = reinterpret_cast<const char*>(buffer);

    int iResult = send(socket, cBuffer, len, 0);
    if (iResult == SOCKET_ERROR) {
      //printf("send failed with error: %d\n", WSAGetLastError());
      return true;
    }

    return false;
  }

#ifndef MSG_WAITALL
#define MSG_WAITALL 0x8 /* do not complete until packet is completely filled */
#endif

  // Returns true if error occured
  bool windowsTcpRecive(uint8_t* buffer, uint32_t len, SOCKET socket) {
    // Receive until the peer closes the connection
    int iResult = recv(socket, reinterpret_cast<char*>(buffer), len, MSG_WAITALL);
    if (iResult > 0) {
      return false;
    }
    else {
      return true;
    }
  }

  WinTcpClient::WinTcpClient(SOCKET s) : socket(s) {
    // Empty
  }

  bool WinTcpClient::connect(const WSString& host, const int port) {
    this->socket = windowsTcpConnect(host, port);
    return available();
  }

  bool WinTcpClient::poll() {
    fd_set readSet;
    FD_ZERO(&readSet);
    FD_SET(this->socket, &readSet);
    timeval timeout;
    timeout.tv_sec = 0;  // Zero timeout (poll)
    timeout.tv_usec = 0;
    return select(this->socket, &readSet, NULL, NULL, &timeout) == 1;
  }

  bool WinTcpClient::available() {
    return socket != INVALID_SOCKET;
  }

  void WinTcpClient::send(const WSString& data) {
    this->send(reinterpret_cast<uint8_t*>(const_cast<char*>(data.c_str())), data.size());
  }

  void WinTcpClient::send(const WSString&& data) {
    this->send(reinterpret_cast<uint8_t*>(const_cast<char*>(data.c_str())), data.size());
  }

  void WinTcpClient::send(const uint8_t* data, const uint32_t len) {
    auto error = windowsTcpSend(data, len, this->socket);
    if(error) close();
  }

  WSString WinTcpClient::readLine() {
    uint8_t byte = '0';
    WSString line;
    auto error = windowsTcpRecive(&byte, 1, this->socket);
    while (!error) {
      line += static_cast<char>(byte);
      if (byte == '\n') break;
      error = windowsTcpRecive(&byte, 1, this->socket);
    }
    if(error) close();
    return line;
  }
  void WinTcpClient::read(uint8_t* buffer, const uint32_t len) {
    auto error = windowsTcpRecive(buffer, len, this->socket);
    if(error) close();
  }

  void WinTcpClient::close() {
    if(socket != INVALID_SOCKET) {
      socket = INVALID_SOCKET;
      // shutdown the connection since no more data will be sent/received
      shutdown(socket, SD_BOTH);
      closesocket(socket);
      // TODO WSA cleanup shouldnt be called multiple times?
      //WSACleanup();
    }
  }
  
  int WinTcpClient::getSocket() const { 
    return socket; 
  }

  WinTcpClient::~WinTcpClient() {
    close();
  }
}} // websockets::network


// Server Impl
namespace websockets { namespace network {
  /*
    Note: Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
    With MSVC:
    #pragma comment (lib, "Ws2_32.lib")
    #pragma comment (lib, "Mswsock.lib")
    #pragma comment (lib, "AdvApi32.lib")
  */

  SOCKET getWindowsServerSocket(int port) {
    WSADATA wsaData;
    // Initialize Winsock if not already initialized
    if(isWinsockInitialized() == false) {
      int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
      if (iResult != 0) {
        return INVALID_SOCKET;
      }
    }
    
    struct addrinfo hints;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    struct addrinfo *result = NULL;
    // Resolve the server address and port
    int iResult = getaddrinfo(NULL, std::to_string(port).c_str(), &hints, &result);
    if ( iResult != 0 ) {
      //printf("getaddrinfo failed with error: %d\n", iResult);
      //WSACleanup();
      return INVALID_SOCKET;
    }


    // Create a SOCKET for connecting to server
    auto serverSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (serverSocket == INVALID_SOCKET) {
      //printf("socket failed with error: %d\n", WSAGetLastError());
      freeaddrinfo(result);
      //WSACleanup();
      return INVALID_SOCKET;
    }

    // Setup the TCP listening socket
    iResult = bind( serverSocket, result->ai_addr, static_cast<int>(result->ai_addrlen));
    if (iResult == SOCKET_ERROR) {
      //printf("bind failed with error: %d\n", WSAGetLastError());
      freeaddrinfo(result);
      closesocket(serverSocket);
      //WSACleanup();
      return INVALID_SOCKET;
    }

    freeaddrinfo(result);

    iResult = listen(serverSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR) {
      //printf("listen failed with error: %d\n", WSAGetLastError());
      closesocket(serverSocket);
      //WSACleanup();
      return INVALID_SOCKET;
    }

    return serverSocket;  
  }

  bool WinTcpServer::listen(uint16_t port) {
    this->socket = getWindowsServerSocket(port);
    return this->available();
  }

  SOCKET windowsAccept(SOCKET serverSocket) {
    // Accept a client socket
    auto clientSocket = accept(serverSocket, NULL, NULL);
    if (clientSocket == INVALID_SOCKET) {
      //printf("accept failed with error: %d\n", WSAGetLastError());
    }
    return clientSocket;
  }

  TcpClient* WinTcpServer::accept() {
    auto soc = windowsAccept(this->socket);
    if(soc == INVALID_SOCKET) {
      close();
    }
    return new WinTcpClient(soc);
  }
  
  
  bool WinTcpServer::available() {
    return this->socket != INVALID_SOCKET;
  }
  
  
  bool WinTcpServer::poll() {
    fd_set readSet;
    FD_ZERO(&readSet);
    FD_SET(this->socket, &readSet);
    timeval timeout;
    timeout.tv_sec = 0;  // Zero timeout (poll)
    timeout.tv_usec = 0;
    return select(this->socket, &readSet, NULL, NULL, &timeout) == 1;
  }
  
  void WinTcpServer::close() {
    closesocket(this->socket);
    this->socket = INVALID_SOCKET;
    
    // TODO: WSACleanup should only called once per application??
    // WSACleanup();
  }

  int WinTcpServer::getSocket() const{ 
    return socket; 
  }

  WinTcpServer::~WinTcpServer() {
    this->close();
  }

}} // websockets::network

#endif // #ifdef _WIN32