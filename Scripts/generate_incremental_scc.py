#!/usr/bin/env python3
# SPDX-License-Identifier: CC0-1.0
"""オフラインSCCの単独貼付版にSCCとUnionFindを同梱する。"""

import argparse
from pathlib import Path

try:
    from public_namespace import wrap_source, unwrap_source
except ModuleNotFoundError:
    from Scripts.public_namespace import wrap_source, unwrap_source


def render(root: Path) -> bytes:
    parts = root / "Graph/Incremental_SCC/components"
    header = (parts / "incremental_scc_header.inc").read_text().rstrip()
    body = (parts / "incremental_scc_body.inc").read_text().rstrip()
    overviews = []
    payloads = []
    for source, label in (("Graph/SCC.cpp", "SCC"), ("DataStructure/union_find.cpp", "UnionFind")):
        code = unwrap_source((root / source).read_text())
        include_start = code.index("#include ")
        notes, overview = code[:include_start].split("// API一覧\n", 1)
        references = "\n".join(line for line in notes.splitlines() if "Source:" in line or "参考" in line)
        overviews.append("\n".join(part for part in (f"//\n// [同梱{label}]", references, overview.strip()) if part))
        payloads.append(code[include_start:].rstrip())
    full_header = "\n".join((header, *overviews))
    return wrap_source("\n\n".join((full_header, "#include <span>", *payloads, body)) + "\n").encode()


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--check", action="store_true")
    options = parser.parse_args()
    root = Path(__file__).resolve().parents[1]
    target = root / "Graph/Incremental_SCC.cpp"
    data = render(root)
    if options.check:
        if not target.is_file() or target.read_bytes() != data:
            raise ValueError("Incremental_SCC.cppが共通部品と同期していません")
    else:
        target.write_bytes(data)
    print("オフラインSCC:", "同期確認済み" if options.check else "生成済み")


if __name__ == "__main__":
    main()
