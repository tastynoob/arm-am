# mem_test_latency

Pointer-chasing latency test adapted from nexus-am `memory-test-v3`.

It randomly links 4000 nodes, flushes a footprint of `256 KiB * INST`, then
performs 500 iterations of two dependent loads (1000 loads total) and prints
the elapsed cycles and `cycles/load`.

```sh
make APP=mem_test_latency run
make APP=mem_test_latency INST=32 run
```

On the KMHv3 ideal gem5 platform the L3 is 48 MB, so the default
`INST=16` only flushes 4 MB and the reported value is an L3-hit latency
(about 30 cycles/load).  Use `INST=256` (64 MB flush) to evict the list
from L3; it then reports the DRAM latency (about 238 cycles/load on the
current platform).  QEMU has no cache model, so only gem5/RTL numbers are
meaningful.
