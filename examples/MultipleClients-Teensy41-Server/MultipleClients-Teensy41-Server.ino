/*
  Teensy41 Websockets Server and Http Server (using NativeEthernet).
  Combining the Teensy41-Server example with the NativeEthernet WebServer
  example (https://github.com/vjmuzik/NativeEthernet/blob/master/examples/WebServer/WebServer.ino).

  This sketch:
  1. Connects to a ethernet network
  2. Starts a websocket server on port 80
  3. Waits for connections
  4. As soon as a client wants to establish a connection, it checks whether a
     free slot is available and accepts it accordingly
  5. If the client is accepted it sends a welcome message and echoes any 
     messages from the client
  6. Goes back to step 3

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

using namespace websockets;

// We will set the MAC address at the beginning of `setup()` using TeensyID's
// `teensyMac` helper.
byte mac[6];

// Enter websockets server port.
const uint16_t port = 80;

// Define how many clients we accpet simultaneously.
const byte maxClients = 4;

WebsocketsClient clients[maxClients];
WebsocketsServer server;

void setup() {
  // Set the MAC address.
  teensyMAC(mac);

  // Connect to ethernet.
  if (Ethernet.begin(mac)) {
    Serial.println("Ethernet connected");
  } else {
    Serial.println("Ethernet failed");
  }

  // Start websockets server.
  server.listen(port);
  if (server.available()) {
    Serial.print("Server available at ws://");
    Serial.print(Ethernet.localIP());
    // Also log any non default port.
    if (port != 80) Serial.printf(":%d", port);
    Serial.println();
  } else {
    Serial.println("Server not available!");
  }  
}

void handleMessage(WebsocketsClient &client, WebsocketsMessage message) {
  auto data = message.data();

  // Log message
  Serial.print("Got Message: ");
  Serial.println(data);

  // Echo message
  client.send("Echo: " + data);
}

void handleEvent(WebsocketsClient &client, WebsocketsEvent event, String data) {
  if (event == WebsocketsEvent::ConnectionClosed) {
    Serial.println("Connection closed");
  }
}

int8_t getFreeClientIndex() {
  // If a client in our list is not available, it's connection is closed and we
  // can use it for a new client.  
  for (byte i = 0; i < maxClients; i++) {
    if (!clients[i].available()) return i;
  }
  return -1;
}

void listenForClients() {
  if (server.poll()) {
    int8_t freeIndex = getFreeClientIndex();
    if (freeIndex >= 0) {
      WebsocketsClient newClient = server.accept();
      Serial.printf("Accepted new websockets client at index %d\n", freeIndex);
      newClient.onMessage(handleMessage);
      newClient.onEvent(handleEvent);
      newClient.send("Hello from Teensy");
      clients[freeIndex] = newClient;
    }
  }
}

void pollClients() {
  for (byte i = 0; i < maxClients; i++) {
    clients[i].poll();
  }
}

void loop() {
  listenForClients();
  pollClients();
}