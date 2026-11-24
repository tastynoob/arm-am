---
name: minibench
description: Use when constructing controlled microarchitecture stimulus patterns for the arm-am AArch64 bare-metal suite, especially when converting a claimed CPU resource, latency, bandwidth, or predictor detail into a parameterized minibench family.
whenToUse: Load before designing a new timed body or a sweep of bodies for a microarchitectural claim.
---

# Constructing a minibench

This repository is a single-core AArch64 EL1 bare-metal test bench for
microarchitecture reverse engineering.  Each test is a minibench that produces
one `.bin`.  This skill covers stimulus design and the necessary controls;
execution and result analysis are out of scope.

Inspect the closest existing `apps/<app>/main.c` and `app.mk` for harness style
and parameter plumbing.  Do not re-document the existing tests here.

## Model the target before writing asm

A useful minibench is a **family**, not one body.  Name the resource and the
kind of limit first:

| Target | What to vary | Keep fixed for comparison |
| --- | --- | --- |
| Occupancy/capacity (ROB, IQ, LSQ, store buffer) | number/distance of independent ops | instruction mix, address pattern, loop count |
| Latency (load-use, cache/DRAM, forwarding) | dependency distance or footprint | access count, instruction count |
| Bandwidth / MLP | number of independent streams | footprint, line size, access count |
| Address mapping (set, bank, way) | one address bit/stride | footprint, access count, line count |
| Branch direction/history | pattern period/distance | branch count, code placement |
| Branch target/RAS | target count/depth | call/branch count |
| Fetch/rename/issue width | class mix/body length | total instruction count |
| Instruction supply / fetch | alignment, branch density, body size | dynamic instruction count |
| Memory dependence / disambiguation | alias vs disjoint, known vs unknown address, distance | access count, footprint |
| Cache line/associativity/replacement | stride, conflict-set size, access order | footprint, access count |
| DRAM organization | bank/row/stride/stream count | total lines, instruction count |
| TLB / page walk | page stride/size, footprint | access count |
| Prefetch behavior | stride/stream count/footprint | total lines/instructions |

The stimulus should make the target visible as a **step, plateau, or knee** in
a sweep.  A single sweep point is not an experiment.

## Core construction patterns

These are abstractions; the detailed recipes are in
`references/construction-patterns.md`.

1. **Blocker + filler.**  Put one long-latency operation (typically a load that
   misses beyond LLC) at the head of the body, followed by N independent
   instructions.  The blocker occupies a resource while the filler tries to
   proceed.  Sweep N; the negative control is the same body without the
   blocker.  This is the general shape for ROB/queue/window capacity, but it
   can also expose issue-queue, LSQ, store-buffer, MSHR, or scheduler limits.
2. **Parallel vs serial requests.**  Parallel independent loads exercise
   bandwidth/MLP; a dependent pointer chain exercises latency.  Keep the access
   count and footprint comparable.  Useful for bandwidth/MLP, outstanding-miss
   limits, prefetch, and latency hiding.
3. **Address-bit sweep.**  For set/bank/way/aliasing claims, construct pairs or
   groups of accesses that differ in exactly one address bit or stride.  A
   disjoint/independent layout is the control.  Sweep the bit/stride; avoid
   random addresses, which confound mapping with prefetch and TLB behavior.
   Useful for cache set/bank/way mapping, predictor/index aliasing, TLB/page
   mapping, and conflict-related latency.
4. **Anchor branches + regular filler.**  For predictor/history claims, use two
   anchor branches with a constant direction, and place a *regular* intervening
   branch pattern between them.  Sweep the filler length/period while keeping
   the anchors at stable addresses.  Random filler also perturbs the global
   history you are trying to isolate.  Useful for branch-history tracking depth,
   global or local history, predictor table capacity/aliasing, and loop/phase
   predictors; the two anchors are a fixed reference, not the whole test.
5. **Aliasing pair.**  Construct two anchors/accesses that are intended to map
   to the same predictor/cache/index entry, and a non-aliasing control that
   differs only in the relevant address/placement bits.  Vary one bit at a
   time.  Useful for predictor/cache/index capacity, conflict, and alias
   hypotheses; same-entry vs different-entry is the key comparison.
6. **Density/alignment.**  For instruction-supply/width claims, vary the
   number of instructions or branches per aligned block while keeping the
   total dynamic instruction count fixed.  Use explicit alignment/padding; do
   not let the assembler choose code layout.  Useful for fetch/decode width, uop-cache or
   I-cache line/way effects, and branch-density effects.
7. **Class mix / port pressure.**  Build one-class and mixed-class bodies with
   the same total op count.  Interleave at instruction granularity; vary one
   class ratio at a time.  Do not use blocks of eight same-class ops.  Useful
   for rename/issue width, port contention, execution-unit mix, and
   scheduler/PRF pressure.
