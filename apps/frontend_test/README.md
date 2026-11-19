# frontend_test

Frontend microbenchmarks ported from
[OpenXiangShan/nexus-am tests/frontendtest](https://github.com/OpenXiangShan/nexus-am/tree/master/tests/frontendtest)
(the top-level `tests/*.c` set).  Each test is one `.bin`.

## Groups

- Fetch/direction: `2fetch`, `brsimple`, `brnum`, `brnum2`, `brnum2_uftb`,
  `brnum3`, `brwidth`, `resolve`.
- History predictors: `tage1`, `tage2`, `tage3`, `tage4`, `tage5`, `ittage`.
- Return-address stack: `rastest`, `ras_recursive`.
- Width (non-branch reference): `aluwidth`, `fpuwidth`, `renamewidth`,
  `ifuwidth`, `forloop`.

`fetchfrag` is not ported: it depends on RISC-V compressed (`c.add`)
instructions to create fetch fragmentation inside 32 B, and AArch64 has
fixed-width 4 B instructions, so there is no equivalent test.

## AArch64 port notes

- RISC-V `beqz`/`bnez`/`blt`/`bne`/`j` became `cbz`/`cbnz`/`cmp`+`b.cond`/`b`.
- `jal`/`ret` became `bl`/`ret`; `fadd.d` became `fadd dN, ...`.
- `brnum3`'s `lui` padding is now `nop`; it is still 4 B and keeps the
  multi-branch block longer than 32 B.
- `2fetch` uses 64 B alignment and two three-instruction blocks inside one
  line.
- `ittage`'s indirect call is a C call through a `volatile` function pointer,
  which lowers to `blr`; `tage5`/`ittage` use `cbnz` for the forced branch.
- Each test uses `BENCH_RUN(name, expr)` from `am/include/bench.h`; it prints
  `cycles`, `instret`, `ipc` and a `PASS` line.  These tests measure timing,
  not architectural correctness, so `PASS` only means the image ran to
  completion.  QEMU cycle counts are not meaningful; use gem5/RTL for timing.

```sh
make APP=frontend_test run
make APP=frontend_test TEST=2fetch run
make APP=frontend_test TEST=brsimple gem5-run
```

Images: `build/arm64-minivirt/frontend_test/<name>.bin`.
