#include "ws_common.h"
#include "network/tcp_client.h"
#include "websockets/data_frame.h"
#include "websockets/message.h"
#include "websockets/websockets_client.h"
#include "wscrypto/crypto.h"

namespace websockets {
    WebsocketsClient::WebsocketsClient(network::TcpClient* client) : _client(client), WebsocketsEndpoint(*client), _connectionOpen(false) {
        // Empty
    }

    struct HandshakeRequestResult {
        WSString requestStr;
        WSString expectedAcceptKey;
    };
    HandshakeRequestResult generateHandshake(WSString uri) {
        WSString key = crypto::base64Encode(crypto::randomBytes(16));

        WSString handshake = "GET " + uri + " HTTP/1.1\r\n";
        handshake += "Upgrade: websocket\r\n";
        handshake += "Connection: Upgrade\r\n";
        handshake += "Sec-WebSocket-Key: " + key + "\r\n";
        handshake += "Sec-WebSocket-Version: 13\r\n";
        handshake += "\r\n";

        WSString expectedAccept = crypto::websocketsHandshakeEncodeKey(key);

        HandshakeRequestResult result;
        result.requestStr = handshake;
        result.expectedAcceptKey = expectedAccept;
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

            // read header value until \r\n or whitespace
            while(idx < responseHeaders.size() && !isWhitespace(responseHeaders[idx])) value += responseHeaders[idx++];

            // ignore rest of whitespace
            while(idx < responseHeaders.size() && isWhitespace(responseHeaders[idx])) idx++;

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

    bool WebsocketsClient::connect(WSString host, WSString path, int port) {
        this->_connectionOpen = this->_client->connect(host, port);
        if (!this->_connectionOpen) return false;

        auto handshake = generateHandshake(path);
        this->_client->send(handshake.requestStr);

        auto head = this->_client->readLine();
        if(head != "HTTP/1.1 101 Switching Protocols\r\n") {
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
        if(parsedResponse.isSuccess == false || parsedResponse.serverAccept != handshake.expectedAcceptKey) {
            close();
            return false;
        }

        return true;
    }

    void WebsocketsClient::onMessage(MessageCallback callback) {
        this->_callback = callback;
    }

    void WebsocketsClient::poll() {
        while(available() && WebsocketsEndpoint::poll()) {
            auto frame = WebsocketsEndpoint::recv();
            
            auto msg = WebsocketsMessage::CreateFromFrame(frame);
            if(msg.isBinary() || msg.isText()) {
                this->_callback(std::move(msg));
            } else if(msg.type() == MessageType::Ping) {
                _handlePing(std::move(msg));
            } else if(msg.type() == MessageType::Pong) {
                _handlePong(std::move(msg));
            } else if(msg.type() == MessageType::Close) {
                _handleClose(std::move(msg));
            }
        }
    }

    void WebsocketsClient::send(WSString data) {
        if(available()) {
            WebsocketsEndpoint::send(data, MessageType::Text);
        }
    }

    void WebsocketsClient::sendBinary(WSString data) {
        if(available()) {
            WebsocketsEndpoint::send(data, MessageType::Binary);
        }
    }

    bool WebsocketsClient::available(bool activeTest) {
        this->_connectionOpen &= this->_client->available();
        if(this->_connectionOpen && activeTest)  {
            WebsocketsEndpoint::ping();
        }
        return _connectionOpen;
    }

    void WebsocketsClient::close() {
        if(available()) {
            this->_connectionOpen = false;
            WebsocketsEndpoint::close(true);
        }
    }

    void WebsocketsClient::_handlePing(WebsocketsMessage) {
        // TODO handle ping
    }

    void WebsocketsClient::_handlePong(WebsocketsMessage) {
        // TODO handle pong
    }

    void WebsocketsClient::_handleClose(WebsocketsMessage) {
        if(available()) {
            this->_connectionOpen = false;
            WebsocketsEndpoint::close(false);
        }
    }

    WebsocketsClient::~WebsocketsClient() {
        delete this->_client;
    }
}