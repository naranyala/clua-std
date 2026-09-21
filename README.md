# clua-std

`clua-std` is a small native standard library for Lua. It exposes safe,
low-level data operations through a compact `cl` module: Lua provides the
ergonomic surface, while C provides native byte storage and computational
primitives.

```lua
local cl = require("cl")

local packet = cl.mem.new(8)
packet:write(0, 0xCA):write(1, 0xFE)

local header = cl.bin.u16le(0xBEEF)
assert(cl.bin.read_u16le(header) == 0xBEEF)

local summary = cl.math.stats({ 4, 8, 15, 16, 23, 42 })
print(summary.mean)
```

## What it provides

- `cl.mem` — mutable native byte buffers with bounds-checked access, copying,
  slicing, comparison, and conversion to Lua strings.
- `cl.bytes` — friendly hex conversion and concatenation for strings and
  buffers.
- `cl.bin` — fixed-width signed and unsigned 8/16/32-bit integer packing and
  reading, IEEE 754 `f32`/`f64` values, little- or big-endian order, and CRC-32.
- `cl.math` — native statistics for non-empty numeric Lua arrays.
- `cl.flags` — readable bit flag operations using bit indexes.
- `cl.bits` — integer bitwise operations, bit testing/set/clear, logical
  shifts, rotations, and population count.

All byte offsets are zero-based. Invalid ranges, integer values, source
lengths, and shift counts raise Lua errors instead of allowing unchecked
memory operations.

## Quick start

Requirements: Lua 5.3+ with a 64-bit integer type, Lua development headers
and a shared or PIC-compatible Lua library, a C11 compiler, and CMake 3.20+.

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

Run the example program:

```sh
LUA_CPATH="$(pwd)/build/lua/?.so;;" \
  lua lua/main.lua 4 8 15 16 23 42
```

The module is built under `build/lua/`. If CMake finds a static Lua archive,
that archive must have been compiled with position-independent code in order
to link the loadable module.

## Documentation

Start at [docs/README.md](docs/README.md), then see:

- [API reference](docs/api.md)
- [Getting started](docs/getting-started.md)
- [Architecture](docs/architecture.md)
- [Testing](docs/testing.md)
- [Build and portability](docs/build-and-portability.md)

Project direction is defined in [PYRAMID-OF-INTENTS.md](PYRAMID-OF-INTENTS.md),
and planned work is tracked in [TODOS.md](TODOS.md).

## Project status

The API and tests are actively being developed. The current environment can
compile the C sources, but its non-PIC static Lua library prevents linking
`cl.so`; see [Testing](docs/testing.md) for the diagnosis and workaround.
