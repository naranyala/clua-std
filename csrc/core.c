#include "core.h"

#include <string.h>

_Static_assert(sizeof(float) == 4, "cl requires 32-bit float support");
_Static_assert(sizeof(double) == 8, "cl requires 64-bit double support");

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

static uint64_t read_u64le(const unsigned char bytes[8]) {
    uint64_t value = 0;
    for (unsigned i = 0; i < 8; ++i) value |= (uint64_t)bytes[i] << (8 * i);
    return value;
}

static uint64_t read_u64be(const unsigned char bytes[8]) {
    uint64_t value = 0;
    for (unsigned i = 0; i < 8; ++i) value |= (uint64_t)bytes[i] << (8 * (7 - i));
    return value;
}

static void write_u64le(unsigned char bytes[8], uint64_t value) {
    for (unsigned i = 0; i < 8; ++i) bytes[i] = (unsigned char)(value >> (8 * i));
}

static void write_u64be(unsigned char bytes[8], uint64_t value) {
    for (unsigned i = 0; i < 8; ++i) bytes[i] = (unsigned char)(value >> (8 * (7 - i)));
}

float clua_read_f32le(const unsigned char bytes[4]) {
    const uint32_t raw = clua_read_u32le(bytes);
    float value;
    memcpy(&value, &raw, sizeof(value));
    return value;
}

float clua_read_f32be(const unsigned char bytes[4]) {
    const uint32_t raw = clua_read_u32be(bytes);
    float value;
    memcpy(&value, &raw, sizeof(value));
    return value;
}

double clua_read_f64le(const unsigned char bytes[8]) {
    const uint64_t raw = read_u64le(bytes);
    double value;
    memcpy(&value, &raw, sizeof(value));
    return value;
}

double clua_read_f64be(const unsigned char bytes[8]) {
    const uint64_t raw = read_u64be(bytes);
    double value;
    memcpy(&value, &raw, sizeof(value));
    return value;
}

void clua_write_f32le(unsigned char bytes[4], float value) {
    uint32_t raw;
    memcpy(&raw, &value, sizeof(raw));
    clua_write_u32le(bytes, raw);
}

void clua_write_f32be(unsigned char bytes[4], float value) {
    uint32_t raw;
    memcpy(&raw, &value, sizeof(raw));
    clua_write_u32be(bytes, raw);
}

void clua_write_f64le(unsigned char bytes[8], double value) {
    uint64_t raw;
    memcpy(&raw, &value, sizeof(raw));
    write_u64le(bytes, raw);
}

void clua_write_f64be(unsigned char bytes[8], double value) {
    uint64_t raw;
    memcpy(&raw, &value, sizeof(raw));
    write_u64be(bytes, raw);
}
