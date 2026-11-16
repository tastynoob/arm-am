# mem_test_latency

Pointer-chasing latency test adapted from nexus-am `memory-test-v3`.

It randomly links 4000 nodes, flushes a footprint of `256 KiB * INST`, then
performs 500 iterations of two dependent loads (1000 loads total) and prints
the elapsed cycles and `cycles/load`.

```sh
make APP=mem_test_latency run
make APP=mem_test_latency INST=32 run
```

QEMU has no cache model, so the absolute latency is only meaningful on a
cycle-accurate model such as gem5.
