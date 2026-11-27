#!/usr/bin/env python3
# SPDX-License-Identifier: CC0-1.0
"""木の距離分布の単独貼付版に重心分解とNTTを同梱する。"""

import argparse
from pathlib import Path

try:
    from public_namespace import wrap_source, unwrap_source
except ModuleNotFoundError:
    from Scripts.public_namespace import wrap_source, unwrap_source


def render(root: Path) -> bytes:
    parts = root / "Graph/Tree_Distance_Frequency/components"
    header = (parts / "tree_distance_frequency_header.inc").read_text().rstrip()
    body = (parts / "tree_distance_frequency_body.inc").read_text().rstrip()
    overviews = []
    payloads = []
    for source, label in (
        ("Graph/Centroid_Decomposition.cpp", "重心分解"),
        ("Math/NTT.cpp", "NTT"),
    ):
        code = unwrap_source((root / source).read_text())
        include_start = code.index("#include ")
        intro, payload = code[:include_start], code[include_start:]
        overviews.append(f"//\n// [同梱{label}]\n" + intro.split("// API一覧\n", 1)[1].strip())
        payloads.append(payload.rstrip())
    full_header = "\n".join((header, *overviews))
    return wrap_source("\n\n".join((full_header, "#include <span>", *payloads, body)) + "\n").encode()


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--check", action="store_true")
    options = parser.parse_args()
    root = Path(__file__).resolve().parents[1]
    target = root / "Graph/Tree_Distance_Frequency.cpp"
    data = render(root)
    if options.check:
        if not target.is_file() or target.read_bytes() != data:
            raise ValueError("Tree_Distance_Frequency.cppが共通部品と同期していません")
    else:
        target.write_bytes(data)
    print("木の距離分布:", "同期確認済み" if options.check else "生成済み")


if __name__ == "__main__":
    main()
