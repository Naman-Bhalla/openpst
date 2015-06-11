/**
* LICENSE PLACEHOLDER
*/

#include "endian.h"

uint16_t flip_endian16(uint16_t value) {
    uint16_t result = 0;
    result |= (value & 0xFF) << 8;
    result |= (value & 0xFF00) >> 8;
    return result;
}

uint32_t flip_endian32(uint32_t value) {
    uint32_t result = 0;
    result |= (value & 0xFF) << 24;
    result |= (value & 0xFF00) << 8;
    result |= (value & 0xFF0000) >> 8;
    result |= (value & 0xFF000000) >> 24;
    return result;
}
