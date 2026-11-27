#!/usr/bin/env python3
# SPDX-License-Identifier: CC0-1.0
"""64ビット因数分解を使う原始根・二平方和の単独貼付版を生成する。"""
import argparse
from pathlib import Path

try:
    from public_namespace import unwrap_source, wrap_source
except ModuleNotFoundError:
    from Scripts.public_namespace import unwrap_source, wrap_source


def render(root: Path, library: str) -> bytes:
    source = unwrap_source((root / "Math/Pollard_Rho.cpp").read_text())
    include_start = source.index("#include ")
    notes, overview = source[:include_start].split("// API一覧\n", 1)
    notes = "\n".join(line for line in notes.strip().splitlines()[1:] if "ドキュメント" not in line)
    components = root / "Math" / library / "components"
    header = (components / "header.inc").read_text().rstrip()
    body = (components / "body.inc").read_text().rstrip()
    prerequisites, body = body.split("\n\n", 1)
    assembled = (
        f"{header}\n//\n// [同梱64ビット因数分解]\n{notes}\n{overview.strip()}\n\n"
        f"{prerequisites}\n{source[include_start:].rstrip()}\n\n{body}\n"
    )
    return wrap_source(assembled).encode()


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--check", action="store_true")
    options = parser.parse_args()
    root = Path(__file__).resolve().parents[1]
    for library in ("Primitive_Root", "Two_Square_Sum"):
        output = root / "Math" / (library + ".cpp")
        expected = render(root, library)
        if options.check:
            if not output.is_file() or output.read_bytes() != expected:
                raise ValueError(f"{output.name}が因数分解の正本と同期していません")
        else:
            output.write_bytes(expected)


if __name__ == "__main__":
    main()
