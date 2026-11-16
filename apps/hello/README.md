# hello

Prints `arm-am: hello from mini-virt` and exits through `_halt(0)`.

Smoke test for the serial console, newlib `puts`, and the QEMU/gem5 exit path.

```sh
make APP=hello run
```
