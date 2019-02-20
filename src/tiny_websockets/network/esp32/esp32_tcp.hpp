#pragma once

#ifdef ESP32 

#include <tiny_websockets/internals/ws_common.hpp>
#include <tiny_websockets/network/tcp_client.hpp>
#include <Arduino.h>

#include <WiFi.h>

namespace websockets { namespace network {
	class Esp32TcpClient : public TcpClient {
	public:
		bool connect(WSString host, int port) : closed(false) {
			return client.connect(host.c_str(), port);
		}

		bool poll() {
			return client.available();
		}

		bool available() override {
			return !closed;
			/*
				NOTE: it seems like there is a bug in esp32 impl of connected(). the method returns false
			          even when the connection is active and can still be used. This is why a socket is assumed here 
					  to be open unless it was manually closed.
					  TODO: this is bad, and should be fixed in some way.
			*/
			//return client.connected();
		}

		void send(WSString data) override {
			client.write(reinterpret_cast<uint8_t*>(const_cast<char*>(data.c_str())), data.size());
		}

		void send(uint8_t* data, uint32_t len) override {
			client.write(data, len);
		}
		
		WSString readLine() override {
			int val;
			WSString line;
			do {
				val = client.read();
				if(val < 0) continue;
				line += (char)val;
			} while(val != '\n');
			if(!available()) close();
			return line;
		}

		void read(uint8_t* buffer, uint32_t len) override {
			client.read(buffer, len);
		}

		void close() override {
			closed = true;
			client.stop();
		}

		virtual ~Esp32TcpClient() {
			closed = true;
			client.stop();
		}
	private:
		WiFiClient client;
		bool closed = false;
	};
}} // websockets::network

#endif // #ifdef ESP32 