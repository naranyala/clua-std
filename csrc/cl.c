#include <lua.h>
#include <lauxlib.h>

#include <stdint.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "core.h"

#define CL_BUFFER_MT "cl.buffer"

/* Exact round-trips for the full unsigned 32-bit range require this. */
_Static_assert(sizeof(lua_Integer) >= 8, "cl requires a 64-bit Lua integer type");

typedef struct { size_t size; unsigned char bytes[]; } cl_buffer;

static size_t check_size(lua_State *L, int arg) {
    const lua_Integer value = luaL_checkinteger(L, arg);
    if (value < 0 || (uintmax_t)value > (uintmax_t)SIZE_MAX) luaL_argerror(L, arg, "must be a non-negative size");
    return (size_t)value;
}

static cl_buffer *check_buffer(lua_State *L, int arg) { return (cl_buffer *)luaL_checkudata(L, arg, CL_BUFFER_MT); }

static void check_range(lua_State *L, int arg, size_t offset, size_t length, size_t size) {
    if (offset > size || length > size - offset) luaL_argerror(L, arg, "range is outside the buffer");
}

static const unsigned char *check_byte_source(lua_State *L, int arg, size_t *length);

static cl_buffer *push_buffer(lua_State *L, size_t size) {
    if (size > SIZE_MAX - sizeof(cl_buffer)) luaL_error(L, "buffer is too large");
    cl_buffer *buffer = (cl_buffer *)lua_newuserdata(L, sizeof(*buffer) + size);
    buffer->size = size;
    luaL_setmetatable(L, CL_BUFFER_MT);
    return buffer;
}

static int check_number_array(lua_State *L, int index, double **values, size_t *count) {
    index = lua_absindex(L, index);
    luaL_checktype(L, index, LUA_TTABLE);
    const lua_Integer length = luaL_len(L, index);
    if (length < 1) return luaL_error(L, "expected a non-empty array");
    if ((uintmax_t)length > (uintmax_t)(SIZE_MAX / sizeof(**values))) return luaL_error(L, "array is too large");
    *count = (size_t)length;
    *values = malloc(*count * sizeof(**values));
    if (*values == NULL) return luaL_error(L, "out of memory");
    for (size_t i = 0; i < *count; ++i) {
        lua_geti(L, index, (lua_Integer)i + 1);
        if (!lua_isnumber(L, -1)) {
            free(*values);
            return luaL_error(L, "array element %zu must be a number", i + 1);
        }
        (*values)[i] = lua_tonumber(L, -1);
        if (!isfinite((*values)[i])) {
            free(*values);
            return luaL_error(L, "array element %zu must be finite", i + 1);
        }
        lua_pop(L, 1);
    }
    return 0;
}

static int l_buffer_new(lua_State *L) {
    cl_buffer *buffer = push_buffer(L, check_size(L, 1));
    const lua_Integer value = luaL_optinteger(L, 2, 0);
    if (value < 0 || value > 255) return luaL_argerror(L, 2, "must be a byte (0..255)");
    memset(buffer->bytes, (int)value, buffer->size);
    return 1;
}

static int l_buffer_from_string(lua_State *L) {
    size_t size;
    const char *text = luaL_checklstring(L, 1, &size);
    cl_buffer *buffer = push_buffer(L, size);
    memcpy(buffer->bytes, text, size);
    return 1;
}

static int l_buffer_size(lua_State *L) { lua_pushinteger(L, (lua_Integer)check_buffer(L, 1)->size); return 1; }

static int l_buffer_read(lua_State *L) {
    cl_buffer *buffer = check_buffer(L, 1);
    const size_t offset = check_size(L, 2);
    check_range(L, 2, offset, 1, buffer->size);
    lua_pushinteger(L, buffer->bytes[offset]);
    return 1;
}

static int l_buffer_write(lua_State *L) {
    cl_buffer *buffer = check_buffer(L, 1);
    const size_t offset = check_size(L, 2);
    const lua_Integer value = luaL_checkinteger(L, 3);
    check_range(L, 2, offset, 1, buffer->size);
    if (value < 0 || value > 255) return luaL_argerror(L, 3, "must be a byte (0..255)");
    buffer->bytes[offset] = (unsigned char)value;
    lua_settop(L, 1);
    return 1;
}

