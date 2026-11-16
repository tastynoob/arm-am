# microbench

Small AArch64 microbenchmarks adapted from the NEMU/nexus-am minibench
style.  Each loop is inline assembly so the operations are not optimised
away, and cycles/instructions come from the architectural PMU.

| Test | What it measures |
| --- | --- |
| `alu` | 64 independent `add` instructions per iteration; ALU throughput |
| `ldst` | 8 scalar loads + 8 scalar stores per iteration, 8 KiB L1-resident buffer; L1 ld/st throughput |
| `pair` | 8 `ldp` + 8 `stp` per iteration, same L1 buffer; pair ld/st throughput |
| `cmpbr` | 32 `cmp` + `b.ne` pairs per iteration; cmp+branch fusion/throughput |

Output includes `ops/cyc`, `cyc/op`, `ipc`, and for memory tests `B/cyc`.
The PMU reports real `PMCCNTR_EL0` cycles and `INST_RETIRED` instructions;
without a PMU the cycle count falls back to `CNTVCT` and instructions are 0.

```sh
make APP=microbench run
make APP=microbench gem5-run
```

Reference values from the current KMHv3 ideal gem5 platform (not guarantees;
they depend on the CPU/cache model):

```text
alu     ~4.8 ops/cyc   IPC ~5
ldst    ~3.7 ops/cyc   ~29 B/cyc
pair    ~3.7 pair-ops/cyc  ~59 B/cyc
cmpbr   ~2.9 pairs/cyc  instret ~34/iter
```

The `cmpbr` instruction count is the interesting one: 32 `cmp`+`b.ne` pairs
per iteration would be 64 architectural instructions, but `INST_RETIRED`
reports about 34-35 per iteration, which is consistent with the model
fusing each compare/branch pair into one macro-op.  QEMU TCG does not expose
`INST_RETIRED`, so its `instret`/IPC columns are 0 and its cycle count is
only a simulation proxy.

These tests are adapted from the NEMU/nexus-am minibench style, not
verbatim copies of upstream code.
