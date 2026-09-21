package.path = "./?.lua;./lib/?.lua;" .. package.path

local cl = require("cl")
local hex = require("hex")
local packet = require("packet")

local outgoing = packet.encode({
    sequence = 9001,
    flags = packet.make_flags({ alert = true, battery_low = true }),
    samples = { 22, 23, 21, 25, 24, 26 },
})

print("1. device produced packet")
print("   bytes:", #outgoing)
print("   hex:", hex.encode(outgoing:to_string()))
print("   crc32:", ("0x%08X"):format(cl.bin.crc32(outgoing)))

local received = packet.decode(outgoing)
local summary = cl.math.stats(received.samples)
print("2. gateway decoded packet")
print("   sequence:", received.sequence)
print("   alert:", packet.has_flag(received.flags, packet.FLAG.ALERT))
print("   battery low:", packet.has_flag(received.flags, packet.FLAG.BATTERY_LOW))
print(("3. gateway summarized %d samples: mean=%.2f min=%g max=%g"):format(
    summary.count, summary.mean, summary.min, summary.max
))

local corrupted = cl.mem.from_string(outgoing:to_string())
corrupted:write(10, cl.bits.bxor(corrupted:read(10), 0x01))
local ok = pcall(function() packet.decode(corrupted) end)
print("4. corrupted packet rejected:", not ok)
assert(not ok)
