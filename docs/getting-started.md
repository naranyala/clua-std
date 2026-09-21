# Getting started

## Requirements

You need:

- Lua 5.3 or newer with a 64-bit `lua_Integer` type;
- Lua development headers and a linkable Lua library;
- a C11 compiler;
- CMake 3.20 or newer.

The 64-bit Lua integer requirement is necessary for exact round-trips of all
unsigned 32-bit values.

## Build

From the repository root:

```sh
cmake -S . -B build
cmake --build build
```

The module is written to `build/lua/` as `cl.so` or the platform-equivalent
module file.

## Run tests

```sh
ctest --test-dir build --output-on-failure
```

The test executable loads the module built by the current CMake configuration.
See [Testing](testing.md) if configuration or linking fails before CTest can
run.

## Run the example

The example calculates statistics for command-line numbers:

```sh
LUA_CPATH="$(pwd)/build/lua/?.so;;" \
  lua lua/main.lua 4 8 15 16 23 42
```

Expected output is similar to:

```text
count=6 sum=108 mean=18 min=4 max=42
```

For platforms whose Lua installation uses another module suffix, replace
`?.so` with the suffix expected by that Lua installation.