static int l_buffer_fill(lua_State *L) {
    cl_buffer *buffer = check_buffer(L, 1);
    const lua_Integer value = luaL_checkinteger(L, 2);
    const size_t offset = check_size(L, 3);
    check_range(L, 3, offset, 0, buffer->size);
    const size_t length = lua_isnoneornil(L, 4) ? buffer->size - offset : check_size(L, 4);
    if (value < 0 || value > 255) return luaL_argerror(L, 2, "must be a byte (0..255)");
    check_range(L, 3, offset, length, buffer->size);
    memset(buffer->bytes + offset, (int)value, length);
    lua_settop(L, 1);
    return 1;
}

static int l_buffer_copy(lua_State *L) {
    cl_buffer *destination = check_buffer(L, 1);
    const size_t destination_offset = check_size(L, 2);
    cl_buffer *source = check_buffer(L, 3);
    const size_t source_offset = check_size(L, 4);
    const size_t length = check_size(L, 5);
    check_range(L, 2, destination_offset, length, destination->size);
    check_range(L, 4, source_offset, length, source->size);
    memmove(destination->bytes + destination_offset, source->bytes + source_offset, length);
    lua_settop(L, 1);
    return 1;
}

static int l_buffer_to_string(lua_State *L) {
    cl_buffer *buffer = check_buffer(L, 1);
    const size_t offset = lua_isnoneornil(L, 2) ? 0 : check_size(L, 2);
    check_range(L, 2, offset, 0, buffer->size);
    const size_t length = lua_isnoneornil(L, 3) ? buffer->size - offset : check_size(L, 3);
    check_range(L, 2, offset, length, buffer->size);
    lua_pushlstring(L, (const char *)buffer->bytes + offset, length);
    return 1;
}

static int l_buffer_clear(lua_State *L) {
    cl_buffer *buffer = check_buffer(L, 1);
    memset(buffer->bytes, 0, buffer->size);
    lua_settop(L, 1);
    return 1;
}

static int l_buffer_slice(lua_State *L) {
    cl_buffer *buffer = check_buffer(L, 1);
    const size_t offset = check_size(L, 2);
    check_range(L, 2, offset, 0, buffer->size);
    const size_t length = lua_isnoneornil(L, 3) ? buffer->size - offset : check_size(L, 3);
    check_range(L, 2, offset, length, buffer->size);
    cl_buffer *result = push_buffer(L, length);
    memcpy(result->bytes, buffer->bytes + offset, length);
    return 1;
}

static int l_buffer_compare(lua_State *L) {
    cl_buffer *left = check_buffer(L, 1);
    cl_buffer *right = check_buffer(L, 2);
    const size_t common = left->size < right->size ? left->size : right->size;
    const int comparison = memcmp(left->bytes, right->bytes, common);
    lua_pushinteger(L, comparison != 0 ? (comparison < 0 ? -1 : 1) : (left->size > right->size) - (left->size < right->size));
    return 1;
}

static int l_buffer_equals(lua_State *L) {
    cl_buffer *left = check_buffer(L, 1);
    cl_buffer *right = check_buffer(L, 2);
    lua_pushboolean(L, left->size == right->size && memcmp(left->bytes, right->bytes, left->size) == 0);
    return 1;
}

static int l_buffer_find(lua_State *L) {
    cl_buffer *buffer = check_buffer(L, 1);
    size_t needle_length;
    const unsigned char *needle = check_byte_source(L, 2, &needle_length);
    const size_t offset = lua_isnoneornil(L, 3) ? 0 : check_size(L, 3);
    check_range(L, 3, offset, 0, buffer->size);
    if (needle_length == 0) {
        lua_pushinteger(L, (lua_Integer)offset);
        return 1;
    }
    if (needle_length > buffer->size - offset) {
        lua_pushnil(L);
        return 1;
    }
    for (size_t position = offset; position <= buffer->size - needle_length; ++position) {
        if (memcmp(buffer->bytes + position, needle, needle_length) == 0) {
            lua_pushinteger(L, (lua_Integer)position);
            return 1;
        }
    }
    lua_pushnil(L);
    return 1;
}

