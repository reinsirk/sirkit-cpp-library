#!/usr/bin/env python3
# SPDX-License-Identifier: CC0-1.0
"""Library Checker上位提出向けの、制約内のストレス入力を生成する。

第三者の提出コードは含めない。入力の妥当性は別途、公式validatorで確認する。
"""
from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path
import random


def write_graph(output: Path, left: int, right: int, edges: list[tuple[int, int]],
                matching: list[tuple[int, int]], shuffle_seed: int | None) -> dict:
    if shuffle_seed is not None:
        random.Random(shuffle_seed).shuffle(edges)
    with output.open("w", encoding="ascii", newline="\n") as stream:
        stream.write(f"{left} {right} {len(edges)}\n")
        for u, v in edges:
            stream.write(f"{u} {v}\n")
    output.with_suffix(".expected").write_text(
        f"{len(matching)}\n" + "".join(f"{u} {v}\n" for u, v in matching), encoding="ascii")
    return {"problem": "bipartitematching", "left": left, "right": right,
            "edges": len(edges), "maximum_matching_size": len(matching),
            "shuffle_seed": shuffle_seed}


def bipartite_shared(output: Path, left: int, right: int, active_left: int,
                     shuffle_seed: int | None = None) -> dict:
    """active_left個を全右頂点に接続し、残りの左頂点は孤立させる。"""
    if not (1 <= left <= 100_000 and 1 <= right <= 100_000 and 1 <= active_left <= left):
        raise ValueError("左右の頂点数は1..100000、active_leftは1..leftが必要です")
    if active_left * right > 200_000:
        raise ValueError("辺数は200000以下が必要です")
    edges = [(u, v) for u in range(active_left) for v in range(right)]
    matching = [(u, u) for u in range(min(active_left, right))]
    return write_graph(output, left, right, edges, matching, shuffle_seed)


def bipartite_chain(output: Path, vertices: int, shuffle_seed: int | None = None) -> dict:
    """完全マッチングを持ち、端点の処理順によって長い増加道ができる鎖。"""
    if not 1 <= vertices <= 100_000:
        raise ValueError("頂点数は1..100000が必要です")
    edges = [(u, v) for u in range(vertices - 1) for v in (u + 1, u)]
    edges.append((vertices - 1, 0))
    matching = [(u, u + 1) for u in range(vertices - 1)] + [(vertices - 1, 0)]
    return write_graph(output, vertices, vertices, edges, matching, shuffle_seed)


def bipartite_right_hub(output: Path, vertices: int, shuffle_seed: int | None = None) -> dict:
    """各左頂点の専用右頂点に加え、全左頂点に共通の右頂点を置く。"""
    if not 1 <= vertices <= 99_999:
        raise ValueError("頂点数は1..99999が必要です")
    edges = [(u, v) for u in range(vertices) for v in (0, u + 1)]
    matching = [(u, u + 1) for u in range(vertices)]
    return write_graph(output, vertices, vertices + 1, edges, matching, shuffle_seed)


def bipartite_hall_deficient(output: Path, vertices: int, shuffle_seed: int | None = None) -> dict:
    """孤立点なしでHall条件に1本不足し、共通右頂点に余剰が残る。"""
    if not 3 <= vertices <= 100_000:
        raise ValueError("頂点数は3..100000が必要です")
    leaves = vertices - 2
    edges = [(u, v) for u in range(leaves) for v in (0, u + 1)]
    edges.extend([(vertices - 2, vertices - 1), (vertices - 1, vertices - 1)])
    matching = [(u, u + 1) for u in range(leaves)] + [(vertices - 2, vertices - 1)]
    return write_graph(output, vertices, vertices, edges, matching, shuffle_seed)


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("case", choices=["bipartite_shared", "bipartite_chain", "bipartite_right_hub",
                                        "bipartite_hall_deficient"])
    parser.add_argument("output", type=Path)
    parser.add_argument("--left", type=int, default=1)
    parser.add_argument("--right", type=int, default=100_000)
    parser.add_argument("--active-left", type=int, default=1)
    parser.add_argument("--shuffle-seed", type=int)
    args = parser.parse_args()
    args.output.parent.mkdir(parents=True, exist_ok=True)
    if args.output.suffix != ".in":
        parser.error("outputは.inファイルを指定してください")
    if args.case == "bipartite_shared":
        metadata = bipartite_shared(args.output, args.left, args.right, args.active_left, args.shuffle_seed)
    elif args.case == "bipartite_chain":
        metadata = bipartite_chain(args.output, args.left, args.shuffle_seed)
    elif args.case == "bipartite_right_hub":
        metadata = bipartite_right_hub(args.output, args.left, args.shuffle_seed)
    else:
        metadata = bipartite_hall_deficient(args.output, args.left, args.shuffle_seed)
    metadata.update(input_sha256=hashlib.sha256(args.output.read_bytes()).hexdigest(),
                    expected_sha256=hashlib.sha256(args.output.with_suffix(".expected").read_bytes()).hexdigest())
    args.output.with_suffix(".json").write_text(json.dumps(metadata, indent=2) + "\n")
    print(json.dumps(metadata))


if __name__ == "__main__":
    main()
