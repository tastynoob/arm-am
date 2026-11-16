#!/usr/bin/env bash
#
# Run every cputest ELF under gem5 with the patched --bare-metal path.
#
# The gem5 config's --bare-metal mode builds the QEMU-aligned platform:
# RAM 0x40000000, SimpleUart 0x13000000 (stdout), and the HLT #0 simtrap.
# As with QEMU, the authoritative result is the console marker
# "[arm-am] exit=N"; the simtrap itself always exits gem5 with status 0.

set -u

build_dir=${1:?usage: run-gem5.sh <build-dir> <gem5> [config] [args...]}
gem5=${2:?usage: run-gem5.sh <build-dir> <gem5> [config] [args...]}
config=${3:-configs/example/idealkmhv3_arm.py}
shift $(( $# > 3 ? 3 : $# ))
extra_args=("$@")

: "${GEM5_HOME:?run-gem5.sh: GEM5_HOME must point at the patched gem5 tree}"

if [ ! -d "$build_dir" ]; then
    echo "run-gem5.sh: no build directory: $build_dir" >&2
    exit 1
fi

gem5_abs=$(CDPATH= cd -- "$(dirname -- "$gem5")" && pwd)/$(basename -- "$gem5")
outroot="$build_dir/m5out-gem5"
rm -rf "$outroot"
mkdir -p "$outroot"
cd "$GEM5_HOME" || exit 1

shopt -s nullglob
binaries=("$build_dir"/*/*.elf)
if [ ${#binaries[@]} -eq 0 ]; then
    echo "run-gem5.sh: no test images under $build_dir" >&2
    exit 1
fi

pass=0
fail=0
failed=()

for elf in "${binaries[@]}"; do
    name=$(basename "$(dirname "$elf")")
    output=$(timeout 180 "$gem5_abs" -d "$outroot/$name" \
        "$config" --bare-metal --kernel "$elf" "${extra_args[@]}" 2>&1)
    status=$?

    if [ "$status" -eq 124 ]; then
        printf 'FAIL %-18s (timeout)\n' "$name"
        fail=$((fail + 1))
        failed+=("$name")
    elif printf '%s\n' "$output" | grep -q '\[arm-am\] exit=0'; then
        printf 'PASS %-18s\n' "$name"
        pass=$((pass + 1))
    else
        printf 'FAIL %-18s (gem5=%d)\n' "$name" "$status"
        printf '%s\n' "$output" | tail -n 25 | sed 's/^/    /'
        fail=$((fail + 1))
        failed+=("$name")
    fi
done

printf '\ncputest (gem5): %d passed, %d failed, %d total\n' \
    "$pass" "$fail" "$((pass + fail))"
if [ "$fail" -ne 0 ]; then
    printf 'failed: %s\n' "${failed[*]}"
    exit 1
fi
