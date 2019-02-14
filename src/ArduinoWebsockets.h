#ifndef _WEBSOCKETS_CLIENT_H
#define _WEBSOCKETS_CLIENT_H

#include "websockets/websockets_client.h"

#ifdef ESP8266
#include "network/esp8266/esp8266_tcp.h"
#endif
#ifdef ESP32
#include "network/esp32/esp32_tcp.h"
#endif


#endif //_WEBSOCKETS_CLIENT_H