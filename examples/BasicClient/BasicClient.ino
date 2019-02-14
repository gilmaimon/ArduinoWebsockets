#include "ArduinoWebsockets.h"

websockets::WebsocketsClient client(nullptr);
void setup() {
    Serial.begin(115200);

    client.connect("localhost", "/", 8080);
    client.onMessage([&](websockets::WebsocketsMessage message){
		Serial.print("Got Data: ");
        Serial.println(message.data().c_str());
	});
}

void loop() {
    if(client.available()) {
        client.poll();
    }
}