8. **Distance sweep.**  For forwarding, load-use, or scheduler claims, vary the
   number of independent instructions between producer and consumer.  Keep the
   producer and consumer fixed; the distance is the only variable.  Useful for
   load-use latency, store-to-load forwarding, bypass/wakeup, and scheduler
   distance.
9. **Footprint / stream sweep.**  For cache and prefetch claims, vary footprint
   and stream count independently.  Use one access per cache line for streaming
   bandwidth; use a pointer chase for latency; keep cold/warm state explicit.
   Useful for cache hierarchy, prefetch depth/stream count, memory-controller
   limits, and TLB effects when page controls are included.
10. **Cold/warm control.**  Build a separate eviction buffer; make one pass cold
    and one warm.  Keep the two separate; averaging hides the state.  The
    buffer should not overlap the measured region.  Useful for
    fill/replacement policy, prefetch state, and cold-start effects.
11. **Memory-dependence probe.**  Use an older store and a younger load to the
    same address/line, and a disjoint-address control.  Variants: unknown store
    address (the store address comes from another load), different offsets,
    load/store width mismatch, store-to-load forwarding distance, training then
    flip, and loop-carried RAW/WAR.  Useful for memory disambiguation,
    forwarding, ordering, and predictor training.  A hazard workload does not
    need a result check.
12. **Instruction-supply probe.**  Build a deterministic instruction sled with
    controlled alignment, branch density, and block size.  Variants: NOP/ALU
    sleds, a branch every K instructions, taken vs not-taken, aligned vs
    unaligned targets, and loop-body sizes crossing I-cache/uop-cache/loop
    buffer boundaries.  Keep the dynamic instruction count fixed.  Useful for
    fetch/decode width, uop cache, I-cache, loop buffer, and branch-density
    effects.
13. **Cache-structure probe.**  Build a conflict array or stride sweep to
    distinguish line size, associativity, replacement, and sectoring.  Vary one
    of: line stride, number of aliasing lines, way-cycling order, access order.
    Use a non-conflicting layout at the same footprint.  Useful for cache
    geometry, replacement, and prefetch interaction.

These notes are hypotheses, not labels.  The same stimulus can expose more
than one structure: a blocker may occupy ROB, LSQ, or MSHRs; an address sweep may
hit cache mapping or TLB behavior.  Keep alternative explanations open, add
controls, and combine patterns when one pattern cannot discriminate.  Do not
infer a specific width/depth from a single point; the sweep shape and the
control are the evidence.  A single pattern is a lens; a real claim often needs
two lenses (for example blocker + address sweep for a queue with banked
storage, or anchor + alias for a predictor table).

## Experiment contract

Before writing asm, make these choices explicit.  They are not a judgement on
the technique; they make the constructed family interpretable:

| Pattern | Sweep | Distinguishes | Control | If the curve is flat |
| --- | --- | --- | --- | --- |
| Blocker + filler | filler count/distance | occupancy limit vs latency | blocker hit/no blocker | widen the filler range or change the blocker; the resource may not be visible in this configuration |
| Parallel vs serial | streams, chain length | bandwidth/MLP vs latency | same access count, opposite shape | change stream count/footprint; another resource may dominate |
| Address-bit | one bit/stride | alias/conflict vs disjoint | same footprint, different bit | try another bit or control; the mapping may be indexed differently |
| Anchor branches | filler length/period | history depth/table capacity vs no history | no filler or irregular filler | move/align the anchors or change the injected pattern; the predictor may be insensitive to that history |
| Memory dependence | alias, unknown address, distance | forwarding/disambiguation vs independent | disjoint addresses | change address/distance/width; the hazard may not enter the speculative path |
| Fetch/instruction supply | alignment, density, body size | fetch/decode/uop/I-cache limit vs backend | fixed dynamic instruction count | change alignment/density/body; the backend may be the limiting resource |
| Cache structure | N aliases, stride, access order | line/assoc/replacement vs spread | non-conflicting layout at same footprint | change N/stride/order; the mapping may not be exposed |
| Store queue/LSQ | outstanding N | queue capacity/ordering vs unlimited | fewer outstanding ops | change the mix/probe; another resource may be the limit |
| DRAM organization | streams, stride | bank/row/channel limit vs single stream | fixed request count | change streams/stride; controller/prefetch may mask it |

A flat or ambiguous sweep is information: adjust the stimulus or control, or
state that the effect is not exposed in this configuration.  It is not a reason
to discard the technique.

## Architecture dependence

