# br_target_test

Branch-target microbenchmarks ported from
[OpenXiangShan/nexus-am tests/frontendtest/br_target_test](https://github.com/OpenXiangShan/nexus-am/tree/master/tests/frontendtest/br_target_test).

- `call_branch`: calls four different functions in a 16-iteration pattern.
- `indirect_branch`: four targets selected through a function-pointer array.
- `jump_branch`: `switch`/`goto` to four unconditional-jump targets.
- `return_branch`: one function called from four different call sites.

The tests are C; they are built with `-O0` so the call, indirect-branch and
return structure is not folded away.  Each test uses `BENCH_RUN` and reports
cycles, instret and IPC.

```sh
make APP=br_target_test run
make APP=br_target_test TEST=indirect_branch gem5-run
```

Images: `build/arm64-minivirt/br_target_test/<name>.bin`.
