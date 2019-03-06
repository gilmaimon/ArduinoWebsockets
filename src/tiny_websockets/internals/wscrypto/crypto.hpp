#pragma once

#include <tiny_websockets/internals/ws_common.hpp>

namespace websockets { namespace crypto {
  WSString base64Encode(WSString data);
  WSString base64Encode(uint8_t* data, size_t len);
  WSString base64Decode(WSString data);
  WSString websocketsHandshakeEncodeKey(WSString key);
  WSString randomBytes(size_t len);
}} // websockets::crypto