static int l_buffer_starts_with(lua_State *L) {
    cl_buffer *buffer = check_buffer(L, 1);
    size_t needle_length;
    const unsigned char *needle = check_byte_source(L, 2, &needle_length);
    lua_pushboolean(L, needle_length <= buffer->size && memcmp(buffer->bytes, needle, needle_length) == 0);
    return 1;
}

static int l_buffer_ends_with(lua_State *L) {
    cl_buffer *buffer = check_buffer(L, 1);
    size_t needle_length;
    const unsigned char *needle = check_byte_source(L, 2, &needle_length);
    lua_pushboolean(L, needle_length <= buffer->size && memcmp(buffer->bytes + buffer->size - needle_length, needle, needle_length) == 0);
    return 1;
}

static int l_buffer_reverse(lua_State *L) {
    cl_buffer *buffer = check_buffer(L, 1);
    for (size_t left = 0, right = buffer->size; left < right / 2; ++left) {
        const size_t other = right - left - 1;
        const unsigned char value = buffer->bytes[left];
        buffer->bytes[left] = buffer->bytes[other];
        buffer->bytes[other] = value;
    }
    lua_settop(L, 1);
    return 1;
}

static int l_mem_is_buffer(lua_State *L) {
    lua_pushboolean(L, luaL_testudata(L, 1, CL_BUFFER_MT) != NULL);
    return 1;
}

static int l_buffer_tostring(lua_State *L) {
    char description[64];
    const cl_buffer *buffer = check_buffer(L, 1);
    snprintf(description, sizeof(description), "cl.buffer(%zu)", buffer->size);
    lua_pushstring(L, description);
    return 1;
}

static int l_stats(lua_State *L) {
    double *values; size_t count;
    if (check_number_array(L, 1, &values, &count) != 0) return 0;
    clua_stats stats;
    clua_stats_calculate(values, count, &stats);
    free(values);
    lua_createtable(L, 0, 5);
    lua_pushinteger(L, (lua_Integer)stats.count); lua_setfield(L, -2, "count");
    lua_pushnumber(L, stats.sum); lua_setfield(L, -2, "sum");
    lua_pushnumber(L, stats.sum / (double)stats.count); lua_setfield(L, -2, "mean");
    lua_pushnumber(L, stats.min); lua_setfield(L, -2, "min");
    lua_pushnumber(L, stats.max); lua_setfield(L, -2, "max");
    return 1;
}

static int l_crc32(lua_State *L) {
    size_t length;
    const unsigned char *bytes = check_byte_source(L, 1, &length);
    lua_pushinteger(L, (lua_Integer)clua_crc32(bytes, length));
    return 1;
}

static const unsigned char *check_byte_source(lua_State *L, int arg, size_t *length) {
    cl_buffer *buffer = (cl_buffer *)luaL_testudata(L, arg, CL_BUFFER_MT);
    if (buffer != NULL) {
        *length = buffer->size;
        return buffer->bytes;
    }
    return (const unsigned char *)luaL_checklstring(L, arg, length);
}

static int hex_digit(unsigned char value) {
    if (value >= '0' && value <= '9') return value - '0';
    if (value >= 'a' && value <= 'f') return value - 'a' + 10;
    if (value >= 'A' && value <= 'F') return value - 'A' + 10;
    return -1;
}

static int l_bytes_to_hex(lua_State *L) {
    size_t length;
    const unsigned char *bytes = check_byte_source(L, 1, &length);
    static const char digits[] = "0123456789ABCDEF";
    luaL_Buffer output;
    luaL_buffinit(L, &output);
    for (size_t i = 0; i < length; ++i) {
        char pair[2] = {digits[bytes[i] >> 4], digits[bytes[i] & 0x0F]};
        luaL_addlstring(&output, pair, sizeof(pair));
    }
    luaL_pushresult(&output);
    return 1;
}

