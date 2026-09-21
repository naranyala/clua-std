# Architecture

## Runtime model

The project is a dynamically loadable Lua C module. Lua code owns application
flow and calls `require("cl")`; the native module registers the `cl` table and
its namespaces when Lua loads it.

```text
Lua application
      |
      v
require("cl") -> cl.mem / cl.bin / cl.math / cl.bits
      |
      v
Lua/C binding layer: csrc/cl.c
      |
      v
Lua-independent algorithms: csrc/core.c
```

## Source responsibilities

- `csrc/core.h` defines the Lua-independent data structures and function
  contracts.
- `csrc/core.c` implements statistics, CRC32, and endian encoding helpers.
- `csrc/cl.c` owns Lua userdata, argument checking, namespace registration,
  integer primitive closures, and bit operations.
- `lua/main.lua` demonstrates Lua-side orchestration.
- `tests/test_cl.lua` exercises the public Lua API.
- `CMakeLists.txt` builds the loadable module and registers the Lua test.

## Buffer ownership

`cl.mem.new` and `cl.mem.from_string` allocate Lua userdata containing the
buffer header and bytes in one object. Lua's garbage collector owns that
object. `from_string`, `to_string`, and `slice` copy bytes; there are no
zero-copy views exposed to Lua. `copy` uses overlap-safe movement.

The binding passes temporary pointers into C algorithms only for the duration
of the call. No native function retains a pointer to a Lua string or buffer
after returning.

## Boundary rules

The binding layer is responsible for:

- converting Lua integers and numbers;
- checking sizes, offsets, lengths, and fixed-width ranges;
- validating buffer and string sources;
- converting C failures into Lua errors;
- preserving exact unsigned 32-bit values through the 64-bit Lua integer
  requirement.
- producing independent buffers from the friendly `cl.bytes` helpers.

Floating-point binary primitives assume the platform provides 32-bit IEEE
`float` and 64-bit IEEE `double`, which are enforced by compile-time checks.

The core layer does not depend on a `lua_State`, which keeps its algorithms
portable and suitable for separate C-level tests in the future.

## Compatibility assumptions

The module requires Lua 5.3 or newer and a 64-bit `lua_Integer`. It must be
linked against a shared Lua library or a static Lua library built with
position-independent code, because the output is a shared loadable module.
