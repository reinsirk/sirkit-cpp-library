#!/usr/bin/env python3
# SPDX-License-Identifier: CC0-1.0
"""Sync the standalone PersistentArray into the paste-ready PersistentUnionFind."""

import argparse
from pathlib import Path


START = "#ifndef SIRKIT_PERSISTENT_ARRAY_DEFINED\n"
ARRAY_END = "\n} // namespace sirkit"
UNION_FIND = "#ifndef SIRKIT_PERSISTENT_UNION_FIND_DEFINED\n"


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--check", action="store_true")
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[1] / "DataStructure"
    array = (root / "Persistent_Array.cpp").read_text(encoding="utf-8")
    target = root / "persistent-union-find.cpp"
    bundled = target.read_text(encoding="utf-8")
    start = array.index(START)
    end = array.index(ARRAY_END, start)
    body = array[start:end].rstrip()
    former_start = bundled.index(START)
    former_end = bundled.index(UNION_FIND, former_start)
    generated = bundled[:former_start] + body + "\n\n" + bundled[former_end:]
    if args.check:
        if generated != bundled:
            raise ValueError("PersistentUnionFindの同梱PersistentArrayが同期していません")
    else:
        target.write_text(generated, encoding="utf-8")


if __name__ == "__main__":
    main()
