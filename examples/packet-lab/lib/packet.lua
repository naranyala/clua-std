local cl = require("cl")

local M = {}

M.MAGIC = 0xCAFE
M.VERSION = 1
M.FLAG = {
    ALERT = 0x01,
    CALIBRATED = 0x02,
    BATTERY_LOW = 0x04,
    COMPRESSED = 0x08,
}

local function pack_samples(samples)
    local encoded = {}
    for i, sample in ipairs(samples) do
        encoded[i] = cl.bin.i16le(sample)
    end
    return table.concat(encoded)
end

function M.make_flags(options)
    local flags = 0
    if options.alert then flags = cl.bits.bor(flags, M.FLAG.ALERT) end
    if options.calibrated then flags = cl.bits.bor(flags, M.FLAG.CALIBRATED) end
    if options.battery_low then flags = cl.bits.bor(flags, M.FLAG.BATTERY_LOW) end
    if options.compressed then flags = cl.bits.bor(flags, M.FLAG.COMPRESSED) end
    return flags
end

function M.has_flag(flags, flag)
    return cl.bits.band(flags, flag) ~= 0
end

function M.encode(packet)
    assert(#packet.samples <= 65535, "too many samples")
    local body = table.concat({
        cl.bin.u16be(M.MAGIC),
        cl.bin.u8(M.VERSION),
        cl.bin.u8(packet.flags or 0),
        cl.bin.u32le(packet.sequence),
        cl.bin.u16le(#packet.samples),
        pack_samples(packet.samples),
    })
    return cl.mem.from_string(body .. cl.bin.u32be(cl.bin.crc32(body)))
end

function M.decode(source)
    local buffer = cl.mem.is_buffer(source) and source or cl.mem.from_string(source)
    local offset = 0
    local function read(name)
        local value = cl.bin["read_" .. name](buffer, offset)
        offset = offset + ({ u8 = 1, u16be = 2, u16le = 2, u32le = 4, u32be = 4, i16le = 2 })[name]
        return value
    end

    local magic = read("u16be")
    assert(magic == M.MAGIC, "invalid packet magic")
    local version = read("u8")
    assert(version == M.VERSION, "unsupported packet version")
    local flags = read("u8")
    local sequence = read("u32le")
    local count = read("u16le")
    local samples = {}
    for i = 1, count do samples[i] = read("i16le") end

    local checksum_offset = offset
    local expected = read("u32be")
    assert(offset == #buffer, "trailing packet data")
    local actual = cl.bin.crc32(buffer:to_string(0, checksum_offset))
    assert(actual == expected, "packet checksum mismatch")

    return {
        flags = flags,
        sequence = sequence,
        samples = samples,
        checksum = expected,
    }
end

return M
