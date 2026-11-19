# throughput

Instruction-throughput microbenchmarks, grouped as one app/test set.  The set
shares [app.mk](app.mk), this README, and one build entry point
(`make APP=throughput`); each subdirectory only contains `main.c` and produces its
own `.bin`.  `TEST=<name>` selects one test.

| Test | What it measures |
| --- | --- |
| `alu` | 64 independent adds per iteration; ALU throughput. |
| `chain_alu` | `CHAINS` dependent add chains; scheduling and ALU latency. |
| `cmpbr` | 32 not-taken `cmp; b.ne` pairs; branch throughput and fusion. |
| `ld` | 64 scalar `ldr` over 512 B; L1 load throughput. |
| `st` | 64 scalar `str` over 512 B; L1 store throughput. |
| `ldp` | 64 `ldp` over 1 KiB; pair-load throughput. |
| `stp` | 64 `stp` over 1 KiB; pair-store throughput. |

Common parameters from `app.mk`: `ITERS` (default 100000), and `CHAINS`
(`chain_alu` only, default 4).

```sh
make APP=throughput run                   # all seven tests
make APP=throughput TEST=alu run          # one test
make APP=throughput TEST=chain_alu CHAINS=1 run
make APP=throughput TEST=chain_alu gem5-run
```

Images: `build/arm64-minivirt/throughput/<name>.bin` (all tests of the set
share the set build directory).
