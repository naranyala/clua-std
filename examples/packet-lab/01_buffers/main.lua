package.path = "./?.lua;./lib/?.lua;" .. package.path

local cl = require("cl")
local hex = require("hex")

local frame = cl.mem.new(12, 0)
frame:write(0, 0xCA):write(1, 0xFE)
frame:fill(0xA5, 2, 4)
local header = frame:slice(0, 6)

print("buffer:", tostring(frame), "size=" .. #frame)
print("header:", hex.encode(header:to_string()))
print("crc32:", ("0x%08X"):format(cl.bin.crc32(frame)))

local copy = cl.mem.from_string(frame:to_string())
copy:copy(6, header, 0, #header)
print("copied:", hex.encode(copy:to_string()))
print("equal:", frame:equals(copy))
print("header position:", copy:find(header))
print("has CAFE prefix:", copy:starts_with("\202\254"))
print("has zero suffix:", copy:ends_with("\0\0\0\0\0\0"))
print("reversed copy:", hex.encode(copy:slice(0):reverse():to_string()))
