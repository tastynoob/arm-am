# minibench

Two small loops used as a quick core/memory smoke test:

- an integer LCG/xorshift loop, 1,000,000 iterations;
- a read/write loop over a 8 KiB buffer, 1,000,000 iterations.

When the implementation exposes a PMU, it reports `PMCCNTR_EL0` cycles,
`INST_RETIRED` (event `0x08`) instructions, and IPC.  If the PMU is missing,
cycles fall back to `CNTVCT_EL0` and the instruction count is printed as 0.

```sh
make APP=minibench run
```
