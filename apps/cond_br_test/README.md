# cond_br_test

Conditional-branch pattern microbenchmarks ported from
[OpenXiangShan/nexus-am tests/frontendtest/cond_br_test](https://github.com/OpenXiangShan/nexus-am/tree/master/tests/frontendtest/cond_br_test).

Patterns: `always_taken`, `never_taken`, `alternating`, `two_bit_pattern`,
`three_bit_pattern`, `prime_based_pattern`, `switching_pattern`,
`nested_branches`, `early_exits`, `rare_branches`, `gradual_transition`,
`aliasing_pattern`, and `all_patterns` (runs all of them in one image).

These are plain C branch patterns, built with `-O0` so the conditions are not
folded into straight-line code.  Each test uses `BENCH_RUN` and reports cycles,
instret and IPC; `PASS` means the image completed.

```sh
make APP=cond_br_test run
make APP=cond_br_test TEST=alternating gem5-run
```

Images: `build/arm64-minivirt/cond_br_test/<name>.bin`.
