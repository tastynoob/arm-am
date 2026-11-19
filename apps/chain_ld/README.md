# chain_ld

Serial pointer chasing over a configurable footprint, i.e. the latency
counterpart of `stream_ld`.  The footprint is a single-cycle linked list of
64 B nodes; one pass follows `nodes` dependent loads and returns to the head.

- `FOOTPRINT` (default `1M`, K/M/G accepted): address span.  The node count is
  the largest power of two that fits: 1M is 16,384 nodes (and dependent loads)
  per pass.  `FOOTPRINT=64M` is 1,048,576 dependent loads per pass; that is the
  full-DDR configuration but is very slow on a detailed model.
- `PASSES` (default 1; `ITER` alias): timed passes.  1M with two passes is
  32,768 dependent loads.
- `FLUSH` (default `64M`, `0` disables): the linked list is written during
  setup, so the 64M flush (> 48M L3) evicts it before pass 0.  Pass 1 then starts with the
  cache state left by pass 0; a 1M footprint is cache-resident after pass 0.

```sh
make APP=chain_ld FOOTPRINT=4M PASSES=2 run
make APP=chain_ld FOOTPRINT=4M PASSES=2 gem5-run
```

`cyc/op` is the dependent-load latency and `B/cyc` uses the 64 B line
footprint of each node (8 B payload).  Image:
`build/arm64-minivirt/chain_ld/chain_ld.bin`.
