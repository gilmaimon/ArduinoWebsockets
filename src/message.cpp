#include <tiny_websockets/message.hpp>

namespace websockets { 
  MessageType messageTypeFromOpcode(uint8_t opcode) {
      switch(opcode) {
          case internals::ContentType::Binary: return MessageType::Binary;
          case internals::ContentType::Text: return MessageType::Text;
          case internals::ContentType::Ping: return MessageType::Ping;
          case internals::ContentType::Pong: return MessageType::Pong;
          case internals::ContentType::Close: return MessageType::Close;
          default: return MessageType::Empty;
      }
  }
}