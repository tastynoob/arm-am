#!/usr/bin/env bash
#
# Run every cputest image under mini-virt and report PASS/FAIL.
#
# A test passes when the AM runtime reaches _exit(0); the marker is printed by
# am/src/arm64/newlib_syscalls.c.  mini-virt always shuts QEMU down with status
# 0 on HLT #0, so the console marker (not $?) decides the result.

set -u

build_dir=${1:?usage: run.sh <build-dir> [qemu] [cpu] [memory]}
qemu=${2:-qemu-system-aarch64}
cpu=${3:-neoverse-n2}
memory=${4:-8G}

if [ ! -d "$build_dir" ]; then
    echo "run.sh: no build directory: $build_dir" >&2
    exit 1
fi

shopt -s nullglob
binaries=("$build_dir"/*/*.bin)
if [ ${#binaries[@]} -eq 0 ]; then
    echo "run.sh: no test images under $build_dir" >&2
    exit 1
fi

pass=0
fail=0
failed=()

for bin in "${binaries[@]}"; do
    name=$(basename "$(dirname "$bin")")
    output=$(timeout 60 "$qemu" -M mini-virt,platform=bm -cpu "$cpu" \
        -smp 1 -m "$memory" -nographic -monitor none -kernel "$bin" 2>&1)
    status=$?

    if [ "$status" -eq 124 ]; then
        printf 'FAIL %-18s (timeout)\n' "$name"
        fail=$((fail + 1))
        failed+=("$name")
    elif printf '%s\n' "$output" | grep -q '\[arm-am\] exit=0'; then
        printf 'PASS %-18s\n' "$name"
        pass=$((pass + 1))
    else
        printf 'FAIL %-18s (qemu=%d)\n' "$name" "$status"
        printf '%s\n' "$output" | sed 's/^/    /'
        fail=$((fail + 1))
        failed+=("$name")
    fi
done

printf '\ncputest: %d passed, %d failed, %d total\n' \
    "$pass" "$fail" "$((pass + fail))"
if [ "$fail" -ne 0 ]; then
    printf 'failed: %s\n' "${failed[*]}"
    exit 1
fi
