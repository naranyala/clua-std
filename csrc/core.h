#ifndef CLUA_STD_CORE_H
#define CLUA_STD_CORE_H

#include <stddef.h>
#include <stdint.h>

typedef struct {
    size_t count;
    double sum;
    double min;
    double max;
} clua_stats;

/* Returns 0 when values is empty, otherwise populates out and returns 1. */
int clua_stats_calculate(const double *values, size_t count, clua_stats *out);

/* Standard IEEE 802.3 CRC-32, suitable for binary Lua strings. */
uint32_t clua_crc32(const unsigned char *bytes, size_t length);

uint16_t clua_read_u16le(const unsigned char bytes[2]);
uint32_t clua_read_u32le(const unsigned char bytes[4]);
uint16_t clua_read_u16be(const unsigned char bytes[2]);
uint32_t clua_read_u32be(const unsigned char bytes[4]);
void clua_write_u16le(unsigned char bytes[2], uint16_t value);
void clua_write_u32le(unsigned char bytes[4], uint32_t value);
void clua_write_u16be(unsigned char bytes[2], uint16_t value);
void clua_write_u32be(unsigned char bytes[4], uint32_t value);

float clua_read_f32le(const unsigned char bytes[4]);
float clua_read_f32be(const unsigned char bytes[4]);
double clua_read_f64le(const unsigned char bytes[8]);
double clua_read_f64be(const unsigned char bytes[8]);
void clua_write_f32le(unsigned char bytes[4], float value);
void clua_write_f32be(unsigned char bytes[4], float value);
void clua_write_f64le(unsigned char bytes[8], double value);
void clua_write_f64be(unsigned char bytes[8], double value);

#endif
