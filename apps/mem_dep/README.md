# mem_dep

Single-core memory-dependence violation tests.  These exercise the out-of-order
load/store path and the memory-dependence predictor (MDP); they are not
exceptions, page faults, or multi-core litmus tests.

The tests execute the hazard patterns; no software correctness check is added.  A
correct core recovers by flushing/replaying, so the interesting signal is the
gem5 MDP counter, not a software PASS/FAIL:

- RAW: an older store followed by a younger load from the same address/line.
- WAR: an older load followed by a younger store to the same address/line.
- Loop-carried RAW: the load in iteration `i+1` depends on the store from
  iteration `i`; this is what makes the MDP work across loop iterations.

| Test | Pattern | Expected gem5 MDP behaviour |
| --- | --- | --- |
| `raw_same_addr` | store then load, same address | easy; only cold/warm-up violations |
| `raw_store_pair` | two stores then load, same address | easy; only cold/warm-up violations |
| `raw_loop_carry` | store at `i`, two loads at `i+1` | **hard/intentional**: sustained predicted violations |
| `raw_loop_late` | loop-carried RAW with a delayed store address | predictor still handles it; low violations |
| `raw_train_flip` | train dependent/independent store-load pairs, then flip | training test; low violations |
| `war_same_addr` | load then store, same address | easy; low violations |
| `war_same_line` | load word 0, store word 1 of the same line | easy; low violations |
| `mixed` | random mix of the above | low violations, branch noise |

The relevant gem5 counters are under `system.cpu.MemDepUnit__0`:
`conflictingLoads`, `mdpUnpredictedViolations`, `mdpPredictedViolations`,
`phastViolationUpdates`, and `mdpFalseDepAtCommit`.  `done` only means the
workload completed; the counters show whether the hazard was actually
exercised.  There is intentionally no error-checking code in the tests.

```sh
make APP=mem_dep run
make APP=mem_dep TEST=raw_loop_carry gem5-run
```

Images: `build/arm64-minivirt/mem_dep/<test>.bin`.

## gem5 counter sanity (KMHv3, 2026-11)

Observed with `--enable-difftest`:

| Test | `mdpUnpredictedViolations` | `mdpPredictedViolations` | Reading |
| --- | ---: | ---: | --- |
| `raw_same_addr` | 3 | 0 | easy; cold/warm-up only |
| `raw_store_pair` | 3 | 0 | easy; cold/warm-up only |
| `raw_train_flip` | 3 | 0 | training test; predictor handles it |
| `raw_loop_carry` | 5 | 20003 | **hard/intentional**; sustained violations |
| `raw_loop_late` | 7 | 1 | delayed address, still easy for the predictor |
| `war_same_addr` | 4 | 0 | easy; many conflicts, few violations |
| `war_same_line` | 3 | 0 | easy |
| `mixed` | 15 | 6 | random mix; low memory violations, branch noise |

`conflictingLoads` is not the violation count: `war_same_addr` has ~100k
conflicts but only four violations, because the predictor resolves them.
`raw_loop_carry` is the test that should make the violation counters move;
the others should stay near the cold/warm-up noise floor.
