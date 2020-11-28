/*
  Teensy41 Websockets Server and Http Server (using NativeEthernet).
  Combining the Teensy41-Server-Multiple-Clients example with the NativeEthernet
  WebServer example (https://github.com/vjmuzik/NativeEthernet/blob/master/examples/WebServer/WebServer.ino).

  This sketch:
  1. Connects to a ethernet network
  2. Starts a websocket server on port 3000
  3. Starts a http server at the default port 80
  4. Waits for both http and websockets connections
  5. Once a http client connects, it serves an html document, once a socket
     client wants to connect, it checks whether a free slot is available and
     accepts it accordingly
  5. If the socket client is accepted it sends a welcome message and echoes any
     messages from the client
  6. Goes back to step 4

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
const uint16_t websocketsPort = 3000;

// Define how many clients we accpet simultaneously.
const byte maxSocketClients = 4;

WebsocketsClient socketClients[maxSocketClients];
WebsocketsServer socketServer;
EthernetServer httpServer;

void setup() {
  // Set the MAC address.
  teensyMAC(mac);

  // Start Serial and wait until it is ready.
  Serial.begin(9600);
  while (!Serial) {}

  // Connect to ethernet.
  if (Ethernet.begin(mac)) {
    Serial.println("Ethernet connected");
  } else {
    Serial.println("Ethernet failed");
  }

  // Start websockets server.
  socketServer.listen(websocketsPort);
  if (!socketServer.available()) {
    Serial.println("Websockets Server not available!");
  }

  // Start http server.
  httpServer.begin(80);
  Serial.print("Visit http://");
  Serial.print(Ethernet.localIP());
  Serial.println(" in the browser to connect.");
}

int8_t getFreeSocketClientIndex() {
  // If a client in our list is not available, it's connection is closed and we
  // can use it for a new client.  
  for (byte i = 0; i < maxSocketClients; i++) {
    if (!socketClients[i].available()) return i;
  }
  return -1;
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

void listenForSocketClients() {
  if (socketServer.poll()) {
    int8_t freeIndex = getFreeSocketClientIndex();
    if (freeIndex >= 0) {
      WebsocketsClient newClient = socketServer.accept();
      Serial.printf("Accepted new websockets client at index %d\n", freeIndex);
      newClient.onMessage(handleMessage);
      newClient.onEvent(handleEvent);
      newClient.send("Hello from Teensy");
      socketClients[freeIndex] = newClient;
    }
  }
}

void pollSocketClients() {
  for (byte i = 0; i < maxSocketClients; i++) {
    socketClients[i].poll();
  }
}

void sendHttpReply(EthernetClient &client) {
  // Send a website that connects to the websocket server and allows to
  // communicate with the teensy.

  const char* header = 
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: text/html\r\n"
    "Connection: close\r\n"
    "\r\n";

  const char* document = 
    "<!DOCTYPE html>\n"
    "<title>Teensy 4.1 Websockets</title>\n"
    "<meta charset='UTF-8'>\n"
    "<style>\n"
    "  body {\n"
    "    display: grid;\n"
    "    grid-template: min-content auto / auto min-content;\n"
    "    grid-gap: 1em;\n"
    "    margin: 0;\n"
    "    padding: 1em;\n"
    "    height: 100vh;\n"
    "    box-sizing: border-box;\n"
    "  }\n"
    "  #output {\n"
    "    grid-column-start: span 2;\n"
    "    overflow-y: scroll;\n"
    "    padding: 0.1em;\n"
    "    border: 1px solid;\n"
    "    font-family: monospace;\n"
    "  }\n"
    "</style>\n"
    "<input type='text' id='message' placeholder='Send a message and Teensy will echo it back!'>\n"
    "<button id='send-message'>send</button>\n"
    "<div id='output'></div>\n"
    "<script>\n"
    "  const url = `ws://${window.location.host}:3000`\n"
    "  const ws = new WebSocket(url)\n"
    "  let connected = false\n"
    "  const sendMessage = document.querySelector('#send-message')\n"
    "  const message = document.querySelector('#message')\n"
    "  const output = document.querySelector('#output')\n"
    "  function log(message, color = 'black') {\n"
    "    const el = document.createElement('div')\n"
    "    el.innerHTML = message\n"
    "    el.style.color = color\n"
    "    output.append(el)\n"
    "    output.scrollTop = output.scrollHeight\n"
    "  }\n"
    "  ws.addEventListener('open', () => {\n"
    "    connected = true\n"
    "    log('(✔️) Open', 'green')\n"
    "  })\n"
    "  ws.addEventListener('close', () => {\n"
    "    connected = false\n"
    "    log('(❌) Close', 'red')\n"
    "  })\n"
    "  ws.addEventListener('message', ({ data }) =>\n"
    "    log(`(💌) ${data}`)\n"
    "  )\n"
    "  sendMessage.addEventListener('click', () => {\n"
    "    connected && ws.send(message.value)\n"
    "  })\n"
    "  message.addEventListener('keyup', ({ keyCode }) => {\n"
    "     connected && keyCode === 13 && ws.send(message.value)\n"
    "  })\n"
    "  log(`(📡) Connecting to ${url} ...`, 'blue')\n"
    "</script>\n";

  client.write(header);
  client.write(document);  
}

void listenForHttpClients() {
  // Listen for incoming http clients.
  EthernetClient client = httpServer.available();

  if (client) {
    Serial.println("Http client connected!");

    // An http request ends with a blank line.
    bool currentLineIsBlank = true;

    while (client.connected()) {
      if (client.available()) {
        char c = client.read();

        if (c == '\n' && currentLineIsBlank) {
          // If we've gotten to the end of the line (received a newline
          // character) and the line is blank, the http request has ended,
          // so we can send a reply.
          sendHttpReply(client);
          break;
        } else if (c == '\n') {
          // Starting a new line.
          currentLineIsBlank = true;
        } else if (c != '\r') {
          // Read a character on the current line.
          currentLineIsBlank = false;
        }
      }
    }

    // The NativeEthernet's WebServer example adds a small delay here. For me it
    // seems to work without the delay. Uncomment to following line if you have
    // issues connecting to the website in the browser.
    // delay(1);

    // Close the connection.
    client.stop();
  }
}

void loop() {
  listenForSocketClients();
  pollSocketClients();
  listenForHttpClients();
}