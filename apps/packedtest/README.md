# packedtest

Exercises gem5's packed 32-bit register allocation: two adjacent `w9`/`w10`
definitions must be kept in one physical register, then consumed by a 64-bit
add.  The expected result is `0x0000000000000802`.

```sh
make APP=packedtest run
```
