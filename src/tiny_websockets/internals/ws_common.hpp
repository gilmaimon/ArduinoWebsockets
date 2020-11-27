#pragma once

#include <tiny_websockets/ws_config_defs.hpp>
#include <string>
#include <Arduino.h>

namespace websockets {
    typedef std::string WSString;
    typedef String WSInterfaceString;

    namespace internals {
        WSString fromInterfaceString(const WSInterfaceString& str);
        WSString fromInterfaceString(const WSInterfaceString&& str);
        WSInterfaceString fromInternalString(const WSString& str);
        WSInterfaceString fromInternalString(const WSString&& str);
    }
}

#ifdef ESP8266
    #define PLATFORM_DOES_NOT_SUPPORT_BLOCKING_READ

    #include <tiny_websockets/network/esp8266/esp8266_tcp.hpp>
    #define WSDefaultTcpClient websockets::network::Esp8266TcpClient
    #define WSDefaultTcpServer websockets::network::Esp8266TcpServer

    #ifndef _WS_CONFIG_NO_SSL
        // OpenSSL Dependent
        #define WSDefaultSecuredTcpClient websockets::network::SecuredEsp8266TcpClient
    #endif //_WS_CONFIG_NO_SSL

#elif defined(ESP32)

    #define PLATFORM_DOES_NOT_SUPPORT_BLOCKING_READ

    #include <tiny_websockets/network/esp32/esp32_tcp.hpp>
    #define WSDefaultTcpClient websockets::network::Esp32TcpClient
    #define WSDefaultTcpServer websockets::network::Esp32TcpServer

    #ifndef _WS_CONFIG_NO_SSL
        // OpenSSL Dependent
        #define WSDefaultSecuredTcpClient websockets::network::SecuredEsp32TcpClient
    #endif //_WS_CONFIG_NO_SSL

#elif defined(ARDUINO_TEENSY41)
    #define PLATFORM_DOES_NOT_SUPPORT_BLOCKING_READ
    #define _WS_CONFIG_NO_SSL

    #include <tiny_websockets/network/teensy41/teensy41_tcp_client.hpp>
    #include <tiny_websockets/network/teensy41/teensy41_tcp_server.hpp>

    #define WSDefaultTcpClient websockets::network::Teensy41TcpClient
    #define WSDefaultTcpServer websockets::network::Teensy41TcpServer    
#endif
