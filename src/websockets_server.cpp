#include <tiny_websockets/server.hpp>
#include <tiny_websockets/internals/wscrypto/crypto.hpp>
#include <memory>
#include <map>

namespace websockets {
    WebsocketsServer::WebsocketsServer(network::TcpServer* server) : _server(server) {}

    bool WebsocketsServer::available() {
        return this->_server->available();
    }

    void WebsocketsServer::listen(uint16_t port) {
        this->_server->listen(port);
    }

    bool WebsocketsServer::poll() {
        return this->_server->poll();
    }

    struct ParsedHandshakeParams {
        WSString head;
        // To store original headers
        std::map<WSString, WSString> headers;       
        // To store lowercased headers
        std::map<WSString, WSString> lowheaders;
    };

    ParsedHandshakeParams recvHandshakeRequest(network::TcpClient& client) {
        ParsedHandshakeParams result;

        result.head = client.readLine();

        WSString line = client.readLine();
        do {
            WSString key = "", value = "";
            size_t idx = 0;

            // read key
            while(idx < line.size() && line[idx] != ':') {
                key += line[idx];
                idx++;
            }

            // skip key and whitespace
            idx++;
            while(idx < line.size() && (line[idx] == ' ' || line[idx] == '\t')) idx++;

            // read value (until \r\n)
            while(idx < line.size() && line[idx] != '\r') {
                value += line[idx];
                idx++;
            }

            // store headers before tolower(), so we can search both
            result.headers[key] = value;
           
            // convert to lower case
            std::transform(key.begin(), key.end(), key.begin(), ::tolower);
            
            // Important, don't change these case-sensitive data : `Sec-WebSocket-Key` and `Origin`
            if ( (key != WS_KEY_LOWER_CASE) && (key != HEADER_ORIGIN_LOWER_CASE) )
            {    
              std::transform(value.begin(), value.end(), value.begin(), ::tolower);
            }
        
            // store header after tolower()
            result.lowheaders[key] = value;

            line = client.readLine();
        } while(client.available() && line != "\r\n");

        return result;
    }

    WebsocketsClient WebsocketsServer::accept() {
        std::shared_ptr<network::TcpClient> tcpClient(_server->accept());
        if(tcpClient->available() == false) return {};
        
        auto params = recvHandshakeRequest(*tcpClient);
        
        if ( (params.headers[HEADER_CONNECTION_NORMAL].find(HEADER_UPGRADE_NORMAL) == std::string::npos) && 
             (params.lowheaders[HEADER_CONNECTION_LOWER_CASE].find(HEADER_UPGRADE_LOWER_CASE) == std::string::npos) ) return {};         
        if ( (params.headers[HEADER_UPGRADE_NORMAL] != HEADER_WEBSOCKET_LOWER_CASE) && 
             (params.lowheaders[HEADER_UPGRADE_LOWER_CASE] != HEADER_WEBSOCKET_LOWER_CASE) ) return {};         
        if ( (params.headers[WS_VERSION_NORMAL] != "13") && (params.lowheaders[WS_VERSION_LOWER_CASE] != "13") ) return {};       
        if ( (params.headers[WS_KEY_NORMAL] == "") && (params.lowheaders[WS_KEY_LOWER_CASE] == "") ) return {};
      
        auto serverAccept = crypto::websocketsHandshakeEncodeKey(params.lowheaders[WS_KEY_LOWER_CASE]);
      
        tcpClient->send("HTTP/1.1 101 Switching Protocols\r\n");
        tcpClient->send(HEADER_CONNECTION_UPGRADE_NORMAL);
        tcpClient->send(HEADER_UPGRADE_WS_NORMAL);
        tcpClient->send(HEADER_WS_VERSION_13_NORMAL);
        tcpClient->send(HEADER_WS_ACCEPT_NORMAL + serverAccept + HEADER_HOST_RN);
        tcpClient->send(HEADER_HOST_RN);
                
        WebsocketsClient wsClient(tcpClient);
        // Don't use masking from server to client (according to RFC)
        wsClient.setUseMasking(false);
        return wsClient;
    }

    WebsocketsServer::~WebsocketsServer() {
        this->_server->close();
    }

} //websockets
