#pragma once

#ifdef ESP8266 

#include "ws_common.h"
#include "network/tcp_client.h"

#include <ESP8266WiFi.h>

namespace websockets { namespace network {
	class Esp8266TcpClient : public TcpClient {
	public:
		bool connect(WSString host, int port) {
			return client.connect(host.c_str(), port);
		}

		bool poll() {
			return client.peek();
		}

		bool available() override {
			return client.connected();
		}

		void send(WSString data) override {
			client.write(reinterpret_cast<uint8_t*>(const_cast<char*>(data.c_str())), data.size());
		}

		void send(uint8_t* data, uint32_t len) override {
			client.write(data, len);
		}
		
		WSString readLine() override {
			uint8_t byte = '\0';
			WSString line;
			while (poll()) {
				read(&byte, 1);
				if(byte == '\0') continue;
				line += (char)byte;
				if (byte == '\n') break;
			}
			if(!available()) close();
			return line;
		}

		void read(uint8_t* buffer, uint32_t len) override {
			client.read(buffer, len);
		}

		void close() override {
			client.stop();
		}

		virtual ~Esp8266TcpClient() {
			client.stop();
		}
	private:
		WiFiClient client;
	};
}} // websockets::network

#endif // #ifdef ESP8266 