package.path = "./?.lua;./lib/?.lua;" .. package.path

local packet = require("packet")

local flags = packet.make_flags({
    alert = true,
    calibrated = true,
    battery_low = true,
})

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
assert(not packet.has_flag(flags, packet.FLAG.COMPRESSED))
