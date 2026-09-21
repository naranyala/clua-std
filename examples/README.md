# Examples

The examples are small integration projects built on top of `clua-std`. They
are intentionally closer to real usage than to unit tests: each one combines
several `cl` namespaces and prints a result a developer can inspect.

## Projects

- [`packet-lab`](packet-lab/README.md) — a miniature telemetry packet project
  with buffers, binary formats, bit flags, CRC32, and statistics.

The examples expect the native module to have been built first:

```sh
cmake -S . -B build
cmake --build build
```
