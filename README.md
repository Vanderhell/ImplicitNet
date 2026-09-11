# ImplicitNet

ImplicitNet is a small, allocation-free C17 library for deterministic implicit networks over power-of-two node sets. It computes mappings, dimension-based neighbors, routing steps, and reversible state changes directly from a 64-bit state—without materialising a graph.

It is a compact building block for simulations, constrained systems, and experiments where predictable memory use matters.

## Features

- C17, no dependencies, heap allocations, recursion, or mutable global state
- Supported network sizes: `N = 2^w`, from 4 through 65,536
- State-derived bijective `map` / `unmap` operations
- `log2(N)` implicit neighbor dimensions
- Deterministic next-hop routing and reversible state events
- C invariant tests and a Python reference-oracle cross-check

## Build and test

Requirements: CMake 3.20+ and a C17 compiler. Python 3 is optional for the oracle test.

```sh
cmake -S . -B build
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

The shared library and `implicit_net_c_test` executable are emitted under the selected build directory. On single-configuration generators, omit `--config Release` and `-C Release`.

## Minimal example

```c
#include "implicit_net.h"
#include <stdint.h>

int main(void) {
    in_state state;
    const uint32_t node_count = 1024;

    if (!in_init(&state, node_count, UINT64_C(0x1234))) return 1;

    uint32_t neighbor = in_neighbor(17, 3, node_count, state);
    uint32_t next_hop = in_next(17, 900, node_count, state);
    return (neighbor == UINT32_MAX || next_hop == UINT32_MAX);
}
```

## API and guarantees

All APIs take a valid network size: a power of two in `[4, 65536]`. Value-returning APIs use `UINT32_MAX` for invalid input; state-changing APIs return `0` on failure and `1` on success.

| Function | Purpose |
| --- | --- |
| `in_init` | Initializes an `in_state` from a seed. |
| `in_map` / `in_unmap` | State-derived inverse identifier bijections. |
| `in_neighbor` | Returns the neighbor for a valid dimension. |
| `in_next` | Returns the deterministic next hop toward a target. |
| `in_step` / `in_prev` | Applies or reverses an event-derived state update. |

For a valid node and direction, `in_neighbor` is a fixed-point-free involution: calling it again with the same direction returns the original node. `in_map` and `in_unmap` are inverses for valid inputs. `in_prev` reverses `in_step` when invoked with the same network size and event.

## Repository layout

| Path | Description |
| --- | --- |
| `implicit_net.h` | Public C API. |
| `implicit_net.c` | Implementation. |
| `c_test.c` | C invariants and microbenchmark harness. |
| `oracle.py` | Python reference implementation. |
| `test_implicit_net.py` | Randomized C/Python consistency and routing checks. |
| `RESULTS.md` | Scope, design notes, and validation record. |

## Status and scope

ImplicitNet is an experimental library. It provides a deterministic, compact family of state-configured implicit networks; it does not claim cryptographic security or universal optimal routing. Please validate it for your workload before production use.

## License

Released under the [MIT License](LICENSE).
