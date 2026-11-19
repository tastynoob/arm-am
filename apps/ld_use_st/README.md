# ld_use_st

Load-use-store chain: a single cache line is repeatedly loaded, incremented
and stored back, so the next load depends on the previous store.

```
load -> add -> store -> load -> add -> store -> ...
```

`cyc/op` is the sustained read-modify-write latency, including store-to-load
forwarding.  Parameter: `ITERS` (default 100000) is the number of RMW rounds.

```sh
make APP=ld_use_st run
make APP=ld_use_st gem5-run
```

Image: `build/arm64-minivirt/ld_use_st/ld_use_st.bin`.
