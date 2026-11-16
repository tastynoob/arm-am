# cputest

来自 nexus-am `tests/cputest`（`memory-test-v3`，commit `812b18f`）的基础
ISA/后端回归测试，共 33 个独立程序。

## 运行

```sh
make cputest          # 从仓库根目录编译并在 QEMU 上运行全部
make cputest-build    # 只编译
make cputest-gem5     # 编译并在 gem5 上运行（--bare-metal 对齐平台）
```

每个测试编译为 `build/arm64-minivirt/cputest/<name>/<name>.bin` 和
`<name>.elf`；`run.sh` 把 `.bin` 喂给 mini-virt QEMU，`run-gem5.sh`
把 `.elf` 交给 gem5 的 `--bare-metal`。

## 判定

- 程序返回 0 或执行 `_halt(0)`：通过。
- `nemu_assert` 失败调用 `_halt(1)`：失败。
- 超时或没有打印 `[arm-am] exit=0`：失败。

mini-virt 把 `HLT #0` 当作正常关机，QEMU 进程退出码不会反映测试结果，
因此必须看 console marker。

## 与上游的差异

- `string.c`：`strcmp` 的负值断言从 `== -1` 改为 `< 0`，因为 newlib
  不一定返回 -1。
- `app.mk` 加 `-fwrapv`，避免 `-O2` 利用有符号溢出 UB。
- 其余源文件未修改；`trap.h` 使用本仓库的 `klib` 兼容头。
