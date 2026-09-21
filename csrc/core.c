#include "core.h"

int clua_stats_calculate(const double *values, size_t count, clua_stats *out) {
    if (count == 0) {
        return 0;
    }

    out->count = count;
    out->sum = values[0];
    out->min = values[0];
    out->max = values[0];

    for (size_t i = 1; i < count; ++i) {
        const double value = values[i];
        out->sum += value;
        if (value < out->min) out->min = value;
        if (value > out->max) out->max = value;
    }
    return 1;
}

uint32_t clua_crc32(const unsigned char *bytes, size_t length) {
    uint32_t crc = UINT32_C(0xFFFFFFFF);
    for (size_t i = 0; i < length; ++i) {
        crc ^= bytes[i];
        for (unsigned bit = 0; bit < 8; ++bit) {
            crc = (crc >> 1) ^ (UINT32_C(0xEDB88320) & -(int32_t)(crc & 1));
        }
    }
    return ~crc;
}

uint16_t clua_read_u16le(const unsigned char bytes[2]) {
    return (uint16_t)bytes[0] | ((uint16_t)bytes[1] << 8);
}

uint32_t clua_read_u32le(const unsigned char bytes[4]) {
    return (uint32_t)bytes[0]
        | ((uint32_t)bytes[1] << 8)
        | ((uint32_t)bytes[2] << 16)
        | ((uint32_t)bytes[3] << 24);
}

uint16_t clua_read_u16be(const unsigned char bytes[2]) {
    return ((uint16_t)bytes[0] << 8) | (uint16_t)bytes[1];
}

uint32_t clua_read_u32be(const unsigned char bytes[4]) {
    return ((uint32_t)bytes[0] << 24)
        | ((uint32_t)bytes[1] << 16)
        | ((uint32_t)bytes[2] << 8)
        | (uint32_t)bytes[3];
}

void clua_write_u16le(unsigned char bytes[2], uint16_t value) {
    bytes[0] = (unsigned char)value;
    bytes[1] = (unsigned char)(value >> 8);
}

void clua_write_u32le(unsigned char bytes[4], uint32_t value) {
    bytes[0] = (unsigned char)value;
    bytes[1] = (unsigned char)(value >> 8);
    bytes[2] = (unsigned char)(value >> 16);
    bytes[3] = (unsigned char)(value >> 24);
}

void clua_write_u16be(unsigned char bytes[2], uint16_t value) {
    bytes[0] = (unsigned char)(value >> 8);
    bytes[1] = (unsigned char)value;
}

void clua_write_u32be(unsigned char bytes[4], uint32_t value) {
    bytes[0] = (unsigned char)(value >> 24);
    bytes[1] = (unsigned char)(value >> 16);
    bytes[2] = (unsigned char)(value >> 8);
    bytes[3] = (unsigned char)value;
}
