#pragma once

#define _WS_CONFIG_NO_TRUE_RANDOMNESS
#define _WS_BUFFER_SIZE 512
#define _CONNECTION_TIMEOUT 1000

#if !defined(WS_HEADERS_NORMAL_CASE)
  #define WS_HEADERS_NORMAL_CASE     true
#endif

#if (_WEBSOCKETS_LOGLEVEL_ > 3)
  #if WS_HEADERS_NORMAL_CASE
    #warning Not using WS Header lowercase
  #else
    #warning Using WS Header lowercase
  #endif
#endif

  // Common
  #define HEADER_HOST_RN                      "\r\n"
  #define WS_KEY_LOWER_CASE                   "sec-websocket-key"

  #define WS_VERSION_NORMAL                   "Sec-WebSocket-Version"
  #define WS_VERSION_LOWER_CASE               "sec-websocket-version"

  #define WS_KEY_NORMAL                       "Sec-WebSocket-Key"
  #define WS_KEY_LOWER_CASE                   "sec-websocket-key"
  
  #define WS_ACCEPT_NORMAL                    "Sec-WebSocket-Accept"
  
  /////////////////////////////////////////////////////

  // Not using all lowercase headers
  #define HEADER_HOST_NORMAL                      "Host: "
  #define HEADER_CONNECTION_NORMAL                "Connection"
  #define HEADER_CONNECTION_UPGRADE_NORMAL        "Connection: Upgrade\r\n"
  #define HEADER_UPGRADE_NORMAL                   "Upgrade"
  #define HEADER_UPGRADE_WS_NORMAL                "Upgrade: websocket\r\n"
  #define HEADER_WEBSOCKET_NORMAL                 "websocket"
  #define HEADER_WS_VERSION_NORMAL                "Sec-WebSocket-Version: "
  #define HEADER_WS_VERSION_13_NORMAL             "Sec-WebSocket-Version: 13\r\n"
  #define HEADER_WS_KEY_NORMAL                    "Sec-WebSocket-Key: "
  #define HEADER_USER_AGENT_NORMAL                "User-Agent"
  #define HEADER_USER_AGENT_VALUE_NORMAL          "User-Agent: TinyWebsockets Client\r\n"
  #define HEADER_AUTH_BASIC_NORMAL                "Authorization: Basic "
  #define HEADER_ORIGIN_NORMAL                    "Origin"
  #define HEADER_ORIGIN_VALUE_NORMAL              "Origin: https://github.com/khoih-prog/Websockets2_Generic\r\n"
  
  #define HEADER_WS_ACCEPT_NORMAL                 "Sec-WebSocket-Accept: "
  
  /////////////////////////////////////////////////////
  
  // Lowercase headers
  #define HEADER_HOST_LOWER_CASE                  "host: "
  #define HEADER_CONNECTION_LOWER_CASE            "connection"
  #define HEADER_CONNECTION_UPGRADE_LOWER_CASE    "connection: upgrade\r\n"
  #define HEADER_UPGRADE_LOWER_CASE               "upgrade"
  #define HEADER_UPGRADE_WS_LOWER_CASE            "upgrade: websocket\r\n"
  #define HEADER_WEBSOCKET_LOWER_CASE             "websocket"
  #define HEADER_WS_VERSION_LOWER_CASE            "sec-webSocket-version: "
  #define HEADER_WS_VERSION_13_LOWER_CASE         "sec-webSocket-version: 13\r\n"
  #define HEADER_WS_KEY_LOWER_CASE                "sec-webSocket-key: "
  #define HEADER_USER_AGENT_LOWER_CASE            "user-agent"
  #define HEADER_USER_AGENT_VALUE_LOWER_CASE      "user-agent: TinyWebsockets Client\r\n"
  #define HEADER_AUTH_BASIC_LOWER_CASE            "authorization: basic "
  #define HEADER_ORIGIN_LOWER_CASE                "origin"
  #define HEADER_ORIGIN_VALUE_LOWER_CASE          "origin: https://github.com/khoih-prog/Websockets2_Generic\r\n"
  
  /////////////////////////////////////////////////////
  
#if WS_HEADERS_NORMAL_CASE
  // Not using all lowercase headers
  #define HEADER_HOST                             HEADER_HOST_NORMAL
  #define HEADER_CONNECTION                       HEADER_CONNECTION_NORMAL
  #define HEADER_CONNECTION_UPGRADE               HEADER_CONNECTION_UPGRADE_NORMAL
  #define HEADER_UPGRADE                          HEADER_UPGRADE_NORMAL
  #define HEADER_UPGRADE_WS                       HEADER_UPGRADE_WS_NORMAL
  #define HEADER_WEBSOCKET                        HEADER_WEBSOCKET_NORMAL
  #define HEADER_WS_VERSION                       HEADER_WS_VERSION_NORMAL
  #define HEADER_WS_VERSION_13                    HEADER_WS_VERSION_13_NORMAL
  #define HEADER_WS_KEY                           HEADER_WS_KEY_NORMAL
  #define HEADER_USER_AGENT                       HEADER_USER_AGENT_NORMAL
  #define HEADER_USER_AGENT_VALUE                 HEADER_USER_AGENT_VALUE_NORMAL
  #define HEADER_AUTH_BASIC                       HEADER_AUTH_BASIC_NORMAL
  #define HEADER_ORIGIN                           HEADER_ORIGIN_NORMAL
  #define HEADER_ORIGIN_VALUE                     HEADER_ORIGIN_VALUE_NORMAL
  
#else
  // Lowercase headers
  #define HEADER_HOST                             HEADER_HOST_LOWER_CASE
  #define HEADER_CONNECTION                       HEADER_CONNECTION_LOWER_CASE
  #define HEADER_CONNECTION_UPGRADE               HEADER_CONNECTION_UPGRADE_LOWER_CASE
  #define HEADER_UPGRADE                          HEADER_UPGRADE_LOWER_CASE
  #define HEADER_UPGRADE_WS                       HEADER_UPGRADE_WS_LOWER_CASE
  #define HEADER_WEBSOCKET                        HEADER_WEBSOCKET_LOWER_CASE
  #define HEADER_WS_VERSION                       HEADER_WS_VERSION_LOWER_CASE
  #define HEADER_WS_VERSION_13                    HEADER_WS_VERSION_13_LOWER_CASE
  #define HEADER_WS_KEY                           HEADER_WS_KEY_LOWER_CASE
  #define HEADER_USER_AGENT                       HEADER_USER_AGENT_LOWER_CASE
  #define HEADER_USER_AGENT_VALUE                 HEADER_USER_AGENT_VALUE_LOWER_CASE
  #define HEADER_AUTH_BASIC                       HEADER_AUTH_BASIC_LOWER_CASE
  #define HEADER_ORIGIN                           HEADER_ORIGIN_LOWER_CASE
  #define HEADER_ORIGIN_VALUE                     HEADER_ORIGIN_VALUE_LOWER_CASE
  
#endif
