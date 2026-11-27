# move_elim

Move-elimination test.  The measured body is one cheap streaming load plus
`MOVE_COPIES` unrolled blocks of 24 register-to-register moves, repeated by an
assembly outer loop.  A move that reuses its source physical register should
not consume a new physical register, so the IPC curve should not show a knee
as the number of move blocks grows.

`MOVE_COPIES=6..12/14/16` gives 144..288/336/384 moves.  `BODY_INSNS` counts
the load, pointer add, moves, `subs`, and `b.ne`.

```sh
make APP=move_elim MOVE_COPIES=16 MOVE_ITERS=1000 gem5-run
```
