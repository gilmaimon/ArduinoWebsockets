#ifdef __linux__

#include <tiny_websockets/network/linux/linux_tcp_client.hpp>

#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <netdb.h>

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
		auto res = send(_socket, data, len, 0);
		if(res < 0) {
			return false;
		}

		return true;
	}

	bool linuxTcpRead(int _socket, uint8_t* buffer, size_t len) {
		return read(_socket, buffer, len) > 0;
	}

	LinuxTcpSocket::LinuxTcpSocket() : _socket(INVALID_SOCKET) {
		// Empty
	}

	bool LinuxTcpSocket::connect(WSString host, int port) {
		this->_socket = linuxTcpConnect(host, port);
		return available();
	}

	bool LinuxTcpSocket::poll() {
		int count;
		ioctl(this->_socket, FIONREAD, &count);
		return count > 0;
	}
	bool LinuxTcpSocket::available() {
		return this->_socket != INVALID_SOCKET;
	}
	void LinuxTcpSocket::send(WSString data) {
		return this->send(
			reinterpret_cast<uint8_t*>(const_cast<char*>(data.c_str())),
			data.size()
			);
	}
	void LinuxTcpSocket::send(uint8_t* data, uint32_t len) {
		if(!available()) return;// false;

		auto success = linuxTcpSend(
			this->_socket,
			data,
			len
		);

		if(!success) close();
		// return success;
	}
	
	WSString LinuxTcpSocket::readLine() {
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
	
	void LinuxTcpSocket::read(uint8_t* buffer, uint32_t len) {
		linuxTcpRead(this->_socket, buffer, len);
	}

	void LinuxTcpSocket::close()  {
		linuxTcpClose(this->_socket);
	}
	
	LinuxTcpSocket::~LinuxTcpSocket() {
		if(available()) close();
	}
}} // websockets::network

#endif // #ifdef __linux__