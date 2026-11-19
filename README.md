# arm-am

AArch64 bare-metal AM runtime and small tests for QEMU `mini-virt`
(`-M mini-virt,platform=bm`, console UART at `0x13000000`).

## Apps

| App | What it does |
| --- | --- |
| `hello` | Prints a fixed hello message; smoke test for console output and exit |
| `minibench` | Integer loop and small-buffer memory loop with cycle counts |
| `packedtest` | Checks packed 32-bit register allocation with a 64-bit add |
| `throughput` | Instruction-throughput test set (ALU, chained ALU, cmp+branch, scalar/pair L1 load/store); one `.bin` per test, shared build rules and README (`throughput/README.md`) |
| `stream_ld` | Streaming load bandwidth over a configurable footprint and pass count |
| `chain_ld` | Pointer-chasing memory latency over a configurable footprint |
| `ld_use_st` | Same-line load-use-store RMW chain |
| `mem_test_latency` | Pointer-chasing memory latency test with cache-flush footprint |
| `mem_test_bw` | Strided/unrolled memory bandwidth test (`mem` or `cache`) |

An app is a test set.  Every test produces one `.bin`, and `TEST=<name>`
selects a test in any set.  A one-test app such as `hello` is simply a test set
with one test (named after the app); it follows exactly the same build rule as
`throughput`.  All `.bin`s of a set are written directly into its build directory,
e.g. `build/arm64-minivirt/throughput/alu.bin`.  Each test set has one
`README.md`; its test list, source layout and parameters live in
`apps/<app>/app.mk`.

## Build and run

```sh
cp config.local.mk.example config.local.mk   # adjust QEMU/toolchain paths
make list
make APP=hello run
make APP=mem_test_latency run
make APP=mem_test_bw TEST_TYPE=mem run
make APP=throughput run      # all instruction-throughput tests
make APP=throughput TEST=alu run   # one throughput test
make APP=stream_ld FOOTPRINT=64M PASSES=2 run
make APP=stream_ld FOOTPRINT=64M PASSES=2 gem5-run
make cputest
```

QEMU `mini-virt` accepts the raw `.bin` image produced by `make`, not the ELF.
`make run` already passes the `.bin`.  The gem5 `--bare-metal` path in the
current KMHv3 tree also consumes the raw `.bin` (`--generic-arm-cpt
--raw-cpt`); `make APP=... gem5-run` handles that.