static int l_bytes_from_hex(lua_State *L) {
    size_t length;
    const char *text = luaL_checklstring(L, 1, &length);
    if ((length & 1U) != 0) return luaL_argerror(L, 1, "hex text must have an even length");
    cl_buffer *buffer = push_buffer(L, length / 2);
    for (size_t i = 0; i < length; i += 2) {
        const int high = hex_digit((unsigned char)text[i]);
        const int low = hex_digit((unsigned char)text[i + 1]);
        if (high < 0 || low < 0) return luaL_argerror(L, 1, "contains a non-hexadecimal character");
        buffer->bytes[i / 2] = (unsigned char)((high << 4) | low);
    }
    return 1;
}

static int l_bytes_concat(lua_State *L) {
    const int count = lua_gettop(L);
    size_t total = 0;
    for (int arg = 1; arg <= count; ++arg) {
        size_t length;
        (void)check_byte_source(L, arg, &length);
        if (length > SIZE_MAX - total) return luaL_error(L, "combined byte source is too large");
        total += length;
    }
    cl_buffer *result = push_buffer(L, total);
    size_t offset = 0;
    for (int arg = 1; arg <= count; ++arg) {
        size_t length;
        const unsigned char *bytes = check_byte_source(L, arg, &length);
        memcpy(result->bytes + offset, bytes, length);
        offset += length;
    }
    return 1;
}

static uint32_t read_unsigned(const unsigned char *bytes, int width, int little_endian) {
    if (width == 1) return bytes[0];
    if (width == 2) return little_endian ? clua_read_u16le(bytes) : clua_read_u16be(bytes);
    return little_endian ? clua_read_u32le(bytes) : clua_read_u32be(bytes);
}

static void write_unsigned(unsigned char *bytes, int width, int little_endian, uint32_t value) {
    if (width == 1) bytes[0] = (unsigned char)value;
    else if (width == 2) {
        if (little_endian) clua_write_u16le(bytes, (uint16_t)value);
        else clua_write_u16be(bytes, (uint16_t)value);
    } else {
        if (little_endian) clua_write_u32le(bytes, value);
        else clua_write_u32be(bytes, value);
    }
}

static int l_pack_integer(lua_State *L) {
    const int width = (int)lua_tointeger(L, lua_upvalueindex(1));
    const int signed_value = lua_toboolean(L, lua_upvalueindex(2));
    const int little_endian = lua_toboolean(L, lua_upvalueindex(3));
    const lua_Integer value = luaL_checkinteger(L, 1);
    const lua_Integer minimum = signed_value ? (width == 1 ? INT8_MIN : width == 2 ? INT16_MIN : INT32_MIN) : 0;
    const uint32_t maximum = signed_value ? (width == 1 ? INT8_MAX : width == 2 ? INT16_MAX : INT32_MAX) : (width == 1 ? UINT8_MAX : width == 2 ? UINT16_MAX : UINT32_MAX);
    if (value < minimum || (signed_value ? value > (lua_Integer)maximum : (uintmax_t)value > maximum)) {
        return luaL_argerror(L, 1, "is outside this integer type's range");
    }
    unsigned char bytes[4];
    write_unsigned(bytes, width, little_endian, (uint32_t)value);
    lua_pushlstring(L, (const char *)bytes, (size_t)width);
    return 1;
}

static int l_read_integer(lua_State *L) {
    const int width = (int)lua_tointeger(L, lua_upvalueindex(1));
    const int signed_value = lua_toboolean(L, lua_upvalueindex(2));
    const int little_endian = lua_toboolean(L, lua_upvalueindex(3));
    size_t length;
    const unsigned char *bytes = check_byte_source(L, 1, &length);
    const size_t offset = lua_isnoneornil(L, 2) ? 0 : check_size(L, 2);
    check_range(L, 2, offset, (size_t)width, length);
    const uint32_t raw = read_unsigned(bytes + offset, width, little_endian);
    if (signed_value && raw >= (width == 1 ? UINT8_C(0x80) : width == 2 ? UINT16_C(0x8000) : UINT32_C(0x80000000))) {
        const lua_Integer modulus = width == 1 ? 256 : width == 2 ? 65536 : INT64_C(4294967296);
        lua_pushinteger(L, (lua_Integer)raw - modulus);
    } else {
        lua_pushinteger(L, (lua_Integer)raw);
    }
    return 1;
}

