# Extending `clua-std`

New APIs should earn their place by strengthening the project's north-star
intent: safe, predictable, portable low-level operations for Lua.

## Admission criteria

A proposed function or namespace should satisfy all of these conditions:

1. **Concrete use case** — it solves a recurring binary-data, buffer, numeric,
   or systems-adjacent problem demonstrated by an example or consumer.
2. **Namespace fit** — its name and argument order follow an existing family
   such as `read_u16le`, rather than introducing a one-off convention.
3. **Explicit contract** — indexing, ranges, signedness, ownership, errors,
   and platform behavior are documented before implementation.
4. **Safe binding** — Lua values are checked at the C boundary and no raw
   pointer or unchecked shift behavior is exposed.
5. **Tests** — normal, boundary, invalid-input, and relevant cross-platform
   cases are covered.
6. **Example value** — the API is used in an integration example or clearly
   improves an existing one.
7. **Maintenance cost** — the feature does not duplicate Lua's standard
   library or create an abstraction that the project cannot support across its
   target platforms.

## Implementation sequence

1. Add or update the intent and API contract.
2. Add a core algorithm to `csrc/core.c` when it is independent of Lua.
3. Add binding validation and registration in `csrc/cl.c`.
4. Add core tests and Lua binding tests.
5. Add or update an integration example.
6. Update `docs/api.md`, `README.md`, and `TODOS.md`.
7. Run the strict build and CTest suite.

## Rejection signals

Defer a proposal when it is primarily a convenience alias, depends on one
operating system, needs undocumented lifetime behavior, or cannot be tested
without relying on a particular compiler or Lua implementation detail.
