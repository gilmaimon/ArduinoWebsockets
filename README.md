[![arduino-library-badge](https://www.ardu-badge.com/badge/ArduinoWebsockets.svg?)](https://www.ardu-badge.com/ArduinoWebsockets)  [![Build Status](https://travis-ci.org/gilmaimon/ArduinoWebsockets.svg?branch=master)](https://travis-ci.org/gilmaimon/ArduinoWebsockets)

# Arduino Websockets

A library for writing modern websockets applications with Arduino (ESP8266 and ESP32). This project is based on my project [TinyWebsockets](https://github.com/gilmaimon/TinyWebsockets).

The library provides simple and easy interface for websockets work (Client and Server). See the [basic-usage](#Basic-Usage) guide and the [examples](#Full-Examples).

### Please check out the [TinyWebsockets Wiki](https://github.com/gilmaimon/TinyWebsockets/wiki) for many more details!

## Getting Started
This section should help you get started with the library. If you have any questions feel free to open an issue.

### Prerequisites
Currently (version 0.5.*) the library only works with `ESP8266`, `ESP32` and `Teensy 4.1`.

### Installing

You can install the library from the Arduino IDE or using a release ZIP file from the [Github release page](https://github.com/gilmaimon/ArduinoWebsockets/releases).
Detailed instructions can be found [here](https://www.ardu-badge.com/ArduinoWebsockets).

## Basic Usage

### Client

Creating a client and connecting to a server:
```c++
WebsocketsClient client;
client.connect("ws://your-server-ip:port/uri");
```

Sending a message:
```c++
client.send("Hello Server!");
```

Waiting for messages:
```c++
client.onMessage([](WebsocketsMessage msg){
    Serial.println("Got Message: " + msg.data());
});
```

In order to keep receiving messages, you should:
```c++
void loop() {
    client.poll();
}
```
### Server

Creating a server and listening for connections:
```c++
WebsocketsServer server;
server.listen(8080);
```

Accepting connections:
```c++
WebsocketsClient client = server.accept();
// handle client as described before :)
```

## Full Examples

### Client

```c++
#include <ArduinoWebsockets.h>
#include <ESP8266WiFi.h>

const char* ssid = "ssid"; //Enter SSID
const char* password = "password"; //Enter Password
const char* websockets_server = "www.myserver.com:8080"; //server adress and port

using namespace websockets;

void onMessageCallback(WebsocketsMessage message) {
    Serial.print("Got Message: ");
    Serial.println(message.data());
}

void onEventsCallback(WebsocketsEvent event, String data) {
    if(event == WebsocketsEvent::ConnectionOpened) {
        Serial.println("Connnection Opened");
    } else if(event == WebsocketsEvent::ConnectionClosed) {
        Serial.println("Connnection Closed");
    } else if(event == WebsocketsEvent::GotPing) {
        Serial.println("Got a Ping!");
    } else if(event == WebsocketsEvent::GotPong) {
        Serial.println("Got a Pong!");
    }
}

WebsocketsClient client;
void setup() {
    Serial.begin(115200);
    // Connect to wifi
    WiFi.begin(ssid, password);

    // Wait some time to connect to wifi
    for(int i = 0; i < 10 && WiFi.status() != WL_CONNECTED; i++) {
        Serial.print(".");
        delay(1000);
    }

    // Setup Callbacks
    client.onMessage(onMessageCallback);
    client.onEvent(onEventsCallback);
    
    // Connect to server
    client.connect(websockets_server);

    // Send a message
    client.send("Hi Server!");
    // Send a ping
    client.ping();
}

void loop() {
    client.poll();
}
```
***Note:** for ESP32 you only need to change to code that connects to WiFi (replace `#include <ESP8266WiFi.h>` with `#include <WiFi.h>`), everything else stays the same.*

### Server
```c++
#include <ArduinoWebsockets.h>
#include <ESP8266WiFi.h>

const char* ssid = "ssid"; //Enter SSID
const char* password = "password"; //Enter Password

using namespace websockets;

WebsocketsServer server;
void setup() {
  Serial.begin(115200);
  // Connect to wifi
  WiFi.begin(ssid, password);

  // Wait some time to connect to wifi
  for(int i = 0; i < 15 && WiFi.status() != WL_CONNECTED; i++) {
      Serial.print(".");
      delay(1000);
  }
  
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());   //You can get IP address assigned to ESP

  server.listen(80);
  Serial.print("Is server live? ");
  Serial.println(server.available());
}

void loop() {
  auto client = server.accept();
  if(client.available()) {
    auto msg = client.readBlocking();

    // log
    Serial.print("Got Message: ");
    Serial.println(msg.data());

    // return echo
    client.send("Echo: " + msg.data());

    // close the connection
    client.close();
  }
  
  delay(1000);
}
```
***Note:** for ESP32 you only need to change to code that connects to WiFi (replace `#include <ESP8266WiFi.h>` with `#include <WiFi.h>`), everything else stays the same.*

## Binary Data

For binary data it is recommended to use `msg.rawData()` which returns a `std::string`, or `msg.c_str()` which returns a `const char*`. 
The reason is that `msg.data()` returns an Arduino `String`, which is great for Serial printing and very basic memory handling but bad for most binary usages.

See [issue #32](https://github.com/gilmaimon/ArduinoWebsockets/issues/32) for further information.

## SSL and WSS Support

No matter what board you are using, in order to use WSS (websockets over SSL) you need to use
```c++
client.connect("wss://your-secured-server-ip:port/uri");
```

The next sections describe board-specific code for using WSS with the library.

### ESP8266
With the esp8266 there are multiple ways for using WSS. By default, `ArduinoWebsockets` does not validate the certificate chain. This can be set explicitly using:
```c++
client.setInsecure();
```

You can also use a `SSL Fingerprint` to validate the SSL connection, for example:
```c++
const char ssl_fingerprint[] PROGMEM = "D5 07 4D 79 B2 D2 53 D7 74 E6 1B 46 C5 86 4E FE AD 00 F1 98";

client.setFingerprint(ssl_fingerprint);
```

or you could use the `setKnownKey()` method to specify the public key of a certificate in order to validate the server you are connecting to.
```
PublicKey *publicKey = new PublicKey(public_key);
client.setKnownKey(publicKey);
```
or you can specify the Certificate Authority (CA) using `setTrustAnchors` method, as follows:

```
X509List *serverTrustedCA = new X509List(ca_cert);
client.setTrustAnchors(serverTrustedCA);
```

For client-side certificate validation, you can use RSA or EC certificates, using the method `setClientRSACert` or `setClientECCert` .

### ESP32
With the esp32 you could either provide the full certificate, or provide no certificate. An example for setting CA Certificate:
```c++
const char ssl_ca_cert[] PROGMEM = \
    "-----BEGIN CERTIFICATE-----\n" \
    "MIIEkjCCA3qgAwIBAgIQCgFBQgAAAVOFc2oLheynCDANBgkqhkiG9w0BAQsFADA/\n" \
    "MSQwIgYDVQQKExtEaWdpdGFsIFNpZ25hdHVyZSBUcnVzdCBDby4xFzAVBgNVBAMT\n" \
    "DkRTVCBSb290IENBIFgzMB4XDTE2MDMxNzE2NDA0NloXDTIxMDMxNzE2NDA0Nlow\n" \
    "SjELMAkGA1UEBhMCVVMxFjAUBgNVBAoTDUxldCdzIEVuY3J5cHQxIzAhBgNVBAMT\n" \
    "GkxldCdzIEVuY3J5cHQgQXV0aG9yaXR5IFgzMIIBIjANBgkqhkiG9w0BAQEFAAOC\n" \
    "AQ8AMIIBCgKCAQEAnNMM8FrlLke3cl03g7NoYzDq1zUmGSXhvb418XCSL7e4S0EF\n" \
    "q6meNQhY7LEqxGiHC6PjdeTm86dicbp5gWAf15Gan/PQeGdxyGkOlZHP/uaZ6WA8\n" \
    "SMx+yk13EiSdRxta67nsHjcAHJyse6cF6s5K671B5TaYucv9bTyWaN8jKkKQDIZ0\n" \
    "Z8h/pZq4UmEUEz9l6YKHy9v6Dlb2honzhT+Xhq+w3Brvaw2VFn3EK6BlspkENnWA\n" \
    "a6xK8xuQSXgvopZPKiAlKQTGdMDQMc2PMTiVFrqoM7hD8bEfwzB/onkxEz0tNvjj\n" \
    "/PIzark5McWvxI0NHWQWM6r6hCm21AvA2H3DkwIDAQABo4IBfTCCAXkwEgYDVR0T\n" \
    "AQH/BAgwBgEB/wIBADAOBgNVHQ8BAf8EBAMCAYYwfwYIKwYBBQUHAQEEczBxMDIG\n" \
    "CCsGAQUFBzABhiZodHRwOi8vaXNyZy50cnVzdGlkLm9jc3AuaWRlbnRydXN0LmNv\n" \
    "bTA7BggrBgEFBQcwAoYvaHR0cDovL2FwcHMuaWRlbnRydXN0LmNvbS9yb290cy9k\n" \
    "c3Ryb290Y2F4My5wN2MwHwYDVR0jBBgwFoAUxKexpHsscfrb4UuQdf/EFWCFiRAw\n" \
    "VAYDVR0gBE0wSzAIBgZngQwBAgEwPwYLKwYBBAGC3xMBAQEwMDAuBggrBgEFBQcC\n" \
    "ARYiaHR0cDovL2Nwcy5yb290LXgxLmxldHNlbmNyeXB0Lm9yZzA8BgNVHR8ENTAz\n" \
    "MDGgL6AthitodHRwOi8vY3JsLmlkZW50cnVzdC5jb20vRFNUUk9PVENBWDNDUkwu\n" \
    "Y3JsMB0GA1UdDgQWBBSoSmpjBH3duubRObemRWXv86jsoTANBgkqhkiG9w0BAQsF\n" \
    "AAOCAQEA3TPXEfNjWDjdGBX7CVW+dla5cEilaUcne8IkCJLxWh9KEik3JHRRHGJo\n" \
    "uM2VcGfl96S8TihRzZvoroed6ti6WqEBmtzw3Wodatg+VyOeph4EYpr/1wXKtx8/\n" \
    "wApIvJSwtmVi4MFU5aMqrSDE6ea73Mj2tcMyo5jMd6jmeWUHK8so/joWUoHOUgwu\n" \
    "X4Po1QYz+3dszkDqMp4fklxBwXRsW10KXzPMTZ+sOPAveyxindmjkW8lGy+QsRlG\n" \
    "PfZ+G6Z6h7mjem0Y+iWlkYcV4PIWL1iwBi8saCbGS5jN2p8M+X+Q7UNKEkROb3N6\n" \
    "KOqkqm57TH2H3eDJAkSnh6/DNFu0Qg==\n" \
    "-----END CERTIFICATE-----\n";

client.setCACert(ssl_ca_cert);
```

### TEENSY 4.1
Currently WSS is not implemented.

## Contributing
Contributions are welcomed! Please open issues if you have troubles while using the library or any queshtions on how to get started. Pull requests are welcomed, please open an issue first.

## Contributors
Thanks for everyone who reported a bug, suggested a feature and contributed to the development of this library.
<table>
  <tr>
    <td align="center"><a href="https://github.com/arnoson"><img src="https://github.com/arnoson.png" width="100px;" alt="arnoson"/><br /><sub><b>⭐️ arnoson</b></sub></a><br /></td>
    <td align="center"><a href="https://github.com/ramdor"><img src="https://github.com/ramdor.png" width="100px;" alt="ramdor"/><br /><sub><b>⭐️ ramdor</b></sub></a><br /></td>
    <td align="center"><a href="https://github.com/xgarb"><img src="https://github.com/xgarb.png" width="100px;" alt="xgarb"/><br /><sub><b>⭐️ xgarb</b></sub></a><br /></td>
    <td align="center"><a href="https://github.com/matsujirushi"><img src="https://github.com/matsujirushi.png" width="100px;" alt="matsujirushi"/><br /><sub><b>matsujirushi</b></sub></a><br /></td>
    <td align="center"><a href="https://github.com/bastienvans"><img src="https://github.com/bastienvans.png" width="100px;" alt="bastienvans"/><br /><sub><b>bastienvans</b></sub></a><br /></td>
    <td align="center"><a href="https://github.com/johneakin"><img src="https://github.com/johneakin.png" width="100px;" alt="johneakin"/><br /><sub><b>johneakin</b></sub></a><br /></td>
    <td align="center"><a href="https://github.com/lalten"><img src="https://github.com/lalten.png" width="100px;" alt="lalten"/><br /><sub><b>lalten</b></sub></a><br /></td>
  </tr>
  <tr>
      <td align="center"><a href="https://github.com/adelin-mcbsoft"><img src="https://github.com/adelin-mcbsoft.png" width="100px;" alt="adelin-mcbsoft"/><br /><sub><b>⭐️ adelin-mcbsoft</b></sub></a><br /></td>
      <td align="center"><a href="https://github.com/Jonty"><img src="https://github.com/Jonty.png" width="100px;" alt="Jonty"/><br /><sub><b>⭐️ Jonty</b></sub></a><br /></td>
      <td align="center"><a href="https://github.com/Nufflee"><img src="https://github.com/Nufflee.png" width="100px;" alt="Nufflee"/><br /><sub><b>Nufflee</b></sub></a><br /></td>
      <td align="center"><a href="https://github.com/mmcArg"><img src="https://github.com/mmcArg.png" width="100px;" alt="mmcArg"/><br /><sub><b>mmcArg</b></sub></a><br /></td>
      <td align="center"><a href="https://github.com/JohnInWI"><img src="https://github.com/JohnInWI.png" width="100px;" alt="JohnInWI"/><br /><sub><b>JohnInWI</b></sub></a><br /></td>
      <td align="center"><a href="https://github.com/logdog2709"><img src="https://github.com/logdog2709.png" width="100px;" alt="logdog2709"/><br /><sub><b>logdog2709</b></sub></a><br /></td>
       <td align="center"><a href="https://github.com/elC0mpa"><img src="https://github.com/elC0mpa.png" width="100px;" alt="elC0mpa"/><br /><sub><b>elC0mpa</b></sub></a><br /></td>
 </tr>
    
 <tr>
      <td align="center"><a href="https://github.com/oofnik"><img src="https://github.com/oofnik.png" width="100px;" alt="oofnik"/><br /><sub><b>⭐️ oofnik</b></sub></a><br /></td>
          <td align="center"><a href="https://github.com/zastrixarundell"><img src="https://github.com/zastrixarundell.png" width="100px;" alt="zastrixarundell"/><br /><sub><b>⭐️ zastrixarundell</b></sub></a><br /></td>
        <td align="center"><a href="https://github.com/elielmarcos"><img src="https://github.com/elielmarcos.png" width="100px;" alt="elielmarcos"/><br /><sub><b>elielmarcos</b></sub></a><br /></td>

 </tr>
 
</table>

## Change Log
- **14/02/2019 (v0.1.1)** - Initial commits and support for ESP32 and ESP8266 Websocket Clients.
- **16/02/2019 (v0.1.2)** - Added support for events (Pings, Pongs) and more internal improvements (events handling according to [RFC-6455](https://tools.ietf.org/html/rfc6455))
- **20/02/2019 (v0.1.3)** - Users now dont have to specify TCP client types (ESP8266/ESP32) they are selected automatically.
- **21/02/2019 (v0.1.5)** - Bug Fixes. Client now exposes a single string connect interface.
- **24/02/2019 (v0.2.0)** - User-facing interface is now done with Arduino's `String` class. Merged more changes (mainly optimizations) from TinyWebsockets.
- **25/02/2019 (v0.2.1)** - A tiny patch. Fixed missing user-facing strings for client interface. 
- **07/03/2019 (v0.3.0)** - A version update. Now supports a websockets server, better support for fragmented messages and streams. bug fixes and more optimized networking implementations. 
- **08/03/2019 (v0.3.1)** - Small patch. Merged changes from TinyWebsockets - interface changes to callbacks (partial callbacks without WebsocketsClient& as first parameter).
- **12/03/2019 (v0.3.2)** - Fixed a bug with behaviour of WebsokcetsClient (copy c'tor and assignment operator). Added close codes from TinyWebsockets. Thank you [@ramdor](https://github.com/gilmaimon/ArduinoWebsockets/issues/2)
- **13/03/2019 (v0.3.3)** - Fixed a bug in the esp8266 networking impl. Thank you [@ramdor](https://github.com/gilmaimon/ArduinoWebsockets/issues/2)
- **14/03/2019 (v0.3.4)** - changed underling tcp impl for esp8266 and esp32 to use `setNoDelay(true)` instead of sync communication. This makes communication faster and more relaiable than default. Thank you @ramdor for pointing out these methods.
- **06/04/2019 (v0.3.5)** - added very basic support for WSS in esp8266 (no support for fingerprint/ca or any kind of chain validation). 
- **22/04/2019 (v0.4.0)** - Added WSS support for both esp8266 and esp32. E328266 can use `client.setInsecure()` (does not validate certificate chain) or `client.setFingerprint(fingerprint)` in order to use WSS. With ESP32 there is `client.setCACert(certificate)` that can be used. (Usage is same as the built in `WiFiClientSecure`).
- **18/05/2019 (v0.4.1)** - Patch! Addressed an error with some servers. The bugs where first noted in [issue #9](https://github.com/gilmaimon/ArduinoWebsockets/issues/9). Bug was that some servers will drop connections that don't use masking, and TinyWebsockets does not use masking by default. TinyWebsockets changed that and this library merged the changes.
- **24/05/2019 (v0.4.2)** - Patch! Adressed masking issues - server to client messages would get masked but RFC forbbids it. (changes merged from TinyWebsockets)
- **07/06/2019 (v0.4.3)** - Patch! Fixed a bug on some clients (mainly FireFox websokcets impl). Thank you @xgarb ([related issue](https://github.com/gilmaimon/ArduinoWebsockets/issues/11))
- **09/06/2019 (v0.4.4)** - Patch! Fixed an issue with `close` event callback not called in some cases (sudden disconnect, for example). Thank you @adelin-mcbsoft for pointing out the issue ([related issue](https://github.com/gilmaimon/ArduinoWebsockets/issues/14))
- **14/06/2019 (v0.4.5)** - Patch! Fixed a memory leak and an unnecessary use of heap memory in case of masking messages. This was discoverd thanks to [issue #16](https://github.com/gilmaimon/ArduinoWebsockets/issues/16). Thank you [xgarb](https://github.com/xgarb)!
- **13/07/2019 (v0.4.6)** - Very small update. Changed readme to document esp32's secured client behvior ([As discussed in issue #18](https://github.com/gilmaimon/ArduinoWebsockets/issues/18)). Also esp32's version of WebsocketsClient now has a `setInsecure` method. Thank you [adelin-mcbsoft](https://github.com/adelin-mcbsoft)!
- **25/07/2019 (v0.4.7)** - Bugfix. Fixed issues with receving large messages (unchecked reads) which was pointed out in [in issue #21](https://github.com/gilmaimon/ArduinoWebsockets/issues/21)). Thank you [Jonty](https://github.com/Jonty)!
- **26/07/2019 (v0.4.8)** - Feature. Added an `addHeader` method as suggested [in issue #22](https://github.com/gilmaimon/ArduinoWebsockets/issues/21)). Thank you [mmcArg](https://github.com/mmcArg)!
- **01/08/2019 (v0.4.9)** - Patch - Bugfix. Worked around a bug where connecting to unavailable endpoints would not return false (this is a bug with the `WiFiClient` library itself). Added some missing keywords. Thank you [Nufflee](https://github.com/Nufflee) for pointing out the [issue](https://github.com/gilmaimon/ArduinoWebsockets/issues/25)!
- **10/08/2019 (v0.4.10)** - Patch - Bugfix. Fixed a bug (and general in-stability) caused from unchecked and unsafe read operations on sockets. Also improved memory usage and management. Thank you [Jonty](https://github.com/Jonty) for openning and helping with the [issue](https://github.com/gilmaimon/ArduinoWebsockets/issues/26)!
- **14/09/2019 (v0.4.11)** - Bugfixes - masking settings used to not get copied when using assignment between `WebsocketClient` instances. Also handshake validation is now case insensitive. Thank you [logdog2709](https://github.com/logdog2709) for pointing out the [issue](https://github.com/gilmaimon/ArduinoWebsockets/issues/34).
- **12/10/2019 (v0.4.12)** - Patch - Messages are now sent as a single TCP buffer instead of separate messages. Thank you [elC0mpa](https://github.com/elC0mpa) for posting the [issue](https://github.com/gilmaimon/ArduinoWebsockets/issues/44).
- **19/10/2019 (v0.4.13)** - Patch - added `yield` calls in order to prevent software-watchdog resets on esp8266 (on long messages). Thank you [elC0mpa](https://github.com/elC0mpa) for documenting and helping with the [issue](https://github.com/gilmaimon/ArduinoWebsockets/issues/43).
- **22/11/2019 (v0.4.14)** - Added `rawData` and `c_str` as acccessors in `WebsocketsMessage` so now the raw data can be acccessed which should solve issue #32 and not break any existing sketch.
- **24/02/20 (v0.4.15)** - Added `Origin` and `User-Agent` headers to requests sent by the library, this seems to be required by some servers. Thank you [imesut](https://github.com/imesut) for pointing out the issue.
- **21/04/20 (v0.4.16)** - Merged pull request by @oofnik which added 2 way SSL auth for ESP32 and ES8266. Thank you very [oofnik](https://github.com/oofnik) for the contribuation.
- **25/04/20 (v0.4.17)** - Merged pull request by Luka Bodroža (@zastrixarundell) which fixed [issue #69](https://github.com/gilmaimon/ArduinoWebsockets/issues/69) - default headers (like Origin, Host) are now customizable via the `addHeader` method. Thank you [zastrixarundell](https://github.com/zastrixarundell) for the contribution.
- **23/07/20 (v0.4.18)** - Merged pull request by Adelin U (@adelin-mcbsoft) which fixed [issue #84](https://github.com/gilmaimon/ArduinoWebsockets/issues/84) - SSL bug-fixing, implemented public-key certificate validation & EC Certificates for client-side. Thank you Adelin!
- **28/11/20 (v0.5.0)** - Support for Teensy 4.1 added by the awesome [@arnoson](https://github.com/arnoson). Supporting plaintext client/server communication and providing new and useful examples. Thank you arnoson!
- **10/05/21 (v0.5.1)** - Fingerprints and Certificates in the examples were updated by [@Khoi Hoang
](https://github.com/khoih-prog). Thank you Khoi!
