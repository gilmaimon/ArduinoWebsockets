#include <tiny_websockets/internals/ws_common.hpp>

namespace websockets { namespace internals {
    WSString fromInterfaceString(const WSInterfaceString& str) {
        return str.c_str();
    }
    WSString fromInterfaceString(const WSInterfaceString&& str) {
        return str.c_str();
    }

    WSInterfaceString fromInternalString(const WSString& str) {
        return str.c_str();
    }
    WSInterfaceString fromInternalString(const WSString&& str) {
        return str.c_str();
    }
}}