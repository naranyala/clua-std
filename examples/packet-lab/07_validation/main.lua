package.path = "./?.lua;./lib/?.lua;" .. package.path

local cl = require("cl")
local packet = require("packet")

local function rejected(source)
    local ok = pcall(function() packet.decode(source) end)
    return not ok
end

local valid = packet.encode({ sequence = 7, flags = 0, samples = { 1, 2, 3 } })
assert(not rejected(valid))

local bad_magic = cl.mem.from_string(valid:to_string())
bad_magic:write(0, 0x00)
assert(rejected(bad_magic))

local bad_checksum = cl.mem.from_string(valid:to_string())
bad_checksum:write(#bad_checksum - 1, cl.bits.bxor(bad_checksum:read(#bad_checksum - 1), 0x01))
assert(rejected(bad_checksum))

local trailing = cl.bytes.concat(valid, "\0")
assert(rejected(trailing))

print("valid packet accepted; malformed packets rejected")
