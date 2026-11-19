# mix_width

Mixed-instruction throughput for the rename -> IQ dispatch path.  The body of
each test is a **fixed-seed pseudorandom permutation** of its instruction
classes, not blocks of one class and not a round-robin.  The order is
deterministic for reproducibility, but irregular enough to avoid obvious
8-ALU / 8-load patterns.

All body instructions are independent:

- ALU/FP/MUL destinations are only written, never read by another body op.
- Loads and stores use disjoint 4 KiB regions; no load-use or store-load pairs.
- Sources are constants; only WAW name dependences remain, which renaming
  removes.
- A mixed branch is a **not-taken** `cbz x0, 9f` (x0 is constant 1, target at
  the end of the body), so it does not perturb branch prediction or fetch.

| Test | Instruction mix | Body ops |
| --- | --- | ---: |
| `alu_fp` | 32 add + 32 fadd | 64 |
| `alu_ld` | 32 add + 32 ldr | 64 |
| `alu_st` | 32 add + 32 str | 64 |
| `alu_mul` | 32 add + 32 mul | 64 |
| `alu_fp_ld_st` | 16 each add/fadd/ldr/str | 64 |
| `alu_branch_ld_st` | 16 each add/cbz/ldr/str | 64 |
| `alu_fp_branch_ld_st` | 8 each add/fadd/cbz/ldr/str | 40 |
| `alu_mul_branch_ld_st` | 8 each add/mul/cbz/ldr/str | 40 |
| `all_types` | 8 each add/fadd/mul/cbz/ldr/str | 48 |

`ops/cyc` is body instructions per cycle; loop-control overhead is included in
`ipc` but not in `ops`.

```sh
make APP=mix_width run
make APP=mix_width TEST=alu_branch_ld_st gem5-run
```

Images: `build/arm64-minivirt/mix_width/<test>.bin`.

## KMHv3 gem5 data

Single-type baselines from `throughput`:

| Test | cycles | instret | ops/cyc |
| --- | ---: | ---: | ---: |
| `alu` | 1,320,404 | 6,600,036 | 4.84 |
| `ld` | 2,133,561 | 6,600,030 | 2.99 |
| `st` | 3,200,396 | 6,600,038 | 1.99 |

Irregular mixed tests (all 9 pass with difftest):

| Test | cycles | instret | ops/cyc | IPC |
| --- | ---: | ---: | ---: | ---: |
| `alu_fp` | 825,963 | 6,600,035 | 7.74 | 7.99 |
| `alu_ld` | 1,565,450 | 6,600,044 | 4.08 | 4.21 |
| `alu_st` | 1,619,941 | 6,600,036 | 3.95 | 4.07 |
| `alu_mul` | 1,700,262 | 6,600,033 | 3.76 | 3.88 |
| `alu_fp_ld_st` | 963,895 | 6,600,046 | 6.63 | 6.84 |
| `alu_branch_ld_st` | 959,562 | 6,600,044 | 6.66 | 6.87 |
| `alu_fp_branch_ld_st` | 545,049 | 4,200,046 | 7.33 | 7.70 |
| `alu_mul_branch_ld_st` | 560,014 | 4,200,047 | 7.14 | 7.49 |
| `all_types` | 719,932 | 5,000,049 | 6.66 | 6.94 |

The memory-only workloads are limited by their own load/store throughput (2.99
and 1.99 ops/cyc); adding independent ALU work fills dispatch/issue slots and
raises them to 4.08/3.95.  ALU+FP reaches 7.74 ops/cyc, while the MUL mix is
lower at 3.76 because the multiplier is the narrower resource.  The 4/5/6-class
mixes land around 6.6-7.3 ops/cyc with no branch-fetch perturbation.
