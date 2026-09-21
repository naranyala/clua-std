# Testing

## Test layers

The primary suite is [tests/test_cl.lua](../tests/test_cl.lua). It loads the
real module and validates the public contract, including:

- normal and invalid statistics inputs;
- buffer construction, mutation, ranges, copying, slicing, comparison, and
  ownership behavior;
- signed and unsigned integer round-trips at type boundaries;
- both endian modes and both accepted binary source types;
- CRC32 known values and empty input;
- bitwise, shift, rotation, and population-count behavior;
- expected Lua errors for invalid arguments.

Run it through CTest:

```sh
ctest --test-dir build --output-on-failure
```

The test intentionally uses `pcall` for error contracts so it does not depend
on exact Lua error wording.

## Validation expectations

When changing a public function:

1. Add a normal-case assertion.
2. Add a boundary assertion.
3. Add an invalid-input assertion when the function rejects input.
4. Test both string and buffer sources where both are supported.
5. Update [the API reference](api.md) if semantics change.

When changing a core algorithm, add or extend a known-value test and keep the
Lua binding test focused on conversion and boundary behavior.

## Current environment limitation

In the development environment used for this repository, CMake discovers
`/usr/local/lib64/liblua.a`, but that archive is not position-independent. C
compilation succeeds; linking `cl.so` fails with relocation errors before
CTest can run. Use a shared Lua development library or rebuild Lua with PIC
enabled to execute the suite.

## Future test separation

The intent-driven backlog includes a future split between C-level tests for
`core.c` and Lua-level tests for the binding. Until then, the Lua suite is the
authoritative executable contract for the released surface.
