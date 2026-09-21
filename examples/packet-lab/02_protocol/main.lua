package.path = "./?.lua;./lib/?.lua;" .. package.path

local cl = require("cl")
local hex = require("hex")
local packet = require("packet")

local message = packet.encode({
    sequence = 42,
    flags = packet.make_flags({ calibrated = true }),
    samples = { -12, 3, 17, 21 },
})
local decoded = packet.decode(message)

print("encoded bytes:", #message)
print("wire:", hex.encode(message:to_string()))
print("sequence:", decoded.sequence)
print("sample count:", #decoded.samples)
print("checksum:", ("0x%08X"):format(decoded.checksum))
assert(cl.bin.crc32(message) ~= 0)
