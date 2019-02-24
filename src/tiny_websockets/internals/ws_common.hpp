#pragma once

#include <tiny_websockets/ws_config_defs.hpp>
#include <string>
#include <Arduino.h>

namespace websockets {
    typedef std::string WSString;
    typedef String WSInterfaceString;

    namespace internals {
        WSString fromInterfaceString(WSInterfaceString& str);
        WSString fromInterfaceString(WSInterfaceString&& str);
        WSInterfaceString fromInternalString(WSString& str);
        WSInterfaceString fromInternalString(WSString&& str);
    }
}

#ifdef _WIN32
    #include <tiny_websockets/network/windows/win_tcp_client.hpp>
    #define WSDefaultTcpClient websockets::network::WinTcpClient
#elif defined(__linux__)
    #include <tiny_websockets/network/linux/linux_tcp_client.hpp>
    #define WSDefaultTcpClient websockets::network::LinuxTcpClient
#elif defined(ESP8266)
    #include <tiny_websockets/network/esp8266/esp8266_tcp.hpp>
    #define WSDefaultTcpClient websockets::network::Esp8266TcpClient
#elif defined(ESP32)
    #include <tiny_websockets/network/esp32/esp32_tcp.hpp>
    #define WSDefaultTcpClient websockets::network::Esp32TcpClient
#endif