These patterns are portable starting points, not facts about every core.  The
same stimulus can be absorbed by a different structure, hidden by a larger
window, handled by a different mechanism, or served by a different memory
path on another architecture.  Adapt the range, addresses, branch periods, class mix, and
controls to the target; keep the negative control and the one-variable rule.
A technique that produces a clear step on one core may produce a smooth line on
another, and that difference is itself useful information.

## Sweep construction

A claim about capacity or a table size needs several binaries, not a runtime
loop over the parameter.

- Express the sweep point as a compile-time constant (`-D...` in `app.mk`).
- One `.bin` per point; record the point in a semantic `BENCH_FORMAT` field
  (`window=64`, `distance=8`, `bank=3`, `pattern=period4`, `streams=4`).
- Keep the body layout, instruction count, and loop count fixed across points
  except for the swept dimension.  Use NOP padding to preserve alignment and
  branch addresses when needed.
- Include a negative control at the same code size/instruction count.
- Use a fixed-seed pseudorandom order only when irregularity is required;
  for predictor/history work, use a deterministic regular pattern instead.

## Dependencies

- **Throughput:** unique destinations, constant sources, no load/store
  dependency, no flag dependency.  Do not read a register written by another
  body op.
- **Latency:** one explicit producer-consumer chain; keep all other work
  independent.
- **Hazard:** the hazard address/line is deliberate; add a disjoint version as
  the control.
- Avoid hidden chains through the loop counter, flags, or reused temporaries.
- If a long-latency load is the blocker, it must be independent of the filler;
  otherwise you have built a latency chain, not an occupancy experiment.

## Compiler and memory rules

- Write the timed body in inline asm when the loop shape or queue occupancy
  matters.
- Clobber every register and `cc`/`memory` the body changes.
- Check `objdump -d`; do not trust a C loop to survive `-O2`.
- Use `volatile`/a sink to prevent removal; keep setup and warm-up outside the
  timed body.
- Align buffers to 64 B.  Independent accesses use disjoint regions;
  same-address or same-line accesses are deliberate.
- For streaming, touch one word per line; for L1 throughput, keep the footprint
  small; for cache-miss latency, chase pointers; for bank tests, hold the line
  count and footprint fixed.

## Branches

- In a dispatch/throughput body, a mixed branch must be not-taken:
  `cbz x0, 9f` with `x0` constant and `9:` at body end.
- In a predictor/history body, branch outcomes are the variable.  Use taken
  and not-taken patterns as needed, and keep the anchor branches' direction
  constant.
- Separate direction from target: direction tests use conditional branches;
  target/RAS tests use calls, indirect branches, and returns.
- Keep the loop-control branch outside the measured pattern and fixed across
  variants.

## Harness and output

A new app uses:

```text
apps/<app>/app.mk          # TESTS, TEST_SRC, parameters
apps/<app>/README.md       # one test-set README
apps/<app>/<test>/main.c   # one test; or apps/<app>/main.c for one-test app
```

Minimal `app.mk`:

```make
TESTS := mytest another
ITERS ?= 100000
APP_CPPFLAGS += -DBENCH_ITERS=$(ITERS)ULL -DWINDOW=$(WINDOW)
TEST_SRC = $(APP_DIR)/$(1)/main.c
```

Harness entry points in `am/include/bench.h`:

- `bench_run(fn)` — time one `fn(BENCH_ITERS)` call and emit the formatted
  result.
- `BENCH_RUN(name, expr)` — time one expression; the name is only for the
  banner.
- `bench_format(cycles, instret, "fmt", ...)` — emit the canonical line plus
  caller-defined fields.  There is no `sample=` parameter.
- `bench_warm_buffer()` — warm the shared 8 KiB buffer.

Output contract:

```text
BENCH_FORMAT test=<app>/<test> ipc=<x.xx> [semantic field ...]
```

`test` and `ipc` are mandatory.  Use semantic fields to identify the sweep
point and configuration.  Keep throughput/bandwidth values out of the formatter;
use a separate human log if needed.

## Checklist

- [ ] Claim, resource model, and falsifying outcome written down.
- [ ] Sweep parameter identified; one point per `.bin`.
- [ ] Negative control at the same instruction/access count.
- [ ] Dependency graph is the intended one; no hidden chain.
- [ ] Long-latency blocker is independent of the filler.
- [ ] Address mapping varies one bit/stride; no random addresses.
- [ ] Predictor filler is regular, not random; anchors stay at fixed addresses.
- [ ] Throughput bodies use unique destinations and constant sources.
- [ ] Dispatch branches are not-taken; predictor outcomes are deliberate
      (taken/not-taken as the pattern requires).
- [ ] Clobbers, loop counter, alignment, and code size recorded.
- [ ] `BENCH_FORMAT` has `test`/`ipc` and semantic extra fields.

## Read next

| Topic | File |
| --- | --- |
| Detailed construction patterns and parameterizations | `references/construction-patterns.md` |
