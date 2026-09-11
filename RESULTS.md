# Design and validation notes

## Scope

ImplicitNet defines a deterministic implicit network for `N = 2^w` nodes, where `4 <= N <= 65536`. The current state is a 64-bit value. The implementation calculates relationships directly and stores no graph edges.

The public API consists of a state-derived identifier bijection, `w` neighbor dimensions, a deterministic routing step, and reversible event updates. It is intended as a compact experimental primitive, not as a cryptographic construction or a claim of optimal routing.

## Implementation properties

- No heap allocation, recursion, or mutable global state
- C17 implementation with unsigned integer arithmetic
- `in_map` and `in_unmap` use a state-derived coordinate permutation plus an XOR mask
- `in_neighbor(x, d, N, state)` is an involution for valid input: applying the same direction twice returns `x`
- `in_step` XORs an event-derived mixed value; therefore `in_prev` is the same operation and reverses it for the same event and network size

## Validation included in this repository

`c_test.c` checks all nodes across supported sizes, a corpus of fixed and single-bit states, inverse mapping identities, neighbor range/symmetry/distinctness properties, state reversibility, and invalid-input handling.

`test_implicit_net.py` is a Python reference-oracle and randomized cross-check harness. It verifies `in_neighbor` and `in_next` against the reference implementation for 100,000 generated vectors, then evaluates routing behaviour on sampled cases.

Run the C validation with CMake:

```sh
cmake -S . -B build
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

Benchmark figures are intentionally not committed here: timings are hardware-, compiler-, and optimization-dependent. The C test executable prints its own approximate CPU-time measurements after the invariant suite completes.
