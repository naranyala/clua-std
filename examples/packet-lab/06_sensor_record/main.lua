package.path = "./?.lua;./lib/?.lua;" .. package.path

local cl = require("cl")
local hex = require("hex")

-- A compact calibration record: sensor id, temperature, and scale factor.
local flags = cl.bits.bset(0, 0) -- calibrated
flags = cl.bits.bset(flags, 3)   -- factory profile
local record = cl.mem.from_string(table.concat({
    cl.bin.u16be(0x1042),
    cl.bin.u8(flags),
    cl.bin.f32le(23.75),
    cl.bin.f64be(0.9875),
}))

print("record:", hex.encode(record:to_string()))
print("sensor id:", cl.bin.read_u16be(record, 0))
print("calibrated:", cl.bits.btest(cl.bin.read_u8(record, 2), 0))
print("factory profile:", cl.bits.btest(cl.bin.read_u8(record, 2), 3))
print("temperature:", cl.bin.read_f32le(record, 3))
print("scale:", cl.bin.read_f64be(record, 7))
assert(cl.bin.read_u16be(record, 0) == 0x1042)
