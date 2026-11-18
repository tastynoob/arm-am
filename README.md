# arm-am

AArch64 bare-metal AM runtime and small tests for QEMU `mini-virt`
(`-M mini-virt,platform=bm`, console UART at `0x13000000`).

## Apps

| App | What it does |
| --- | --- |
| `hello` | Prints a fixed hello message; smoke test for console output and exit |
| `minibench` | Integer loop and small-buffer memory loop with cycle counts |
| `packedtest` | Checks packed 32-bit register allocation with a 64-bit add |
| `microtest` | Six independent minibench images: ALU, scalar/pair L1 load and store, and cmp+branch throughput (one `.bin` per test) |
| `mem_test_latency` | Pointer-chasing memory latency test with cache-flush footprint |
| `mem_test_bw` | Strided/unrolled memory bandwidth test (`mem` or `cache`) |

Each app directory has a short `README.md` describing its test and run command.

## Build and run

```sh
cp config.local.mk.example config.local.mk   # adjust QEMU/toolchain paths
make list
make APP=hello run
make APP=mem_test_latency run
make APP=mem_test_bw TEST_TYPE=mem run
make microtest               # build six .bin files and run each on QEMU
make microtest TEST=ld       # just the ld image
make microtest-gem5          # run each .bin on gem5
make cputest
```

QEMU `mini-virt` accepts the raw `.bin` image produced by `make`, not the ELF.
`make run` already passes the `.bin`.  The gem5 `--bare-metal` path in the
current KMHv3 tree also consumes the raw `.bin` (`--generic-arm-cpt
--raw-cpt`); `make microtest-gem5` handles that.