static void add_integer_primitive(lua_State *L, const char *name, int width, int signed_value, int little_endian, int read_value) {
    lua_pushinteger(L, width);
    lua_pushboolean(L, signed_value);
    lua_pushboolean(L, little_endian);
    lua_pushcclosure(L, read_value ? l_read_integer : l_pack_integer, 3);
    lua_setfield(L, -2, name);
}

static int l_pack_float(lua_State *L) {
    const int width = (int)lua_tointeger(L, lua_upvalueindex(1));
    const int little_endian = lua_toboolean(L, lua_upvalueindex(2));
    const lua_Number value = luaL_checknumber(L, 1);
    unsigned char bytes[8];
    if (width == 4) {
        const float narrowed = (float)value;
        if (little_endian) clua_write_f32le(bytes, narrowed);
        else clua_write_f32be(bytes, narrowed);
    } else {
        if (little_endian) clua_write_f64le(bytes, (double)value);
        else clua_write_f64be(bytes, (double)value);
    }
    lua_pushlstring(L, (const char *)bytes, (size_t)width);
    return 1;
}

static int l_read_float(lua_State *L) {
    const int width = (int)lua_tointeger(L, lua_upvalueindex(1));
    const int little_endian = lua_toboolean(L, lua_upvalueindex(2));
    size_t length;
    const unsigned char *bytes = check_byte_source(L, 1, &length);
    const size_t offset = lua_isnoneornil(L, 2) ? 0 : check_size(L, 2);
    check_range(L, 2, offset, (size_t)width, length);
    if (width == 4) {
        const float value = little_endian ? clua_read_f32le(bytes + offset) : clua_read_f32be(bytes + offset);
        lua_pushnumber(L, (lua_Number)value);
    } else {
        const double value = little_endian ? clua_read_f64le(bytes + offset) : clua_read_f64be(bytes + offset);
        lua_pushnumber(L, (lua_Number)value);
    }
    return 1;
}

static void add_float_primitive(lua_State *L, const char *name, int width, int little_endian, int read_value) {
    lua_pushinteger(L, width);
    lua_pushboolean(L, little_endian);
    lua_pushcclosure(L, read_value ? l_read_float : l_pack_float, 2);
    lua_setfield(L, -2, name);
}

static int l_bits_band(lua_State *L) { lua_pushinteger(L, (lua_Integer)((lua_Unsigned)luaL_checkinteger(L, 1) & (lua_Unsigned)luaL_checkinteger(L, 2))); return 1; }
static int l_bits_bor(lua_State *L) { lua_pushinteger(L, (lua_Integer)((lua_Unsigned)luaL_checkinteger(L, 1) | (lua_Unsigned)luaL_checkinteger(L, 2))); return 1; }
static int l_bits_bxor(lua_State *L) { lua_pushinteger(L, (lua_Integer)((lua_Unsigned)luaL_checkinteger(L, 1) ^ (lua_Unsigned)luaL_checkinteger(L, 2))); return 1; }
static int l_bits_bnot(lua_State *L) { lua_pushinteger(L, (lua_Integer)~(lua_Unsigned)luaL_checkinteger(L, 1)); return 1; }

static unsigned check_shift(lua_State *L, int arg) {
    const lua_Integer shift = luaL_checkinteger(L, arg);
    const unsigned width = (unsigned)(sizeof(lua_Unsigned) * CHAR_BIT);
    if (shift < 0 || (uintmax_t)shift >= width) luaL_argerror(L, arg, "must be smaller than the integer width");
    return (unsigned)shift;
}

static int l_bits_lshift(lua_State *L) { lua_pushinteger(L, (lua_Integer)((lua_Unsigned)luaL_checkinteger(L, 1) << check_shift(L, 2))); return 1; }
static int l_bits_rshift(lua_State *L) { lua_pushinteger(L, (lua_Integer)((lua_Unsigned)luaL_checkinteger(L, 1) >> check_shift(L, 2))); return 1; }

