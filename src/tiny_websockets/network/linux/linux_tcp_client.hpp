#pragma once

#ifdef __linux__ 

#include <tiny_websockets/internals/ws_common.hpp>
#include <tiny_websockets/network/tcp_client.hpp>
#include <tiny_websockets/network/tcp_socket.hpp>

#define INVALID_SOCKET -1

class LinuxTcpSocket;

namespace websockets { namespace network {
	class LinuxTcpSocket : public TcpSocket {
    public:
        LinuxTcpSocket();
        bool connect(WSString host, int port);
        bool poll() override;
        bool available() override;
        void send(WSString data) override;
        void send(uint8_t* data, uint32_t len) override;
        WSString readLine() override;
        void read(uint8_t* buffer, uint32_t len) override;
        void close() override;
        virtual ~LinuxTcpSocket();

    private:
        int _socket;
    };

	class LinuxTcpClient : public TcpClient {
	public:
		bool connect(WSString host, int port) {
			return _socket.connect(host, port);
		}

		bool poll() {
			return _socket.poll();
		}

		bool available() override {
			return _socket.available();
		}

		void send(WSString data) override {
			_socket.send(data);
		}

		void send(uint8_t* data, uint32_t len) override {
			_socket.send(data, len);
		}
		
		WSString readLine() override {
			return _socket.readLine();
		}

		void read(uint8_t* buffer, uint32_t len) override {
			_socket.read(buffer, len);
		}

		void close() override {
			_socket.close();
		}

		virtual ~LinuxTcpClient() {}
	private:
		LinuxTcpSocket _socket;
	};
}} // websockets::network

#endif // #ifdef __linux__ 