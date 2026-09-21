package.path = "./?.lua;./lib/?.lua;" .. package.path

local cl = require("cl")
local packet = require("packet")

local packets = {
    { sequence = 100, samples = { 10, 12, 11, 13 }, flags = 0 },
    { sequence = 101, samples = { 15, 18, 16, 17 }, flags = packet.FLAG.ALERT },
    { sequence = 102, samples = { 8, 9, 7, 10 }, flags = 0 },
}

local all_samples = {}
for _, item in ipairs(packets) do
    local decoded = packet.decode(packet.encode(item))
    for _, sample in ipairs(decoded.samples) do
        all_samples[#all_samples + 1] = sample
    end
end

local stats = cl.math.stats(all_samples)
print(("samples=%d mean=%.2f min=%g max=%g"):format(
    stats.count, stats.mean, stats.min, stats.max
))
assert(stats.count == 12)
