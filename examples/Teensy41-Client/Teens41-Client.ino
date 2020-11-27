/*
  Teensy41 Websockets Client (using NativeEthernet)

  This sketch:
  1. Connects to a ethernet network
  2. Connects to a websockets server at port 80
  3. Sends the websockets server a message ("Hello Server")
  4. Prints all incoming messages while the connection is open

  Note:
  Make sure you share your computer's internet connection with the Teensy
  via ethernet.

  Libraries:
  To use this sketch install
  * TeensyID library (https://github.com/sstaub/TeensyID)
  * NativeEthernet (https://github.com/vjmuzik/NativeEthernet)

  Hardware:
  For this sketch you need a Teensy 4.1 board and the Teensy 4.1 Ethernet Kit
  (https://www.pjrc.com/store/ethernet_kit.html).
*/

#include <NativeEthernet.h>
#include <ArduinoWebsockets.h>
#include <TeensyID.h>
#include <SPI.h>

using namespace websockets;
WebsocketsClient client;

// We will set the MAC address at the beginning of `setup()` using TeensyID's
// `teensyMac` helper.
byte mac[6];

// Enter websockets url.
// Note: wss:// currently not working.
const char* url  = "ws://echo.websocket.org";

void setup() {
  // Set the MAC address.
  teensyMAC(mac);

  // Start Serial and wait until it is ready.
  Serial.begin(9600);
  while (!Serial) {}

  // Connect to ethernet.
  if (Ethernet.begin(mac)) {
    Serial.print("Ethernet connected (");
    Serial.print(Ethernet.localIP());
    Serial.println(")");
  } else {
    Serial.println("Ethernet failed");
  }

  // Connect to websocket server.
  if (client.connect(url)) {
    Serial.printf("Connected to server %s\n", url);
    // Send welcome message.
    client.send("Hello Server");
  } else {
    Serial.println("Couldn't connect to server!");
  }

  // Run callback when messages are received.
  client.onMessage([&](WebsocketsMessage message) {
    Serial.print("Got Message: ");
    Serial.println(message.data());
  });	
}

void loop() {
  // Check for incoming messages.
  if (client.available()) {
    client.poll();
  }
}