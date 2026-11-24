# Construction patterns

These patterns describe how to build the stimulus.  Start from the closest
existing `main.c` for harness style, then build the family described here.

The "Can probe" notes are starting hypotheses, not labels.  A body can expose
several structures at once, and one point of a sweep is not enough to identify
one.  Combine patterns and controls when the alternatives are not separated.
The same pattern can map differently on another architecture; adapt the
distances, addresses, patterns, and class mix instead of treating the notes as
fixed facts.

## Blocker + filler (capacity and occupancy)

**Can probe:** ROB, issue queue, LSQ, store buffer, MSHR/outstanding-miss
limits, scheduler capacity, and latency-hiding limits.  The key is where the
younger filler stops entering or retiring, not the blocker itself.

Use when the claim is about a window, queue, buffer, or scheduler capacity:
ROB, issue queue, LSQ, store buffer, miss-status registers, or an outstanding
request limit.

**Construction:**

- Place one or a few **long-latency blockers** at the head of the body.  A
  blocker is typically a load whose line is cold and beyond LLC, so it remains
  outstanding for a long time.
- Keep the blocker independent of the filler: the filler must not consume the
  blocker's destination or wait on its completion.
- Fill with N independent instructions (ALU/FP or independent loads) so the
  younger work can issue and occupy the downstream structures.
- Sweep N.  The control is the same body with the blocker replaced by a hit or
  with no blocker, at the same filler count and code size.
- For a queue with separate load/store capacity, repeat with independent loads
  and stores, and with one blocker per queue.
- Ensure the loop-control branch is not the limiting structure; use a simple
  counter and keep the body large enough to dominate loop overhead.

**What makes it clean:** the blocker supplies the stall; the filler supplies
the occupancy.  If the filler depends on the blocker, the body exposes latency,
not capacity.

**Common mistake:** a blocker that hits in cache, or a filler that is too short
to reach the resource.  Make the sweep wide enough to cross the expected
capacity and include points on both sides.

## Parallel requests vs serial chain

**Can probe:** sustained bandwidth, memory-level parallelism, outstanding
request limits, prefetch, and the latency/throughput distinction.  The same
access count with independent vs dependent requests is the control.

Use this pattern to separate bandwidth/MLP from latency.

**Parallel (bandwidth/MLP):**

- N independent loads, each with a unique destination and a unique line.
- One word per cache line; do not load multiple words from the same line.
- Keep the footprint and total line count fixed while varying N or stream
  count.
- The control is a single stream or a serial chain at the same line count.

**Serial (latency):**

- A pointer chase: each load's address comes from the previous load.
- Vary the footprint to move between cache levels; vary the chain length.
- The control is the same footprint with independent loads.
- A serial chain supports a latency claim, not a bandwidth claim: it exposes
  dependent-load latency, not request parallelism.

**MLP variant:** use several independent pointer chains (different heads) so
multiple misses are outstanding; sweep the number of chains while keeping the
total number of dependent loads fixed.

## Cache latency and footprint

**Can probe:** L1/L2/LLC/DRAM latency, fill/replacement behavior, prefetch
state, and TLB effects when the footprint crosses page boundaries.  Keep the
chain shape fixed while changing footprint.

**Construction:**

- Build a single-cycle linked list of cache-line-sized nodes, aligned to 64 B.
- Follow one link per access; only the address chain may connect iterations.
- Choose footprints that fit L1, L2, LLC, and exceed LLC.
- Use a separate eviction buffer for a cold pass; do not overlap it with the
  measured region.
- Keep cold and warm passes separate; do not average them.
- Do not add independent loads to a latency chain unless the claim is about
  MLP; they change the bottleneck.

## Streaming bandwidth and prefetch

**Can probe:** sustained bandwidth, prefetch depth and stream count,
line-fill behavior, memory-controller limits, and cache-resident vs streaming
regimes.

- Touch one 8 B word at each 64 B line boundary; use many independent lines per
  iteration.
- Touching two words from the same line in a streaming body turns misses into
  hits; use one word per line unless line reuse is the variable.
- Sweep footprint, stream count, and stride independently.  Keep the total line
  count fixed when comparing strides.
