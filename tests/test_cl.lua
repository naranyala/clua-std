local cl = require("cl")

local function must_fail(fn)
    local ok = pcall(fn)
    assert(not ok, "expected operation to fail")
end

-- Statistics: normal, singleton, fractional, and invalid inputs.
local result = cl.math.stats({ 3, -1, 4, 2 })
assert(result.count == 4 and result.sum == 8 and result.mean == 2)
assert(result.min == -1 and result.max == 4)
local singleton = cl.math.stats({ 3.5 })
assert(singleton.count == 1 and singleton.sum == 3.5)
assert(singleton.mean == 3.5 and singleton.min == 3.5 and singleton.max == 3.5)
must_fail(function() cl.math.stats({}) end)
must_fail(function() cl.math.stats({ 1, "2" }) end)
must_fail(function() cl.math.stats("not an array") end)
must_fail(function() cl.math.stats({ 0 / 0 }) end)
must_fail(function() cl.math.stats({ math.huge }) end)

-- Buffers: defaults, byte boundaries, zero-length ranges, copying, and
-- independent slices.
local buffer = cl.mem.new(6, 0x2E)
assert(#buffer == 6 and buffer:size() == 6 and tostring(buffer) == "cl.buffer(6)")
assert(cl.mem.is_buffer(buffer) and not cl.mem.is_buffer("bytes"))
assert(cl.mem.new(0):to_string() == "")
buffer:write(0, 0):write(1, 0x41):write(2, 0xFF)
assert(buffer:read(0) == 0 and buffer:read(1) == 0x41 and buffer:read(2) == 0xFF)
buffer:fill(0x21, 3, 2)
assert(buffer:to_string() == "\0A\255!!")
assert(buffer:to_string(1, 0) == "" and buffer:to_string(6, 0) == "")
buffer:fill(0x7F, 5)
assert(buffer:read(5) == 0x7F)

local copy = cl.mem.from_string("123456")
copy:copy(2, copy, 0, 4)
assert(copy:to_string() == "121234") -- overlapping copy must be memmove-safe
local backward = cl.mem.from_string("123456")
backward:copy(0, backward, 2, 4)
assert(backward:to_string() == "345656")
local slice = copy:slice(1, 3)
assert(slice:to_string() == "212" and #slice == 3)
slice:write(0, string.byte("X"))
assert(copy:to_string() == "121234") -- slice is an independent copy
assert(copy:equals(cl.mem.from_string("121234")))
assert(copy:compare(cl.mem.from_string("121235")) == -1)
assert(copy:compare(cl.mem.from_string("121234")) == 0)
assert(copy:compare(cl.mem.from_string("121233")) == 1)
assert(cl.mem.from_string("a"):compare(cl.mem.from_string("aa")) == -1)
assert(copy:find("12") == 0 and copy:find("23", 2) == 3)
assert(copy:find("missing") == nil and copy:find("", #copy) == #copy)
assert(copy:starts_with("121") and copy:ends_with("234"))
assert(copy:starts_with("") and copy:ends_with(""))
local reversed = cl.mem.from_string("abcd")
assert(reversed:reverse() == reversed and reversed:to_string() == "dcba")
must_fail(function() copy:find("1", #copy + 1) end)
must_fail(function() copy:find(123) end)
must_fail(function() copy:starts_with(123) end)
copy:clear()
assert(copy:to_string() == "\0\0\0\0\0\0")

-- Buffer validation and type checks.
must_fail(function() cl.mem.new(-1) end)
must_fail(function() cl.mem.new("6") end)
must_fail(function() cl.mem.new(1, 256) end)
must_fail(function() cl.mem.from_string(123) end)
must_fail(function() buffer:read(6) end)
must_fail(function() buffer:read(-1) end)
must_fail(function() buffer:read("0") end)
must_fail(function() buffer:write(0, 256) end)
must_fail(function() buffer:write(0, "1") end)
must_fail(function() buffer:fill(0, 5, 2) end)
must_fail(function() buffer:fill("0", 0, 1) end)
must_fail(function() buffer:to_string(7) end)
must_fail(function() buffer:to_string("0") end)
must_fail(function() buffer:slice(7) end)
must_fail(function() buffer:copy(5, buffer, 0, 2) end)
must_fail(function() buffer:copy(0, buffer, 5, 2) end)
must_fail(function() buffer:copy(0, "source", 0, 1) end)
must_fail(function() buffer:equals("not a buffer") end)

-- Every fixed-width integer primitive round-trips boundary values in both
-- its packed string and a cl.buffer source.
local integer_cases = {
    { "u8",     "read_u8",     0,          255 },
    { "i8",     "read_i8",    -128,        127 },
    { "u16le",  "read_u16le", 0,        65535 },
    { "u16be",  "read_u16be", 0,        65535 },
    { "i16le",  "read_i16le", -32768,    32767 },
    { "i16be",  "read_i16be", -32768,    32767 },
    { "u32le",  "read_u32le", 0,   0xFFFFFFFF },
    { "u32be",  "read_u32be", 0,   0xFFFFFFFF },
    { "i32le",  "read_i32le", -2147483648, 2147483647 },
    { "i32be",  "read_i32be", -2147483648, 2147483647 },
}
for _, test in ipairs(integer_cases) do
    local pack, read, minimum, maximum = test[1], test[2], test[3], test[4]
    for _, value in ipairs({ minimum, maximum }) do
        local encoded = cl.bin[pack](value)
        local width = pack:find("32") and 4 or pack:find("16") and 2 or 1
        assert(#encoded == width)
        assert(cl.bin[read](encoded) == value)
        assert(cl.bin[read](cl.mem.from_string("x" .. encoded), 1) == value)
    end
end
must_fail(function() cl.bin.u8(-1) end)
must_fail(function() cl.bin.u8("255") end)
must_fail(function() cl.bin.u8(256) end)
must_fail(function() cl.bin.i8(-129) end)
must_fail(function() cl.bin.i16be(32768) end)
must_fail(function() cl.bin.u32le(-1) end)
must_fail(function() cl.bin.read_u32le("123", 0) end)
must_fail(function() cl.bin.read_u16le("12", 2) end)
must_fail(function() cl.bin.read_u8("", 0) end)

-- IEEE floating-point primitives preserve endian-specific wire layouts and
-- round-trip ordinary Lua numbers within the precision of each format.
assert(cl.bin.f32le(1.0) == "\0\0\128\63")
assert(cl.bin.f32be(1.0) == "\63\128\0\0")
assert(math.abs(cl.bin.read_f32le(cl.bin.f32le(12.5)) - 12.5) < 0.0001)
assert(math.abs(cl.bin.read_f32be(cl.mem.from_string(cl.bin.f32be(-3.25))) + 3.25) < 0.0001)
assert(cl.bin.f64le(1.0) == "\0\0\0\0\0\0\240\63")
assert(cl.bin.f64be(1.0) == "\63\240\0\0\0\0\0\0")
assert(cl.bin.read_f64le(cl.bin.f64le(123456.25)) == 123456.25)
assert(cl.bin.read_f64be(cl.bin.f64be(-0.125)) == -0.125)
must_fail(function() cl.bin.read_f32le("123", 0) end)
must_fail(function() cl.bin.f32le("1.5") end)

-- Friendly byte helpers accept either strings or buffers and produce useful
-- application-facing representations.
local binary = cl.bytes.from_hex("00a1FF")
assert(cl.bytes.to_hex(binary) == "00A1FF")
assert(cl.bytes.to_hex("Lua\0") == "4C756100")
assert(cl.bytes.concat("ab", binary, cl.mem.from_string("cd")):to_string() == "ab\0\161\255cd")
assert(#cl.bytes.concat() == 0)
must_fail(function() cl.bytes.from_hex("ABC") end)
must_fail(function() cl.bytes.from_hex("GG") end)
must_fail(function() cl.bytes.to_hex(123) end)

-- CRC32 accepts both supported byte-source types and handles empty input.
assert(cl.bin.crc32("123456789") == 0xCBF43926)
assert(cl.bin.crc32(cl.mem.from_string("123456789")) == 0xCBF43926)
assert(cl.bin.crc32("") == 0)
must_fail(function() cl.bin.crc32(123) end)

-- Bit operations and platform-width shift/rotation boundaries.
assert(cl.bits.band(0xF0, 0x3C) == 0x30)
assert(cl.bits.bor(0xF0, 0x0F) == 0xFF)
assert(cl.bits.bxor(0xAA, 0xFF) == 0x55)
assert(cl.bits.bnot(0) == -1)
assert(cl.bits.btest(0x05, 0) and not cl.bits.btest(0x05, 1))
assert(cl.bits.bset(0x04, 0) == 0x05)
assert(cl.bits.bclear(0x05, 0) == 0x04)
assert(cl.flags.has(0x05, 0) and not cl.flags.has(0x05, 1))
assert(cl.flags.set(0x04, 0) == 0x05)
assert(cl.flags.clear(0x05, 0) == 0x04)
assert(cl.bits.lshift(1, 0) == 1 and cl.bits.lshift(1, 8) == 256)
assert(cl.bits.rshift(256, 0) == 256 and cl.bits.rshift(256, 8) == 1)
assert(cl.bits.rol(0x12, 4) == 0x120 and cl.bits.popcount(0xF0F0) == 8)
local integer_width = 8 * string.packsize("j")
assert(cl.bits.rol(1, integer_width) == 1)
assert(cl.bits.ror(1, integer_width) == 1)
assert(cl.bits.rol(1, -1) == cl.bits.ror(1, 1))
assert(cl.bits.ror(1, -1) == cl.bits.rol(1, 1))
must_fail(function() cl.bits.lshift(1, -1) end)
must_fail(function() cl.bits.lshift(1, integer_width) end)
must_fail(function() cl.bits.rshift(1, integer_width) end)
must_fail(function() cl.bits.band("1", 1) end)
must_fail(function() cl.bits.bnot("1") end)
must_fail(function() cl.bits.popcount("1") end)
must_fail(function() cl.bits.btest(1, integer_width) end)
must_fail(function() cl.bits.bset(1, -1) end)
must_fail(function() cl.flags.has(1, integer_width) end)

assert(cl.version() == "0.5.0")
print("cl tests passed")
