# mem_test_bw

Strided/unrolled memory bandwidth test adapted from nexus-am
`memory-test-v3`.

```sh
make APP=mem_test_bw TEST_TYPE=mem run     # DDR bandwidth
make APP=mem_test_bw TEST_TYPE=cache run   # cache scan
```

It reports `Bytes/cycle` using the upstream formula
`samples * 64 B / cycles`.

QEMU has no cache model, so the bandwidth number is only meaningful on a
cycle-accurate model such as gem5.
