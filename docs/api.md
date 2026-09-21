# API reference

Load the module with:

```lua
local cl = require("cl")
```

All byte offsets are zero-based. Public functions validate their arguments and
raise a Lua error on invalid types, ranges, or buffer bounds.

## `cl.mem`

### `cl.mem.new(size[, fill]) -> buffer`

Creates a mutable native byte buffer of `size` bytes. `size` must be
non-negative. `fill` defaults to `0` and must be an integer from `0` through
`255`.

### `cl.mem.from_string(value) -> buffer`

Creates a buffer containing a copy of a Lua string, including embedded NUL
bytes.

### `cl.mem.is_buffer(value) -> boolean`

Returns whether `value` is a `cl.buffer` userdata.

## `cl.buffer`

Buffers are owned by Lua and are reclaimed by Lua's garbage collector. The
module does not expose their raw address to Lua.

### `buffer:size() -> integer`

Returns the number of bytes.

`#buffer` is equivalent to `buffer:size()`, and `tostring(buffer)` returns a
description such as `cl.buffer(16)` rather than the byte contents.

### `buffer:read(offset) -> integer`

Reads one byte at `offset`, returning `0` through `255`.

### `buffer:write(offset, byte) -> buffer`

Writes one byte and returns the same buffer for chaining.

### `buffer:fill(byte, offset[, length]) -> buffer`

Fills `length` bytes starting at `offset`. If `length` is omitted or `nil`,
the range runs to the end of the buffer. A zero-length range is valid.

### `buffer:copy(destination_offset, source, source_offset, length) -> buffer`

Copies bytes from one buffer to another and returns the destination buffer.
The operation is overlap-safe when source and destination are the same buffer.

### `buffer:to_string([offset[, length]]) -> string`

Returns a copy of a byte range as a Lua string. The default range is the full
buffer. Embedded NUL bytes are preserved.

### `buffer:clear() -> buffer`

Sets every byte to zero and returns the same buffer.

### `buffer:slice(offset[, length]) -> buffer`

Returns an independent copy of a range. Mutating the slice does not mutate the
original buffer.

### `buffer:equals(other) -> boolean`

Returns true only when both buffers have the same length and identical bytes.

### `buffer:compare(other) -> integer`

Performs lexicographic byte comparison and returns `-1`, `0`, or `1`.

## `cl.bin`

Packers return Lua binary strings. Readers accept either a Lua string or a
`cl.buffer`. Reader offsets are zero-based and default to `0`.

| Packer | Reader | Meaning |
| --- | --- | --- |
| `u8(value)` | `read_u8(source[, offset])` | unsigned 8-bit |
| `i8(value)` | `read_i8(source[, offset])` | signed 8-bit |
| `u16le(value)` | `read_u16le(source[, offset])` | unsigned 16-bit little-endian |
| `u16be(value)` | `read_u16be(source[, offset])` | unsigned 16-bit big-endian |
| `i16le(value)` | `read_i16le(source[, offset])` | signed 16-bit little-endian |
| `i16be(value)` | `read_i16be(source[, offset])` | signed 16-bit big-endian |
| `u32le(value)` | `read_u32le(source[, offset])` | unsigned 32-bit little-endian |
| `u32be(value)` | `read_u32be(source[, offset])` | unsigned 32-bit big-endian |
| `i32le(value)` | `read_i32le(source[, offset])` | signed 32-bit little-endian |
| `i32be(value)` | `read_i32be(source[, offset])` | signed 32-bit big-endian |

Packers reject values outside the selected type's range. Readers reject
sources that do not contain the complete fixed-width value.

### `cl.bin.crc32(source) -> integer`

Calculates the standard IEEE 802.3 CRC-32. `source` may be a Lua string or a
`cl.buffer`. The result is returned as a non-negative Lua integer.

## `cl.math`

### `cl.math.stats(values) -> table`

Calculates statistics for a non-empty array-like Lua table of numbers. The
result contains:

```lua
{
    count = integer,
    sum = number,
    mean = number,
    min = number,
    max = number,
}
```

The implementation reads elements `1` through `#values` in order. Empty tables,
non-number elements, NaN, and positive or negative infinity are rejected.
Finite floating-point behavior follows the C `double` arithmetic used by the
native implementation and the input order determines the summation order.

## `cl.bits`

All arguments are Lua integers. `band`, `bor`, `bxor`, and `bnot` perform
bitwise operations using the platform Lua integer width. `lshift` and
`rshift` require a count from `0` through `width - 1`; invalid counts raise an
error. `rol` and `ror` normalize their count modulo the integer width.

- `band(left, right) -> integer`
- `bor(left, right) -> integer`
- `bxor(left, right) -> integer`
- `bnot(value) -> integer`
- `lshift(value, count) -> integer`
- `rshift(value, count) -> integer`
- `rol(value, count) -> integer`
- `ror(value, count) -> integer`
- `popcount(value) -> integer`

`rshift` is a logical right shift over the unsigned representation.

## `cl.version() -> string`

Returns the module version, currently `"0.3.0"`.
