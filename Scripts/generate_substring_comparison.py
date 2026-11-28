#!/usr/bin/env python3
# SPDX-License-Identifier: CC0-1.0
"""部分文字列比較の単独貼付版へ接尾辞配列と静的区間最小値を同梱する。"""
from pathlib import Path
import argparse

try:
    from public_namespace import wrap_source, unwrap_source
except ModuleNotFoundError:
    from Scripts.public_namespace import wrap_source, unwrap_source


def render(root: Path) -> bytes:
    parts = root / "String/Substring_Comparison/components"
    header = [(parts / "header.inc").read_text().rstrip()]
    includes = []
    bodies = []
    for label, path in (
        ("接尾辞配列", root / "String/Suffix_Array.cpp"),
        ("静的区間最小値", root / "DataStructure/Static_Range_Minimum.cpp"),
        (None, parts / "body.inc"),
    ):
        source = unwrap_source(path.read_text())
        include_start = source.index("#include ")
        guard_start = source.index("#ifndef SIRKIT_", include_start)
        if label is not None:
            overview = source[:include_start].split("// API一覧\n", 1)[1].strip()
            header.extend(("//", f"// [同梱の{label}]", overview))
        includes.extend(source[include_start:guard_start].strip().splitlines())
        bodies.append(source[guard_start:].rstrip())
    return wrap_source(
        "\n".join(header) + "\n" + "\n".join(dict.fromkeys(includes)) + "\n\n" + "\n\n".join(bodies) + "\n"
    ).encode()


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--check", action="store_true")
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[1]
    path = root / "String/Substring_Comparison.cpp"
    data = render(root)
    if args.check:
        if not path.is_file() or path.read_bytes() != data:
            raise ValueError("Substring_Comparison.cppが共通部品と同期していません")
    else:
        path.write_bytes(data)
    print("部分文字列比較: " + ("同期確認済み" if args.check else "生成済み"))


if __name__ == "__main__":
    main()
