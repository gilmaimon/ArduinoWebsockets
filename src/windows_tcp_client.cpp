#include "network/windows/win_tcp_client.h"

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN

#undef _WIN32_WINNT
#define _WIN32_WINNT 0x501

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

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

		// Initialize Winsock
		int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (iResult != 0) {
			return INVALID_SOCKET;
		}

		ZeroMemory(&hints, sizeof(hints));
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;

		// Resolve the server address and port
		iResult = getaddrinfo(host.c_str(), std::to_string(port).c_str(), &hints, &result);
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
	bool windowsTcpSend(uint8_t* buffer, uint32_t len, SOCKET socket) {
		// Send an initial buffer
		const char* cBuffer = reinterpret_cast<const char*>(buffer);

		int iResult = send(socket, cBuffer, len, 0);
		if (iResult == SOCKET_ERROR) {
			//printf("send failed with error: %d\n", WSAGetLastError());
			return true;
		}

		return false;
	}

	// Returns true if error occured
	bool windowsTcpRecive(uint8_t* buffer, uint32_t len, SOCKET socket) {
		// Receive until the peer closes the connection
		int iResult = recv(socket, reinterpret_cast<char*>(buffer), len, 0);
		if (iResult > 0) {
			return false;
		}
		else {
			return true;
		}
	}

	bool WinTcpSocket::connect(WSString host, int port) {
		this->socket = windowsTcpConnect(host, port);
		return available();
	}

	bool WinTcpSocket::poll() {
		unsigned long bytesToRead;
		auto errorCode = ioctlsocket(this->socket, FIONREAD, &bytesToRead);
		if(errorCode == SOCKET_ERROR) {
			close();
			return false;
		}
		else {
			return bytesToRead > 0;
		}
	}

	bool WinTcpSocket::available() {
		return socket != INVALID_SOCKET;
	}

	void WinTcpSocket::send(WSString data) {
		this->send(reinterpret_cast<uint8_t*>(const_cast<char*>(data.c_str())), data.size());
	}

	void WinTcpSocket::send(uint8_t* data, uint32_t len) {
		auto error = windowsTcpSend(data, len, this->socket);
		if(error) close();
	}

	WSString WinTcpSocket::readLine() {
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
	void WinTcpSocket::read(uint8_t* buffer, uint32_t len) {
		auto error = windowsTcpRecive(buffer, len, this->socket);
		if(error) close();
	}

	void WinTcpSocket::close() {
		if(socket != INVALID_SOCKET) {
			socket = INVALID_SOCKET;
			// shutdown the connection since no more data will be sent/received
			shutdown(socket, SD_BOTH);
			closesocket(socket);
			// TODO WSA cleanup shouldnt be called multiple times?
			WSACleanup();
		}
	}

	WinTcpSocket::~WinTcpSocket() {
		close();
	}
}} // websockets::network

#endif // #ifdef _WIN32