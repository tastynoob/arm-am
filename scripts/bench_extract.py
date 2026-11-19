#!/usr/bin/env python3
"""Extract BENCH_FORMAT lines from benchmark/gem5 logs.

The canonical record is:

    BENCH_FORMAT test=<app>/<test> ipc=<x.xx> [field=value ...]

`test` and `ipc` are mandatory; all other fields are caller-defined.  The
default output is TSV with a header.  `--json` emits one object per record.
Values are whitespace-delimited (`key=value`); values cannot contain spaces.
"""

from __future__ import annotations

import argparse
import json
import math
import re
import sys
from pathlib import Path
from typing import Iterable, TextIO

RECORD_RE = re.compile(r"(?:^|\s)BENCH_FORMAT\s+(.+?)\s*$")


class RecordError(ValueError):
    pass


def parse_record(payload: str) -> tuple[dict[str, str], float]:
    fields: dict[str, str] = {}
    for token in payload.split():
        if "=" not in token:
            raise RecordError(f"field without '=': {token!r}")
        key, value = token.split("=", 1)
        if not key:
            raise RecordError("empty field name")
        if key in fields:
            raise RecordError(f"duplicate field {key!r}")
        fields[key] = value

    for required in ("test", "ipc"):
        if required not in fields:
            raise RecordError(f"missing required field {required!r}")
    if not fields["test"]:
        raise RecordError("empty test field")

    try:
        ipc = float(fields["ipc"])
    except ValueError as exc:
        raise RecordError(f"invalid ipc {fields['ipc']!r}") from exc
    if not math.isfinite(ipc):
        raise RecordError(f"non-finite ipc {fields['ipc']!r}")

    return fields, ipc


def iter_sources(paths: list[str]) -> Iterable[tuple[str, TextIO]]:
    if not paths:
        paths = ["-"]
    for name in paths:
        if name == "-":
            yield "<stdin>", sys.stdin
        else:
            try:
                handle = open(name, "r", encoding="utf-8", errors="replace")
            except OSError as exc:
                raise RecordError(f"{name}: {exc}") from exc
            yield name, handle


def collect(paths: list[str]) -> tuple[list[dict[str, str]], list[float]]:
    rows: list[dict[str, str]] = []
    ipcs: list[float] = []
    errors: list[str] = []

    for name, handle in iter_sources(paths):
        try:
            for lineno, line in enumerate(handle, 1):
                match = RECORD_RE.search(line)
                if not match:
                    continue
                try:
                    fields, ipc = parse_record(match.group(1))
                except RecordError as exc:
                    errors.append(f"{name}:{lineno}: {exc}")
                    continue
                rows.append(fields)
                ipcs.append(ipc)
        finally:
            if name != "<stdin>":
                handle.close()

    if errors:
        for error in errors:
            print(f"bench_extract: {error}", file=sys.stderr)
    if not rows and not errors:
        errors.append("no BENCH_FORMAT records found")
        print("bench_extract: no BENCH_FORMAT records found", file=sys.stderr)

    return rows, ipcs


def emit_tsv(rows: list[dict[str, str]], ipcs: list[float],
             out: TextIO, header: bool) -> None:
    columns = ["test", "ipc"]
    for row in rows:
        for key in row:
            if key not in columns:
                columns.append(key)

    if header:
        print("\t".join(columns), file=out)
    for row, ipc in zip(rows, ipcs):
        vals = []
        for column in columns:
            if column == "ipc":
                vals.append(f"{ipc:.2f}")
            else:
                vals.append(row.get(column, ""))
        print("\t".join(vals), file=out)


def emit_json(rows: list[dict[str, str]], ipcs: list[float],
              out: TextIO) -> None:
    data = []
    for row, ipc in zip(rows, ipcs):
        item: dict[str, object] = dict(row)
        item["ipc"] = ipc
        data.append(item)
    json_text = json.dumps(data, indent=2, ensure_ascii=False)
    print(json_text, file=out)


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(
        description="Extract canonical BENCH_FORMAT records.")
    parser.add_argument("files", nargs="*", default=["-"],
                        help="log files; '-' or no files reads stdin")
    parser.add_argument("--json", action="store_true",
                        help="emit JSON instead of TSV")
    parser.add_argument("--no-header", action="store_true",
                        help="omit the TSV header")
    parser.add_argument("-o", "--output", metavar="FILE",
                        help="write output to FILE instead of stdout")
    args = parser.parse_args(argv)

    try:
        rows, ipcs = collect(args.files)
    except RecordError as exc:
        print(f"bench_extract: {exc}", file=sys.stderr)
        return 1

    if not rows:
        return 1

    out: TextIO = sys.stdout
    close_out = False
    if args.output:
        try:
            out = open(args.output, "w", encoding="utf-8")
        except OSError as exc:
            print(f"bench_extract: {exc}", file=sys.stderr)
            return 1
        close_out = True
    try:
        if args.json:
            emit_json(rows, ipcs, out)
        else:
            emit_tsv(rows, ipcs, out, header=not args.no_header)
    finally:
        if close_out:
            out.close()
    return 0


if __name__ == "__main__":
    sys.exit(main())
