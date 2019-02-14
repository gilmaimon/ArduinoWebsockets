#pragma once

#include "ws_common.h"
#include "crypto/base64.h"
#include "crypto/sha1.h"

namespace websockets { namespace crypto {
    WSString base64Encode(WSString data) {
      return internals::base64_encode((const unsigned char *) data.c_str(), data.size());
    }
    WSString base64Encode(uint8_t* data, size_t len) {
      return internals::base64_encode((const unsigned char *) data, len);
    }
    
    WSString base64Decode(WSString data) {
      return internals::base64_decode(data);
    }

    WSString websocketsHandshakeEncodeKey(WSString key) {
        char base64[30];
        internals::sha1(key.c_str())
          .add("258EAFA5-E914-47DA-95CA-C5AB0DC85B11")
          .finalize()
          .print_base64(base64);
        
        return WSString(base64);
    }

    WSString randomBytes(size_t len) {
      // TODO: fix dummy implementation (16 is the number of websockets key length for handshakes)
      if(len == 16) {
        return "1234567890123456";
      }
      return "";
    }
}} // websockets::crypto