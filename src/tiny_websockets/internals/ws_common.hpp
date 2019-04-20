#pragma once

#include <tiny_websockets/ws_config_defs.hpp>
#include <string>
#include <Arduino.h>

// Versioning
#define TINY_WS_VERSION_STRING "0.1.0"
#define TINY_WS_VERSION_MAJOR 0
#define TINY_WS_VERSION_MINOR 1
#define TINY_WS_VERSION_PATCH 0

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

#ifdef _WIN32
    #include <tiny_websockets/network/windows/win_tcp_client.hpp>
    #include <tiny_websockets/network/windows/win_tcp_server.hpp>
    #define WSDefaultTcpClient websockets::network::WinTcpClient
    #define WSDefaultTcpServer websockets::network::WinTcpServer

    #ifndef _WS_CONFIG_NO_SSL
        // OpenSSL Dependent
        #include <tiny_websockets/network/openssl_secure_tcp_client.hpp>
        #define WSDefaultSecuredTcpClient websockets::network::OpenSSLSecureTcpClient<WSDefaultTcpClient>
    #endif //_WS_CONFIG_NO_SSL

#elif defined(__linux__)
    #include <tiny_websockets/network/linux/linux_tcp_client.hpp>
    #include <tiny_websockets/network/linux/linux_tcp_server.hpp>
    #define WSDefaultTcpClient websockets::network::LinuxTcpClient
    #define WSDefaultTcpServer websockets::network::LinuxTcpServer

    #ifndef _WS_CONFIG_NO_SSL
        // OpenSSL Dependent
        #include <tiny_websockets/network/openssl_secure_tcp_client.hpp>
        #define WSDefaultSecuredTcpClient websockets::network::OpenSSLSecureTcpClient<WSDefaultTcpClient>
    #endif //_WS_CONFIG_NO_SSL

#elif defined(ESP8266)

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
#endif