- The control is a cache-resident footprint or a serial chain at the same
  access count.
- For prefetch claims, compare sequential and strided streams at the same
  footprint; a prefetcher is a footprint/stride effect, not a single bandwidth
  number.
- Do not use explicit prefetch instructions unless the claim is about them.

## Address mapping: set, bank, way, alias

**Can probe:** cache set/bank/way mapping, predictor/index aliasing, TLB
mapping, and conflict-related latency.  One address bit or stride is the
variable; the same-page disjoint group is the control.

Use when the claim is about a physical/index mapping, bank conflict, set
conflict, or aliasing.

**Construction:**

- Start with a group of accesses at addresses that differ in exactly one bit or
  one stride.  Keep the line count, footprint, alignment, and access count
  fixed.
- Candidate group: addresses that should map to the same set/bank/index.
  Control group: addresses that should map to different entries; change only
  the relevant bit.
- For a bank conflict, use parallel loads to different lines that are intended
  to hit the same bank; compare with a group spread across banks.
- For a set conflict, use N lines that alias one set; compare with N lines in
  different sets at the same footprint.
- Sweep one address bit at a time; do not use random addresses.  Randomization
  mixes mapping, prefetch, and page effects.
- Keep accesses within the same page when testing a cache/bank mapping; page
  walks and TLB behavior are separate variables.
- For code-index aliasing (predictor/BTB), use fixed code placement and change
  only the relevant address bits; record the placement.

**Control check:** a mapping experiment needs a positive and a negative group.
Use the different-bank/different-set group as the control; a single same-bank
point is a starting observation, not the whole experiment.

## Branch direction and global history

**Can probe:** branch-history tracking depth, global/local history, predictor
table capacity/aliasing, loop/phase predictors, and confidence behavior.  The
anchor pair gives a constant reference; the regular filler changes history.

Use when the claim is about a direction predictor, history length, table
capacity, or aliasing.

**Construction:**

- Use two **anchor branches** with a constant direction.  They are the
  reference points whose behavior you want to perturb.
- Between the anchors, insert an unrelated branch pattern.  The pattern must be
  regular (for example a fixed period of taken/not-taken outcomes).  Random
  interleaving also perturbs the global history you are trying to control.
- Sweep the number of intervening branches and their period.  Keep the total
  dynamic branch count and the anchor addresses fixed; use fixed-size slots,
  NOP padding, or aligned blocks.
- Keep loop-control branches outside the pattern and identical across points.
- For predictor capacity, increase the number of distinct branches/history
  entries; for history length, increase the period/distance.
- For aliasing, place two anchor branches or history patterns at addresses
  intended to map to the same predictor entry; vary the relevant address bits
  and use a non-aliasing control.
- Direction tests use conditional branches.  Do not mix indirect-target
  experiments into a direction test.

**Common mistake:** using random filler because it looks "more realistic".  It
destroys the controlled history; use a deterministic period.

## Branch target, indirect target, and RAS

**Can probe:** BTB capacity/associativity, indirect-target prediction, return
address stack depth, call/return behavior, and target aliasing.

- Call/return: several distinct functions called in a fixed rotation; keep the
  call count and target count fixed.
- Indirect: a function-pointer table or register target with a fixed target
  sequence; vary the number of targets.
- Jump: a switch/goto with several labels; vary the target count.
- RAS: nested or recursive calls; vary depth and call-site count.  Keep the
  return addresses stable.
- Build with control flow visible (`-O0` or asm) and verify the call/return or
  indirect instruction is present.
- Do not reuse a direction-predictor pattern as a target test.

## Fetch bandwidth and instruction supply

**Can probe:** fetch/decode width, instruction-cache line/way behavior,
uop-cache/loop-buffer limits, branch density, alignment penalties, and
taken-branch redirect costs.

**Construction:**

- Build a deterministic sled of instructions whose dynamic count is fixed.
- Variants: NOPs vs independent ALU ops; a branch every K instructions;
  taken vs not-taken; aligned vs unaligned branch targets; body sizes that
  cross likely I-cache/uop-cache/loop-buffer boundaries.
