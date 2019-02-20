#include <tiny_websockets/internals/ws_common.hpp>
#include <tiny_websockets/network/tcp_client.hpp>
#include <tiny_websockets/message.hpp>
#include <tiny_websockets/client.hpp>
#include <tiny_websockets/internals/wscrypto/crypto.hpp>

namespace websockets {
    WebsocketsClient::WebsocketsClient(network::TcpClient* client) : 
        WebsocketsEndpoint(*client), 
        _client(client), 
        _connectionOpen(false),
        _messagesCallback([](WebsocketsMessage){}),
        _eventsCallback([](WebsocketsEvent, WSString){}) {
        // Empty
    }

    struct HandshakeRequestResult {
        WSString requestStr;
        WSString expectedAcceptKey;
    };
    HandshakeRequestResult generateHandshake(WSString host, WSString uri) {
        WSString key = crypto::base64Encode(crypto::randomBytes(16));

        WSString handshake = "GET " + uri + " HTTP/1.1\r\n";
        handshake += "Host: " + host + "\r\n";
        handshake += "Upgrade: websocket\r\n";
        handshake += "Connection: Upgrade\r\n";
        handshake += "Sec-WebSocket-Key: " + key + "\r\n";
        handshake += "Sec-WebSocket-Version: 13\r\n";
        handshake += "\r\n";

        HandshakeRequestResult result;
        result.requestStr = handshake;
#ifndef _WS_CONFIG_SKIP_HANDSHAKE_ACCEPT_VALIDATION
        result.expectedAcceptKey = crypto::websocketsHandshakeEncodeKey(key);
#endif
        return std::move(result);
    }

    bool isWhitespace(char ch) {
        return ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n';
    }

    struct HandshakeResponseResult {
        bool isSuccess;
        WSString serverAccept;
    };
    HandshakeResponseResult parseHandshakeResponse(WSString responseHeaders) {
        bool didUpgradeToWebsockets = false, isConnectionUpgraded = false;
        WSString serverAccept = "";
        size_t idx = 0;
        while(idx < responseHeaders.size()) {
            WSString key = "", value = "";
            // read header key
            while(idx < responseHeaders.size() && responseHeaders[idx] != ':') key += responseHeaders[idx++];

            // ignore ':' and whitespace
            ++idx;
            while(idx < responseHeaders.size() && isWhitespace(responseHeaders[idx])) idx++;

            // read header value until \r
            while(idx < responseHeaders.size() && responseHeaders[idx] != '\r') value += responseHeaders[idx++];

            // skip \r\n
            idx += 2;

            if(key == "Upgrade") {
                didUpgradeToWebsockets = (value == "websocket");
            } else if(key == "Connection") {
                isConnectionUpgraded = (value == "Upgrade");
            } else if(key == "Sec-WebSocket-Accept") {
                serverAccept = value;
            }
        }

        HandshakeResponseResult result;
        result.isSuccess = serverAccept != "" && didUpgradeToWebsockets && isConnectionUpgraded;
        result.serverAccept = serverAccept;
        return result;
    }

    bool doestStartsWith(WSString str, WSString prefix) {
        if(str.size() < prefix.size()) return false;
        for(size_t i = 0; i < prefix.size(); i++) {
            if(str[i] != prefix[i]) return false;
        }

        return true;
    }

    bool WebsocketsClient::connect(WSString url) {
        WSString protocol = "";
        if(doestStartsWith(url, "http://")) {
            protocol = "http";
            url = url.substr(strlen("http://"));
        } else if(doestStartsWith(url, "ws://")) {
            protocol = "ws";
            url = url.substr(strlen("ws://"));
        } else {
            return false;
            // Not supported
        }

        auto uriBeg = url.find_first_of('/');
        std::string host = url, uri = "/";

        if(static_cast<int>(uriBeg) != -1) {
            uri = url.substr(uriBeg);
            host = url.substr(0, uriBeg);
        }

        auto portIdx = host.find_first_of(':');
        int port = 80;
        if(static_cast<int>(portIdx) != -1) {
            auto onlyHost = host.substr(0, portIdx);
            ++portIdx;
            port = 0;
            while(portIdx < host.size() && host[portIdx] >= '0' && host[portIdx] <= '9') {
                port = port * 10 + (host[portIdx] - '0');
                ++portIdx;
            }

            host = onlyHost;
        }
        
        return connect(host, port, uri);
    }

