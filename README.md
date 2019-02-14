[![arduino-library-badge](https://www.ardu-badge.com/badge/ArduinoWebsockets.svg?)](https://www.ardu-badge.com/ArduinoWebsockets)

# Arduino Websockets

A library for writing modern websockets applications with Arduino (ESP8266 and ESP32). This project is based on my project [TinyWebsockets](https://github.com/gilmaimon/TinyWebsockets).

## Getting Started
This section should help you get started with the library. If you have any questions feel free to open an issue.

### Prerequisites
*Notice: some of the examples require `WifiConnection.h` which can be installed by installing [Arduino-CloudStorage](https://github.com/gilmaimon/Arduino-CloudStorage). You could also use your own code to connect to a WiFi network.*

### Installing

You can install the library from the Arduino IDE or using a release ZIP file from the [Github realese page](https://github.com/gilmaimon/TinyWebsockets/releases).
Detailed instructions can be found [here](https://www.ardu-badge.com/ArduinoWebsockets).

## Full Basic Example
```c++
#include <ArduinoWebsockets.h>
#include <WifiConnection.h>

using namespace websockets;
using namespace websockets::network;

WebsocketsClient client(new Esp8266TcpClient);
void setup() {
    Serial.begin(115200);
    // Connect to wifi
    WifiConnection::tryConnect("SSID", "PASSWORD");

    // Wait some time to connect to wifi
    for(int i = 0; i < 10 && !WifiConnection::isConnected(); i++) {
        Serial.print(".");
        delay(1000);
    }

    // Check if connected to wifi
    if(!WifiConnection::isConnected()) {
        Serial.println("No Wifi!");
        return;
    }

    Serial.println("Connected to Wifi, Connection to server.");
    // try to connect to Websockets server
    bool connected = client.connect("SERVER_IP", "/", 8080);
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

## Contributing
Contributions are welcomed! Please open issues if you have troubles while using the library or any queshtions on how to get started. Pull requests are welcomed, please open an issue first.