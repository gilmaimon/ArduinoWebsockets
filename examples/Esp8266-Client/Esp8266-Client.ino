#include <ArduinoWebsockets.h>
#include <ESP8266WiFi.h>

const char* ssid = "ssid"; //Enter SSID
const char* password = "password"; //Enter Password
const char* websockets_server_host = "serverip_or_name"; //Enter server adress
const uint16_t websockets_server_port = 8080; // Enter server port

using namespace websockets;
using namespace websockets::network;

WebsocketsClient client(new Esp8266TcpClient);
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