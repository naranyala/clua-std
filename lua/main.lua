-- Lua owns application flow; computational work remains in the C module.
local cl = require("cl")

local values = {}
for i = 1, #arg do
    values[i] = assert(tonumber(arg[i]), "arguments must be numbers")
end

if #values == 0 then
    io.stderr:write("usage: lua lua/main.lua NUMBER [NUMBER ...]\n")
    os.exit(2)
end

local result = cl.math.stats(values)
print(("count=%d sum=%g mean=%g min=%g max=%g"):format(
    result.count, result.sum, result.mean, result.min, result.max
))
