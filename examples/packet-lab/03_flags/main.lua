package.path = "./?.lua;./lib/?.lua;" .. package.path

local packet = require("packet")
local cl = require("cl")

local flags = packet.make_flags({
    alert = true,
    calibrated = true,
    battery_low = true,
})
flags = cl.flags.set(flags, 3) -- add COMPRESSED using a bit index

local names = {
    { "ALERT", packet.FLAG.ALERT },
    { "CALIBRATED", packet.FLAG.CALIBRATED },
    { "BATTERY_LOW", packet.FLAG.BATTERY_LOW },
    { "COMPRESSED", packet.FLAG.COMPRESSED },
}

print(("packed flags: 0x%02X"):format(flags))
for _, item in ipairs(names) do
    print(item[1], packet.has_flag(flags, item[2]) and "on" or "off")
end

assert(packet.has_flag(flags, packet.FLAG.ALERT))
assert(packet.has_flag(flags, packet.FLAG.COMPRESSED))
