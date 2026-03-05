# double_buf_run

Small runner to compare double-buffer behaviors with two pinned threads:
- writer thread writes a payload of 128 cache lines as fast as possible
- reader thread reads once per second

## Build

```bash
cmake --build build-debug --target dbuf_run -j
```

## Run

```bash
./build-debug/apps/double_buf_run/dbuf_run [writer_cpu reader_cpu duration_sec mode]
```

Defaults:
- `writer_cpu=0`
- `reader_cpu=1`
- `duration_sec=10`
- `mode=compare`

Examples:

```bash
./build-debug/apps/double_buf_run/dbuf_run 0 1 10 latest
./build-debug/apps/double_buf_run/dbuf_run 0 1 10 strict
./build-debug/apps/double_buf_run/dbuf_run 0 1 10 initial
./build-debug/apps/double_buf_run/dbuf_run 0 1 10 compare
```

## Modes

- `latest`: writer always publishes latest snapshot (reader can miss updates)
- `strict`: strict handoff semantics (writer waits for reader consumption)
- `initial`: original implementation kept for comparison
- `compare`: runs all modes sequentially and prints a summary