- Keep backend demand low enough that fetch is the visible limit; if the sled
  is too wide or too dependent, the backend hides the fetch effect.
- Keep code placement fixed with `.align`/padding; a compiler-generated layout
  change can move the very boundary under test.
- For I-cache/uop-cache capacity, vary code footprint without changing the
  dynamic instruction count (unroll/replicate blocks, then execute the same
  count).
- Vary instructions per aligned block (e.g., 32 B/64 B), branch spacing, and
  branch density; keep the dynamic instruction and branch counts fixed.
- AArch64 is fixed 4 B; do not try to reproduce a compressed-instruction fetch
  test.
- Keep the instruction cache footprint constant if the claim is about fetch
  width rather than I-cache capacity.
- Use asm or a disassembly check; C can fold NOPs or move blocks.

## Rename, dispatch, and issue width

**Can probe:** rename/allocate width, issue width, port contention,
execution-unit mix, scheduler/PRF pressure, and class-specific limits.

- Build a body of independent instructions with unique destinations and
  constant sources.
- Interleave classes at instruction granularity; do not use blocks of eight.
- Start with one class, then add a second/third class; keep the total op count
  and body length fixed.
- Vary one class ratio at a time; a sweep of ratios reveals a shared port or a
  class-specific pipe.
- Keep branches not-taken in a dispatch body; do not let a redirect confound
  the width measurement.
- Use a control with the same instruction count but a single class.

## Dependency distance and forwarding

**Can probe:** load-use latency, store-to-load forwarding, bypass/wakeup
distance, scheduler behavior, and result-bus pressure.

- Producer and consumer are fixed; insert N independent instructions between
  them.
- Sweep N.  The control is an independent producer/consumer pair at the same
  body length.
- For store-to-load forwarding, keep the store and load on the same line and
  vary the offset/distance; use a disjoint-address control.
- For load-use latency, vary the number of instructions between a load and its
  use, not the cache footprint.
- Do not let the compiler reorder the producer/consumer; use asm or a
  dependency through a register it cannot eliminate.

## Store buffer and write-combining

**Can probe:** store-queue capacity, write-combining, store-to-load
forwarding, ordering behavior, and memory-disambiguation limits.

- Build N independent stores to disjoint lines, followed by a dependent load or
  a probe that forces the stores to be visible.
- Sweep N; the control uses fewer outstanding stores or a load from a
  different line.
- For write-combining, vary the number of stores to the same line and their
  offsets; use a different-line control.
- Add a fence only when the claim is about the fence; otherwise it serializes
  the experiment.
- Do not add result checks; the pattern is the evidence.

## TLB and page-walk scope

**Can probe:** TLB capacity/associativity, page-size effects, and page-walk
depth in principle.  There is no dedicated TLB harness in this suite; a
cache-only result does not isolate TLB behavior, so use a TLB-specific design or
state the scope.

If the claim needs a TLB experiment, first verify the AM mapping supports the
required page size and aliasing, then build a footprint/stride sweep with a
cache-only control.  Keep page-crossing and cache-set effects separate.  If the
mapping cannot isolate the effect, state the scope instead of presenting a
cache test as a TLB test.

## SMT and multi-core scope

The suite is single-core.  It cannot test SMT sharing, coherence, or litmus
properties.  Use a different harness for those claims.

## Memory dependence and disambiguation

**Can probe:** memory disambiguation, store-to-load forwarding, load/store
ordering, memory-dependence prediction and training, and false-dependence
penalties.

**Construction:**

- Base pair: older store, younger load from the same address.  Control:
  disjoint addresses at the same access count/footprint.
- Same-line pair: load word 0, store word 1.  Control: different lines.
- Unknown store address: make the store's address depend on another load, so
  the core cannot resolve the alias at issue.  Control: a known constant
  store address.
- Forwarding distance: fix producer and consumer, insert N independent
  instructions, and sweep N.  Control: independent producer/consumer at the
  same body length.
- Width mismatch: 4 B store → 8 B load, or vice versa; same address/line.
  Control: matching width.
- Training/flip: run a predictable alias pattern for a while, then change the
  aliasing relation or address; useful for predictor learning and confidence.
