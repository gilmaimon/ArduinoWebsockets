#pragma once

#include <tiny_websockets/ws_config_defs.hpp>
#include <string>

namespace websockets {
    typedef std::string WSString;
}

#ifdef _WIN32
    #include <tiny_websockets/network/windows/win_tcp_client.hpp>
    #define DEFAULT_CLIENT websockets::network::WinTcpClient
#elif defined(__linux__)
    #include <tiny_websockets/network/linux/linux_tcp_client.hpp>
    #define DEFAULT_CLIENT websockets::network::LinuxTcpClient
#elif defined(ESP8266)
    #include <tiny_websockets/network/esp8266/esp8266_tcp.hpp>
    #define DEFAULT_CLIENT websockets::network::Esp8266TcpClient
#elif defined(ESP32)
    #include <tiny_websockets/network/esp32/esp32_tcp.hpp>
    #define DEFAULT_CLIENT websockets::network::Esp32TcpClient
#endif