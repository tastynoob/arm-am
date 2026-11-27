# packedtest

32-bit register-file pressure test.  The measured body is:

- one streaming 64-bit load from a 64 MiB pool with a 64 KiB stride;
- `PACKED_COPIES` unrolled copies of 24 independent 32-bit ALU writes
  (`PACKED_COPIES=6..12/14/16` gives 144..288/336/384 ALU instructions);
- the whole body is repeated by an outer assembly loop.

There is no inner ALU loop and no random address generator.  The 64 KiB stride
means each of the `PACKED_ITERS` body loads touches a distinct, freshly
allocated line.  `BODY_INSNS` counts every instruction in the loop body: the
load, the pointer add, the ALU writes, `subs`, and `b.ne`.

```sh
make APP=packedtest PACKED_COPIES=12 PACKED_ITERS=1000 gem5-run
```