- Loop-carried: store in iteration i, load in iteration i+1; this is the
  sustained hazard.  Control: the same loop with the store and load to
  disjoint regions.
- Add a fence only when the claim is about ordering; a fence otherwise
  serializes the very overlap you want to observe.
- Do not add software result checks; a correct core can flush/replay.  Use
  `status=DONE` for a hazard workload.

## Cache line size, associativity, replacement, and sectors

**Can probe:** line size, sectoring, associativity, replacement policy,
conflict behavior, and write-allocate behavior.

**Construction:**

- Line-size/stride sweep: access one word per stride; vary stride around the
  expected line size; keep the number of accesses fixed.  A line-size cliff
  appears when two accesses stop sharing a fill.
- Conflict array: N lines chosen to map to the same set; vary N and the
  way-cycling order.  Control: N lines spread across sets at the same
  footprint.
- Replacement: cycle through more than the suspected associativity; compare
  sequential, cyclic, and reverse orders.  Keep the access count fixed.
- Sectoring: touch one word per 32 B/64 B sector and compare with touching the
  full line; do not let prefetch or write-allocate hide the effect.
- Write-allocate: store to a cold line, then load another word from it;
  control with a line already present.
- Keep page/TLB effects out by staying within one page for a cache-geometry
  test.

## DRAM banks, rows, and channels

**Can probe:** bank/row-buffer behavior, channel interleaving, and
memory-controller parallelism.

**Construction:**

- Stream multiple independent address sequences; vary stride and stream count.
- Hold the total number of lines/requests fixed while changing the address
  interleaving.
- For row-buffer effects, compare repeated accesses to a small set of rows with
  a spread across many rows at the same request count.
- For bank/channel parallelism, vary the number of independent streams and
  their address offsets; a single stream is the control.
- Keep cache/prefetch behavior controlled: use a footprint beyond LLC, one
  access per line, and avoid dependent chains unless latency is the subject.
- Address mapping is usually undocumented; sweep one bit/stride and treat the
  result as a hypothesis, not a known bank number.

## Store queue, LSQ, and ordering

**Can probe:** store-queue depth, load-queue depth, write-combining, ordering,
and forwarding pressure.

**Construction:**

- Store queue: N independent stores to disjoint lines, followed by a dependent
  load/probe that forces visibility.  Sweep N; control with fewer outstanding
  stores or a different line.
- Write-combining: several stores to the same line at different offsets;
  control with stores to different lines.
- LSQ: mix independent loads and stores; add a long-latency load as a blocker
  and sweep the number of younger memory ops.
- Ordering: only add fences/barriers when the claim is about ordering; a fence
  otherwise collapses the experiment.
- No result checks; the pattern is the hazard.

## Parameterization and output

- A capacity/table/pattern claim is a family.  Make the swept value a
  compile-time constant and build one `.bin` per point.
- Name parameters so the output is self-describing:
  `window=`, `distance=`, `streams=`, `stride=`, `bit=`, `period=`,
  `targets=`, `depth=`, `footprint=`.
- Keep `BENCH_FORMAT` test/ipc mandatory; put the sweep point in a semantic
  extra field.  No `sample=` index.
- Keep the code size and alignment fixed across points when branch/code
  placement is part of the experiment; use padding, not unrelated edits.
- Keep throughput/bandwidth out of the formatter; a separate human log is fine
  if the test needs one.

## Construction checklist

- [ ] Resource model and falsifying outcome named.
- [ ] One sweep dimension; one point per `.bin`.
- [ ] Positive and negative control with the same instruction/access count.
- [ ] Blocker independent of filler; no accidental latency chain.
- [ ] Parallel requests use unique lines/destinations; serial chain uses one
      address chain.
- [ ] Address mapping varies one bit/stride; same-page control.
- [ ] Predictor filler is regular; anchors fixed in direction and address.
- [ ] Dispatch branches are not-taken; predictor/target outcomes are deliberate
      (taken/not-taken as the pattern requires).
- [ ] Code size/alignment/padding recorded.
- [ ] Clobbers and loop counter correct.
- [ ] `BENCH_FORMAT` has `test`/`ipc` and semantic fields; no `sample`.
