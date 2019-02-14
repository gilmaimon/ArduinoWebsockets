[![arduino-library-badge](https://www.ardu-badge.com/badge/ArduinoWebsockets.svg?)](https://www.ardu-badge.com/ArduinoWebsockets)

# Arduino Websockets

A library for writing modern websockets applications with Arduino (ESP8266 and ESP32). This project is based on my project [TinyWebsockets](https://github.com/gilmaimon/TinyWebsockets).

## Getting Started
This section should help you get started with the library. If you have any questions feel free to open an issue.

### Prerequisites
Currently (version 0.1.1) the library only works with `ESP8266` and `ESP32`.

### Installing

You can install the library from the Arduino IDE or using a release ZIP file from the [Github realese page](https://github.com/gilmaimon/TinyWebsockets/releases).
Detailed instructions can be found [here](https://www.ardu-badge.com/ArduinoWebsockets).

## Full Basic Example
```c++
#include <ArduinoWebsockets.h>
#include <WiFi.h>

const char* ssid = "ssid"; //Enter SSID
const char* password = "password"; //Enter Password
const char* websockets_server_host = "serverip_or_name"; //Enter server adress
const uint16_t websockets_server_port = 8080; // Enter server port

using namespace websockets;
using namespace websockets::network;

WebsocketsClient client(new Esp32TcpClient);
void setup() {
    Serial.begin(115200);
    // Connect to wifi
    WiFi.begin(ssid, password);

    // Wait some time to connect to wifi
    for(int i = 0; i < 10 && WiFi.status() != WL_CONNECTED; i++) {
        Serial.print(".");
        delay(1000);
    }

    // Check if connected to wifi
    if(WiFi.status() != WL_CONNECTED) {
        Serial.println("No Wifi!");
        return;
    }

    Serial.println("Connected to Wifi, Connection to server.");
    // try to connect to Websockets server
    bool connected = client.connect(websockets_server_host, "/", websockets_server_port);
    if(connected) {
        Serial.println("Connecetd!");
        client.send("Hello Server");
    } else {
        Serial.println("Not Connected!");
    }
    
    // run callback when messages are received
    client.onMessage([&](WebsocketsMessage message){
        Serial.print("Got Data: ");
        Serial.println(message.data().c_str());
    });
}

void loop() {
    // let the websockets client check for incoming messages
    if(client.available()) {
        client.poll();
    }
    delay(500);
}
```

***Note**: for esp32, replace:* 
```c++
new Esp8266TcpClient
``` 
*with*
```c++
new Esp32cpClient
```


## Contributing
Contributions are welcomed! Please open issues if you have troubles while using the library or any queshtions on how to get started. Pull requests are welcomed, please open an issue first.

## Change Log
- **14/02/2019 (v0.1.1)** - Initial commits and support for ESP32 and ESP8266 Websocket Clients.