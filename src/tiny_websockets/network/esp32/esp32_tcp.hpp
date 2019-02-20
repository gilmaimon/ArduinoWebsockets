#pragma once

#ifdef ESP32 

#include <tiny_websockets/internals/ws_common.hpp>
#include <tiny_websockets/network/tcp_client.hpp>
#include <Arduino.h>

#include <WiFi.h>

namespace websockets { namespace network {
	class Esp32TcpClient : public TcpClient {
	public:
		Esp32TcpClient() : _connected(false) {}

		bool connect(WSString host, int port) {
			this->_connected = client.connect(host.c_str(), port);
			return available();
		}

		bool poll() {
			return client.available();
		}

		bool available() override {
			/*
				NOTE: it seems like there is a bug in esp32 impl of connected(). the method returns false
			          even when the connection is active and can still be used. This is why a socket is assumed here 
					  to be open unless it was manually closed. or an even happend (like an unsuccssfull read/write)
					  TODO: this is not optimal, and should be fixed in some way.
			*/
			return this->_connected;
		}

		void send(WSString data) override {
			auto sent = client.write(reinterpret_cast<uint8_t*>(const_cast<char*>(data.c_str())), data.size());
			if(sent < data.size()) {
				this->_connected = false;
			}
		}

		void send(uint8_t* data, uint32_t len) override {
			auto sent = client.write(data, len);
			if(sent < len) {
				this->_connected = false;
			}
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
			auto res = client.read(buffer, len);
			if(res < 0) {
				this->_connected = false;
			}
		}

		void close() override {
			client.stop();
			_connected = false;
		}

		virtual ~Esp32TcpClient() {
			close();
		}
	private:
		WiFiClient client;
		bool _connected;
	};
}} // websockets::network

#endif // #ifdef ESP32 