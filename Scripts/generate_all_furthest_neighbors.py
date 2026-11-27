#!/usr/bin/env python3
# SPDX-License-Identifier: CC0-1.0
"""各頂点の最遠点ライブラリへ既存のSMAWKを同梱する。"""
from pathlib import Path
import argparse

try:
    from public_namespace import wrap_source, unwrap_source
except ModuleNotFoundError:
    from Scripts.public_namespace import wrap_source, unwrap_source


def render(root: Path) -> dict[Path, bytes]:
    parts = root / "Geometry/All_Furthest_Neighbors/components"
    monge = unwrap_source((root / "Algorithm/Monge_Optimization.cpp").read_text())
    include_start = monge.index("#include ")
    shared_header = monge[:include_start].split("// API一覧\n", 1)[1].strip()
    header = (parts / "all_furthest_neighbors_header.inc").read_text().rstrip()
    body = (parts / "all_furthest_neighbors_body.inc").read_text().rstrip()
    content = f"{header}\n//\n// [同梱Monge最適化]\n{shared_header}\n\n{monge[include_start:].rstrip()}\n\n{body}\n"
    return {root / "Geometry/All_Furthest_Neighbors.cpp": wrap_source(content).encode()}


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--check", action="store_true")
    args = parser.parse_args()
    for path, data in render(Path(__file__).resolve().parents[1]).items():
        if args.check:
            if not path.is_file() or path.read_bytes() != data:
                raise ValueError(str(path) + "が共通部品と同期していません")
        else:
            path.write_bytes(data)


if __name__ == "__main__":
    main()
