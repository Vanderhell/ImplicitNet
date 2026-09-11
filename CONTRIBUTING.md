# Contributing

Thanks for your interest in improving ImplicitNet.

## Development workflow

1. Keep the public API in `implicit_net.h` backwards compatible unless a breaking change is explicitly documented.
2. Format new C code consistently with the surrounding source and use C17.
3. Build and run the invariant suite before opening a pull request:

   ```sh
   cmake -S . -B build
   cmake --build build --config Release
   ctest --test-dir build -C Release --output-on-failure
   ```

4. Add or update tests for behavioural changes.

Please avoid committing generated binaries, build directories, or benchmark output.