static int l_bits_btest(lua_State *L) {
    const lua_Unsigned value = (lua_Unsigned)luaL_checkinteger(L, 1);
    const unsigned bit = check_shift(L, 2);
    lua_pushboolean(L, ((value >> bit) & 1U) != 0);
    return 1;
}

static int l_bits_bset(lua_State *L) {
    const lua_Unsigned value = (lua_Unsigned)luaL_checkinteger(L, 1);
    const unsigned bit = check_shift(L, 2);
    lua_pushinteger(L, (lua_Integer)(value | ((lua_Unsigned)1 << bit)));
    return 1;
}

static int l_bits_bclear(lua_State *L) {
    const lua_Unsigned value = (lua_Unsigned)luaL_checkinteger(L, 1);
    const unsigned bit = check_shift(L, 2);
    lua_pushinteger(L, (lua_Integer)(value & ~((lua_Unsigned)1 << bit)));
    return 1;
}

static int l_bits_rol(lua_State *L) {
    const lua_Unsigned value = (lua_Unsigned)luaL_checkinteger(L, 1);
    const unsigned width = (unsigned)(sizeof(lua_Unsigned) * CHAR_BIT);
    const unsigned shift = (unsigned)((lua_Unsigned)luaL_checkinteger(L, 2) % width);
    lua_pushinteger(L, (lua_Integer)(shift == 0 ? value : (value << shift) | (value >> (width - shift))));
    return 1;
}

static int l_bits_ror(lua_State *L) {
    const lua_Unsigned value = (lua_Unsigned)luaL_checkinteger(L, 1);
    const unsigned width = (unsigned)(sizeof(lua_Unsigned) * CHAR_BIT);
    const unsigned shift = (unsigned)((lua_Unsigned)luaL_checkinteger(L, 2) % width);
    lua_pushinteger(L, (lua_Integer)(shift == 0 ? value : (value >> shift) | (value << (width - shift))));
    return 1;
}

static int l_bits_popcount(lua_State *L) {
    lua_Unsigned value = (lua_Unsigned)luaL_checkinteger(L, 1);
    lua_Integer count = 0;
    while (value != 0) { value &= value - 1; ++count; }
    lua_pushinteger(L, count);
    return 1;
}

static int l_flags_has(lua_State *L) {
    const lua_Unsigned value = (lua_Unsigned)luaL_checkinteger(L, 1);
    const unsigned bit = check_shift(L, 2);
    lua_pushboolean(L, ((value >> bit) & 1U) != 0);
    return 1;
}

static int l_flags_set(lua_State *L) {
    const lua_Unsigned value = (lua_Unsigned)luaL_checkinteger(L, 1);
    const unsigned bit = check_shift(L, 2);
    lua_pushinteger(L, (lua_Integer)(value | ((lua_Unsigned)1 << bit)));
    return 1;
}

static int l_flags_clear(lua_State *L) {
    const lua_Unsigned value = (lua_Unsigned)luaL_checkinteger(L, 1);
    const unsigned bit = check_shift(L, 2);
    lua_pushinteger(L, (lua_Integer)(value & ~((lua_Unsigned)1 << bit)));
    return 1;
}

static int l_version(lua_State *L) { lua_pushliteral(L, "0.5.0"); return 1; }

