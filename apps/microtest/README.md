# microtest

AArch64 microbenchmarks for microarchitectural exploration.  The suite builds
**one independent bare-metal image per test**, i.e. six separate `.bin` files
from `make microtest-build`; there is no combined image.  Each `.bin` can be
handed to QEMU or gem5 on its own, so a test never shares a process, a
warm-up, or a PMU counting window with another test.

Load and store throughput is measured separately: `ld`/`st` for scalar
accesses and `ldp`/`stp` for pair accesses.  Nothing is mixed except `cmpbr`,
where the compare+branch pair is the unit under test.

## Tests and images

| Test | Loop body per iteration | ops/iter | bytes/iter | Image |
| --- | --- | --- | --- | --- |
| `alu` | 64 independent 64-bit `add` | 64 | - | `microtest/alu/alu.bin` |
| `ld` | 64 scalar `ldr` over 8 cache lines | 64 | 512 | `microtest/ld/ld.bin` |
| `st` | 64 scalar `str` over 8 cache lines | 64 | 512 | `microtest/st/st.bin` |
| `ldp` | 64 `ldp` pair loads over 16 cache lines | 64 | 1024 | `microtest/ldp/ldp.bin` |
| `stp` | 64 `stp` pair stores over 16 cache lines | 64 | 1024 | `microtest/stp/stp.bin` |
| `cmpbr` | 32 not-taken `cmp; b.ne` pairs | 32 | - | `microtest/cmpbr/cmpbr.bin` |

The image paths are relative to `build/arm64-minivirt/`; the matching `.elf`
is in the same directory.

The loops are inline assembly so the compiler cannot remove or vectorise the
operations.  Each loop body is followed by exactly two loop-control
instructions (`subs` + `b.ne`); `ops/cyc` uses the body op count only, while
`ipc` comes from the retired-instruction counter and includes that overhead.

The default trip count is 100,000; override it with `ITERS=<n>`.  The 8 KiB
buffer is filled and read back before each memory test (`bench_warm_buffer`),
so the scalar tests' 512 B window and the pair tests' 1 KiB window are L1
resident.  The pair tests centre their base pointer in the window because the
`ldp`/`stp` immediate range is only -512..504.

## Build and run

```sh
make microtest-build              # build all six independent .bin files
make microtest                    # QEMU: one process per .bin
make microtest TEST=ld            # build and run only ld
make microtest-gem5               # gem5: one process per .bin
make microtest-gem5 TEST=ld       # only ld on gem5
make microtest ITERS=10000        # shorter run
```

`TEST` filters both the build and the run.  The test list is discovered from
`tests/*.c`, so adding a benchmark does not require editing suite.mk.

The current KMHv3 gem5 bare-metal interface loads the flat `.bin`, not the
ELF, so `microtest-gem5` drives the generic `gem5-run` target, which passes
`--raw-cpt --bare-metal --generic-arm-cpt <bin>`.  When `DIFFTEST_REF_SO`
exists (see `config.local.mk.example`), it also adds `--enable-difftest
--difftest-ref-so <so>`.  Set `GEM5_HOME` and `DIFFTEST_REF_SO` in
`config.local.mk` or on the command line.

A single image can also be run directly:

```sh
cd ~/workspace/GEM5-kmhv3-aarch64
./build/ARM/gem5.fast -d /tmp/m5out-ld \
    configs/example/idealkmhv3_arm.py --raw-cpt --bare-metal \
    --generic-arm-cpt ~/workspace/arm-am/build/arm64-minivirt/microtest/ld/ld.bin \
    --enable-difftest \
    --difftest-ref-so ~/workspace/qemu/build/libqemu-aarch64-difftest.so
```

## Output

```text
alu    cycles=1320404 instret=6600036 ops/cyc=4.84 cyc/op=0.20 ipc=4.99
ld     cycles=2133561 instret=6600030 ops/cyc=2.99 cyc/op=0.33 ipc=3.09 B/cyc=23.99
```

- `cycles`: `PMCCNTR_EL0` (or `CNTVCT_EL0` when no PMU is exposed).
- `instret`: `INST_RETIRED` (event `0x08`) when the implementation provides
  it; 0 on QEMU, which does not expose that event.
- `ops/cyc`, `cyc/op`: body operations per cycle and cycles per body op.
- `ipc`: retired instructions per cycle, including loop control.
- `B/cyc`: bytes moved per cycle for the memory tests.

QEMU does not model caches or PMU events: its `instret` is always 0 and its
`cycles` are only a simulation proxy.  Use gem5 (or RTL) for meaningful
throughput numbers; the difftest reference model validates that the loops
compute what they should.

## Example results

Current KMHv3 ideal gem5 platform with difftest enabled, `ITERS=100000`, one
run per image.  These are examples, not guarantees: they depend on the CPU
and cache parameters.

| Test | cycles | instret | ops/cyc | cyc/op | ipc | B/cyc |
| --- | --- | --- | --- | --- | --- | --- |
| `alu` | 1,320,404 | 6,600,036 | 4.84 | 0.20 | 4.99 | - |
| `ld` | 2,133,561 | 6,600,030 | 2.99 | 0.33 | 3.09 | 23.99 |
| `st` | 3,200,396 | 6,600,038 | 1.99 | 0.50 | 2.06 | 15.99 |
| `ldp` | 2,133,865 | 6,600,035 | 2.99 | 0.33 | 3.09 | 47.98 |
| `stp` | 3,200,398 | 6,600,051 | 1.99 | 0.50 | 2.06 | 31.99 |
| `cmpbr` | 1,101,273 | 6,600,030 | 2.90 | 0.34 | 5.99 | - |

What the split exposes on this model:

- The pure load loop sustains ~3 loads/cycle and the pure store loop ~2
  stores/cycle.  The old mixed `ldst` loop reported 3.68 ops/cycle because
  loads and stores overlapped; that number hid the store-path limit.
- `ldp` and `stp` have the same per-instruction rate as `ldr`/`str`
  (~3 loads/cycle, ~2 stores/cycle), but move twice the bytes per
  instruction.
- `cmpbr` retires 6,600,030 instructions, i.e. 66 per iteration, so the
  model's `INST_RETIRED` shows no compare+branch fusion effect (full fusion
  would report about 34 per iteration).  The `instret` column is the
  diagnostic here.
- The ALU loop reaches 4.84 adds/cycle (IPC 4.99), the sustained integer ALU
  limit of this model configuration.

## Adding a test

Drop a new `<name>.c` into `tests/`; suite.mk discovers it and builds
`build/arm64-minivirt/microtest/<name>/<name>.bin`, so no build-file edit is
needed:

```c
#include "bench.h"

static uint64_t __attribute__((noinline))
my_bench(uint64_t iters)
{
    uint64_t result;
    __asm__ volatile(/* timed loop */ : [out] "=r"(result)
                     : [iters] "r"(iters) : "cc", "memory");
    return result;
}

int main(void)
{
    bench_banner("my");
    bench_warm_buffer();          /* only for memory tests */
    bench_run("my", my_bench, /* ops/iter */ 64, /* bytes/iter */ 512);
    return 0;
}
```

`bench_run` starts the PMU window around one call, prints the result line,
and emits the `microtest <name>: PASS sink=...` marker.  Keep the total
retired instruction count below 2^32 for the 32-bit `instret` counter
(the defaults are about 6.6 million); the cycle counter is 64-bit.
