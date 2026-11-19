# stream_ld

Streaming load bandwidth over a configurable footprint.  Every cache line is
touched exactly once: one 8-byte load at a 64 B line boundary, with 16
independent lines (1 KiB) issued per loop iteration.  Loading multiple words
from the same line would turn misses into L1 hits and understate the streaming
bandwidth.

- `FOOTPRINT` (default `64M`, K/M/G accepted): address span, rounded to 1 KiB.
  64M is 1,048,576 cache lines per pass.
- `PASSES` (default 1; `ITER` alias): timed passes.  Two passes are 2,097,152
  line loads.  Pass 0 is cold (the footprint is never written); pass 1 starts
  with the cache state left by pass 0.
- `FLUSH` (default 0): optional separate eviction buffer.  `stream_ld` does not
  write the footprint, so pass 0 does not need a flush.  Set it only for a
  special cold-start experiment.

```sh
make APP=stream_ld FOOTPRINT=64M PASSES=2 run
make APP=stream_ld FOOTPRINT=64M PASSES=2 gem5-run
```

`ops` is cache lines; `B/cyc` counts the full 64 B line footprint.  Use a
footprint larger than the LLC for DDR streaming; smaller footprints are useful
for quick cache-resident runs.  Image:
`build/arm64-minivirt/stream_ld/stream_ld.bin`.
