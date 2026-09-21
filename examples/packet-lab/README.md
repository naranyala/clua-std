# Packet Lab

`packet-lab` is a small telemetry protocol implemented entirely in Lua on top
of `clua-std`. It models sensor readings as a binary packet with:

```text
magic       u16be   0xCAFE
version     u8      1
flags       u8      bit flags
sequence    u32le   packet sequence number
count       u16le   number of signed samples
samples     i16le[] sensor values
checksum    u32be   CRC32 of every preceding byte
```

The project demonstrates a useful division of labor:

- `cl.mem` owns mutable packet storage;
- `cl.bin` encodes and decodes wire values;
- `cl.bits` manages compact flags;
- `cl.bin.crc32` detects corruption;
- `cl.math.stats` summarizes decoded samples.

## Run it

Build the module from the repository root, then run these commands from this
directory:

```sh
export LUA_CPATH="$(pwd)/../../build/lua/?.so;;"
lua 01_buffers/main.lua
lua 02_protocol/main.lua
lua 03_flags/main.lua
lua 04_metrics/main.lua
lua 05_end_to_end/main.lua
```

The examples use `lib/` through the local Lua package path configured in each
script. If your module uses another suffix, adjust `LUA_CPATH` accordingly.

## Layout

- `lib/hex.lua` — binary-string display helper.
- `lib/packet.lua` — packet encoder, decoder, flags, and checksum validation.
- `01_buffers/` — mutable storage and safe range operations.
- `02_protocol/` — packet construction and round-trip decoding.
- `03_flags/` — integer bit flags and readable flag names.
- `04_metrics/` — native statistics over decoded sample data.
- `05_end_to_end/` — full encode, inspect, validate, decode, and summarize
  workflow.
