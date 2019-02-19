#pragma once

#ifdef _WIN32 

#include <tiny_websockets/internals/ws_common.hpp>
#include <tiny_websockets/network/tcp_client.hpp>
#include <tiny_websockets/network/windows/win_tcp_socket.hpp>

namespace websockets { namespace network {
	class WinTcpClient : public TcpClient {
	public:
		bool connect(WSString host, int port) {
			return socket.connect(host, port);
		}

		bool poll() {
			return socket.poll();
		}

		bool available() override {
			return socket.available();
		}

		void send(WSString data) override {
			socket.send(data);
		}

		void send(uint8_t* data, uint32_t len) override {
			socket.send(data, len);
		}
		
		WSString readLine() override {
			return socket.readLine();
		}

		void read(uint8_t* buffer, uint32_t len) override {
			socket.read(buffer, len);
		}

		void close() override {
			socket.close();
		}

		virtual ~WinTcpClient() {}
	private:
		WinTcpSocket socket;
	};
}} // websockets::network

#endif // #ifdef _WIN32 