#include "core.h"

#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <string.h>

static void test_stats(void) {
    const double values[] = {3.0, -1.0, 4.0, 2.0};
    clua_stats stats;

    assert(clua_stats_calculate(NULL, 0, &stats) == 0);
    assert(clua_stats_calculate(values, 4, &stats) == 1);
    assert(stats.count == 4);
    assert(stats.sum == 8.0);
    assert(stats.min == -1.0);
    assert(stats.max == 4.0);
}

static void test_crc32(void) {
    assert(clua_crc32((const unsigned char *)"", 0) == UINT32_C(0));
    assert(clua_crc32((const unsigned char *)"123456789", 9) == UINT32_C(0xCBF43926));
}

static void test_endian_helpers(void) {
    unsigned char bytes16[2];
    unsigned char bytes32[4];

    clua_write_u16le(bytes16, UINT16_C(0xBEEF));
    assert(memcmp(bytes16, "\xEF\xBE", 2) == 0);
    assert(clua_read_u16le(bytes16) == UINT16_C(0xBEEF));

    clua_write_u16be(bytes16, UINT16_C(0xBEEF));
    assert(memcmp(bytes16, "\xBE\xEF", 2) == 0);
    assert(clua_read_u16be(bytes16) == UINT16_C(0xBEEF));

    clua_write_u32le(bytes32, UINT32_C(0x12345678));
    assert(memcmp(bytes32, "\x78\x56\x34\x12", 4) == 0);
    assert(clua_read_u32le(bytes32) == UINT32_C(0x12345678));

    clua_write_u32be(bytes32, UINT32_C(0x12345678));
    assert(memcmp(bytes32, "\x12\x34\x56\x78", 4) == 0);
    assert(clua_read_u32be(bytes32) == UINT32_C(0x12345678));
}

static void test_float_helpers(void) {
    unsigned char bytes[8];

    clua_write_f32le(bytes, 1.0f);
    assert(memcmp(bytes, "\x00\x00\x80\x3F", 4) == 0);
    assert(clua_read_f32le(bytes) == 1.0f);
    clua_write_f32be(bytes, 1.0f);
    assert(memcmp(bytes, "\x3F\x80\x00\x00", 4) == 0);
    assert(clua_read_f32be(bytes) == 1.0f);

    clua_write_f64le(bytes, 1.0);
    assert(memcmp(bytes, "\x00\x00\x00\x00\x00\x00\xF0\x3F", 8) == 0);
    assert(clua_read_f64le(bytes) == 1.0);
    clua_write_f64be(bytes, 1.0);
    assert(memcmp(bytes, "\x3F\xF0\x00\x00\x00\x00\x00\x00", 8) == 0);
    assert(clua_read_f64be(bytes) == 1.0);
}

int main(void) {
    test_stats();
    test_crc32();
    test_endian_helpers();
    test_float_helpers();
    return 0;
}
