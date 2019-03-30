#include <tiny_websockets/internals/wscrypto/crypto.hpp>
#include <tiny_websockets/internals/wscrypto/base64.hpp>
#include <tiny_websockets/internals/wscrypto/sha1.hpp>

#ifndef _WS_CONFIG_NO_TRUE_RANDOMNESS
#include <time.h>
#endif

namespace websockets { namespace crypto {
  WSString base64Encode(WSString data) {
    return internals::base64_encode(reinterpret_cast<const uint8_t*>(data.c_str()), data.size());
  }
  WSString base64Encode(uint8_t* data, size_t len) {
    return internals::base64_encode(reinterpret_cast<const uint8_t*>(data), len);
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

#ifdef _WS_CONFIG_NO_TRUE_RANDOMNESS
  WSString randomBytes(size_t len) {
    WSString result;
    result.reserve(len);

    for(size_t i = 0; i < len; i++) {
      result += "0123456789abcdef"[i % 16];
    }

    return result;
  }
#else
  WSString randomBytes(size_t len) {
    static int onlyOnce = [](){
      srand(time(NULL));
      return 0;
    }();

    WSString result;
    result.reserve(len);

    for(size_t i = onlyOnce; i < len; i++) {
      result += "0123456789abcdefABCDEFGHIJKLMNOPQRSTUVEXYZ"[rand() % 42];
    }
    return result;
  }
#endif
}} // websockets::crypto