    bool WebsocketsClient::connect(WSString host, int port, WSString path) {
        this->_connectionOpen = this->_client->connect(host, port);
        if (!this->_connectionOpen) return false;

        auto handshake = generateHandshake(host, path);
        this->_client->send(handshake.requestStr);

        auto head = this->_client->readLine();
        if(!doestStartsWith(head, "HTTP/1.1 101")) {
            close();
            return false;
        }

        WSString serverResponseHeaders = "";
        WSString line = "";
        while (true) {
            line = this->_client->readLine();
            serverResponseHeaders += line;
            if (line == "\r\n") break;
        }
        auto parsedResponse = parseHandshakeResponse(serverResponseHeaders);
        
#ifdef _WS_CONFIG_SKIP_HANDSHAKE_ACCEPT_VALIDATION
        bool serverAcceptMismatch = false;
#else
        bool serverAcceptMismatch = parsedResponse.serverAccept != handshake.expectedAcceptKey;
#endif
        if(parsedResponse.isSuccess == false || serverAcceptMismatch) {
            close();
            return false;
        }

        this->_eventsCallback(WebsocketsEvent::ConnectionOpened, "");
        return true;
    }

    void WebsocketsClient::onMessage(MessageCallback callback) {
        this->_messagesCallback = callback;
    }

    void WebsocketsClient::onEvent(EventCallback callback) {
        this->_eventsCallback = callback;
    }

    bool WebsocketsClient::poll() {
        bool messageReceived = false;
        while(available() && WebsocketsEndpoint::poll()) {
            auto msg = WebsocketsEndpoint::recv();
            messageReceived = true;
            
            if(msg.isBinary() || msg.isText()) {
                this->_messagesCallback(std::move(msg));
            } else if(msg.type() == MessageType::Ping) {
                _handlePing(std::move(msg));
            } else if(msg.type() == MessageType::Pong) {
                _handlePong(std::move(msg));
            } else if(msg.type() == MessageType::Close) {
                _handleClose(std::move(msg));
            }
        }

        return messageReceived;
    }

    bool WebsocketsClient::send(WSString data) {
        if(available()) {
            return WebsocketsEndpoint::send(data, MessageType::Text);
        }
        return false;
    }

    bool WebsocketsClient::sendBinary(WSString data) {
        if(available()) {
            return WebsocketsEndpoint::send(data, MessageType::Binary);
        }
        return false;
    }

    bool WebsocketsClient::available(bool activeTest) {
        this->_connectionOpen &= this->_client->available();
        if(this->_connectionOpen && activeTest)  {
            WebsocketsEndpoint::ping();
        }
        return _connectionOpen;
    }

    bool WebsocketsClient::ping(WSString data) {
        return WebsocketsEndpoint::ping(data);
    }

    bool WebsocketsClient::pong(WSString data) {
        return WebsocketsEndpoint::pong(data);
    }

    void WebsocketsClient::close() {
        if(available()) {
            this->_connectionOpen = false;
        }
    }

    void WebsocketsClient::_handlePing(WebsocketsMessage message) {
        this->_eventsCallback(WebsocketsEvent::GotPing, message.data());
    }

    void WebsocketsClient::_handlePong(WebsocketsMessage message) {
        this->_eventsCallback(WebsocketsEvent::GotPong, message.data());
    }

    void WebsocketsClient::_handleClose(WebsocketsMessage message) {
        if(available()) {
            this->_connectionOpen = false;
        }
        this->_eventsCallback(WebsocketsEvent::ConnectionClosed, message.data());
    }

    WebsocketsClient::~WebsocketsClient() {
        delete this->_client;
    }
}