static const luaL_Reg buffer_methods[] = {
    {"size", l_buffer_size}, {"read", l_buffer_read}, {"write", l_buffer_write}, {"fill", l_buffer_fill},
    {"copy", l_buffer_copy}, {"to_string", l_buffer_to_string}, {"clear", l_buffer_clear}, {"slice", l_buffer_slice},
    {"compare", l_buffer_compare}, {"equals", l_buffer_equals}, {"find", l_buffer_find},
    {"starts_with", l_buffer_starts_with}, {"ends_with", l_buffer_ends_with}, {"reverse", l_buffer_reverse}, {NULL, NULL}
};
static const luaL_Reg buffer_meta[] = {{"__len", l_buffer_size}, {"__tostring", l_buffer_tostring}, {NULL, NULL}};
static const luaL_Reg mem_functions[] = {{"new", l_buffer_new}, {"from_string", l_buffer_from_string}, {"is_buffer", l_mem_is_buffer}, {NULL, NULL}};
static const luaL_Reg bytes_functions[] = {{"to_hex", l_bytes_to_hex}, {"from_hex", l_bytes_from_hex}, {"concat", l_bytes_concat}, {NULL, NULL}};
static const luaL_Reg bin_functions[] = {{"crc32", l_crc32}, {NULL, NULL}};
static const luaL_Reg math_functions[] = {{"stats", l_stats}, {NULL, NULL}};
static const luaL_Reg flags_functions[] = {{"has", l_flags_has}, {"set", l_flags_set}, {"clear", l_flags_clear}, {NULL, NULL}};
static const luaL_Reg bits_functions[] = {{"band", l_bits_band}, {"bor", l_bits_bor}, {"bxor", l_bits_bxor}, {"bnot", l_bits_bnot}, {"lshift", l_bits_lshift}, {"rshift", l_bits_rshift}, {"btest", l_bits_btest}, {"bset", l_bits_bset}, {"bclear", l_bits_bclear}, {"rol", l_bits_rol}, {"ror", l_bits_ror}, {"popcount", l_bits_popcount}, {NULL, NULL}};

static void add_namespace(lua_State *L, const char *name, const luaL_Reg *functions) {
    /* Spell out luaL_newlib to avoid a Lua 5.4 macro warning with -Werror. */
    lua_newtable(L);
    luaL_setfuncs(L, functions, 0);
    lua_setfield(L, -2, name);
}

int luaopen_cl(lua_State *L) {
    luaL_newmetatable(L, CL_BUFFER_MT);
    luaL_setfuncs(L, buffer_meta, 0);
    lua_newtable(L); luaL_setfuncs(L, buffer_methods, 0); lua_setfield(L, -2, "__index");
    lua_pop(L, 1);
    lua_newtable(L);
    add_namespace(L, "mem", mem_functions);
    add_namespace(L, "bytes", bytes_functions);
    lua_newtable(L);
    luaL_setfuncs(L, bin_functions, 0);
    const char *names[] = {"u8", "i8", "u16le", "u16be", "i16le", "i16be", "u32le", "u32be", "i32le", "i32be"};
    const int widths[] = {1, 1, 2, 2, 2, 2, 4, 4, 4, 4};
    const int signed_values[] = {0, 1, 0, 0, 1, 1, 0, 0, 1, 1};
    const int little_endian[] = {1, 1, 1, 0, 1, 0, 1, 0, 1, 0};
    for (size_t i = 0; i < sizeof(names) / sizeof(names[0]); ++i) {
        add_integer_primitive(L, names[i], widths[i], signed_values[i], little_endian[i], 0);
        char read_name[20];
        snprintf(read_name, sizeof(read_name), "read_%s", names[i]);
        add_integer_primitive(L, read_name, widths[i], signed_values[i], little_endian[i], 1);
    }
    const char *float_names[] = {"f32le", "f32be", "f64le", "f64be"};
    const int float_widths[] = {4, 4, 8, 8};
    const int float_little_endian[] = {1, 0, 1, 0};
    for (size_t i = 0; i < sizeof(float_names) / sizeof(float_names[0]); ++i) {
        add_float_primitive(L, float_names[i], float_widths[i], float_little_endian[i], 0);
        char read_name[20];
        snprintf(read_name, sizeof(read_name), "read_%s", float_names[i]);
        add_float_primitive(L, read_name, float_widths[i], float_little_endian[i], 1);
    }
    lua_setfield(L, -2, "bin");
    add_namespace(L, "math", math_functions);
    add_namespace(L, "flags", flags_functions);
    add_namespace(L, "bits", bits_functions);
    lua_pushcfunction(L, l_version); lua_setfield(L, -2, "version");
    return 1;
}
