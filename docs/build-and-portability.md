# Build and portability

## CMake targets

The project defines one loadable module target:

- `cl` — builds `csrc/cl.c` and `csrc/core.c` as a Lua module.

CMake locates Lua with `find_package(Lua 5.3 REQUIRED)`, enables C11, and uses
strict warnings (`-Wall -Wextra -Wpedantic -Werror` on non-MSVC toolchains).
The module has no `lib` prefix and is placed in `build/lua/`.

## Lua linking

Because `cl` is a shared module, the Lua library used for linking must be:

- a shared library; or
- a static archive compiled with position-independent code (PIC).

CMake prints a diagnostic when it detects a static Lua archive. A typical
failure for a non-PIC archive looks like:

```text
relocation ... can not be used when making a shared object
```

That is a Lua installation/toolchain issue rather than a Lua API test failure.

## Module discovery

CTest derives `LUA_CPATH` from the built target's directory and suffix. Manual
Lua invocations need an equivalent path, for example:

```sh
LUA_CPATH="$(pwd)/build/lua/?.so;;" lua lua/main.lua 1 2 3
```

The suffix depends on the Lua/platform toolchain. Keep the module name `cl`
and the output directory stable so `require("cl")` remains predictable.

## Supported scope

The current portability target is Lua 5.3+ with 64-bit integers on common
Unix-like platforms. Windows and macOS support still require validation with
their native Lua development packages, compiler, and module naming rules.
