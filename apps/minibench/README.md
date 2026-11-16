# minibench

Two small loops used as a quick core/memory smoke test:

- an integer LCG/xorshift loop, 1,000,000 iterations;
- a read/write loop over a 8 KiB buffer, 1,000,000 iterations.

It prints elapsed cycle counts and a final `PASS` sink value.

```sh
make APP=minibench run
```
