#!/usr/bin/env python3
# SPDX-License-Identifier: CC0-1.0
"""外部保存した二部マッチング提出を、独立参照と公式検査器で照合する。

source_dirには公開APIのranking.json・提出番号.cpp・bin/提出番号を用意する。
実行する第三者ソースは事前に確認し、通常権限でこのスクリプトを使う。
ネットワーク取得、ソースの書換え、オンライン判定の変更は行わない。
"""
from __future__ import annotations

import argparse
import hashlib
import json
import os
from pathlib import Path
import random
import resource
import subprocess
import time

from generate_fastest_counterexamples import bipartite_chain, bipartite_shared


def matching(left: int, right: int, edges: list[tuple[int, int]]) -> list[tuple[int, int]]:
    """小グラフ用。増加道を1本ずつ探す独立参照。"""
    graph = [[] for _ in range(left)]
    for u, v in edges:
        graph[u].append(v)
    mate = [-1] * right

    def augment(u: int, seen: list[bool]) -> bool:
        for v in graph[u]:
            if seen[v]:
                continue
            seen[v] = True
            if mate[v] < 0 or augment(mate[v], seen):
                mate[v] = u
                return True
        return False

    for u in range(left):
        augment(u, [False] * right)
    return [(u, v) for v, u in enumerate(mate) if u >= 0]


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("source_dir", type=Path)
    parser.add_argument("validator", type=Path)
    parser.add_argument("checker", type=Path)
    parser.add_argument("output", type=Path)
    parser.add_argument("--mode", choices=["small", "large"], required=True)
    parser.add_argument("--cpu", type=int, default=2)
    parser.add_argument("--timeout", type=float, default=8)
    parser.add_argument("--seed", type=int, default=20261116)
    args = parser.parse_args()
    source_dir, output = args.source_dir.resolve(), args.output.resolve()
    validator, checker = args.validator.resolve(), args.checker.resolve()
    output.mkdir(parents=True, exist_ok=True)
    os.sched_setaffinity(0, {args.cpu})
    soft, hard = resource.getrlimit(resource.RLIMIT_STACK)
    stack = 1 << 30
    if hard != resource.RLIM_INFINITY:
        stack = min(stack, hard)
    resource.setrlimit(resource.RLIMIT_STACK, (stack, hard))
    ranking = json.loads((source_dir / "ranking.json").read_text())["submissions"]
    groups: dict[str, list[int]] = {}
    for row in ranking:
        number = row["id"]
        digest = hashlib.sha256((source_dir / f"{number}.cpp").read_bytes()).hexdigest()
        groups.setdefault(digest, []).append(number)
    rng = random.Random(args.seed)
    cases: list[tuple[str, dict]] = []
    if args.mode == "large":
        for left in [1, 2, 8, 64, 1024, 100_000]:
            cases.append((f"one-star-L{left}", dict(left=left, right=100_000, active_left=1)))
        for left in [2, 8, 64, 1024, 100_000]:
            cases.append((f"two-shared-L{left}", dict(left=left, right=100_000, active_left=2)))
        for left in [4, 16, 64, 256, 4096, 100_000]:
            cases.append((f"complete-L{left}", dict(left=left, right=200_000 // left, active_left=left)))
        cases.extend([("chain-ordered", dict(vertices=100_000, ordered=True)),
                      ("chain-shuffled", dict(vertices=100_000))])
    else:
        for mask in range(1, 1 << 9):
            edges = [(u, v) for u in range(3) for v in range(3) if mask & (1 << (u * 3 + v))]
            cases.append((f"exhaustive-3x3-{mask}", dict(left=3, right=3, edges=edges)))
        for index in range(200):
            left, right = rng.randrange(1, 45), rng.randrange(1, 65)
            density = rng.choice([0.01, 0.05, 0.2, 0.5, 0.8, 1.0])
            edges = [(u, v) for u in range(left) for v in range(right) if rng.random() < density]
            if not edges:
                edges = [(rng.randrange(left), rng.randrange(right))]
            rng.shuffle(edges)
            cases.append((f"random-{index}", dict(left=left, right=right, edges=edges)))
    results = {"mode": args.mode, "seed": args.seed, "cpu": args.cpu,
               "timeout_seconds": args.timeout, "stack_bytes": stack,
               "source_groups": groups, "ranking_ids": [row["id"] for row in ranking],
               "note": "local timeout is not an official TLE verdict", "cases": []}
    for label, config in cases:
        inp = output / f"{label}.in"
        expected = inp.with_suffix(".expected")
        if args.mode == "small":
            left, right, edges = config["left"], config["right"], config["edges"]
            optimum = matching(left, right, edges)
            inp.write_text(f"{left} {right} {len(edges)}\n" + "".join(f"{u} {v}\n" for u, v in edges))
            expected.write_text(f"{len(optimum)}\n" + "".join(f"{u} {v}\n" for u, v in optimum))
        elif label.startswith("chain"):
            bipartite_chain(inp, config["vertices"], None if config.get("ordered") else args.seed)
        else:
            bipartite_shared(inp, **config, shuffle_seed=args.seed)
        with inp.open("rb") as stream:
            valid = subprocess.run([str(validator)], stdin=stream, capture_output=True, timeout=10)
        if valid.returncode:
            raise RuntimeError(f"invalid input {label}: {valid.stderr!r}")
        case = {"label": label, "input_sha256": hashlib.sha256(inp.read_bytes()).hexdigest(),
                "header": inp.open().readline().strip(), "validator_exit": valid.returncode, "runs": []}
        for digest, numbers in groups.items():
            actual = output / f"{label}-{numbers[0]}.actual"
            before = resource.getrusage(resource.RUSAGE_CHILDREN)
            start = time.monotonic()
            try:
                with inp.open("rb") as stream, actual.open("wb") as target:
                    run = subprocess.run([str(source_dir / "bin" / str(numbers[0]))], stdin=stream,
                                         stdout=target, stderr=subprocess.PIPE, timeout=args.timeout)
                elapsed = time.monotonic() - start
                after = resource.getrusage(resource.RUSAGE_CHILDREN)
                checked = subprocess.run([str(checker), str(inp), str(actual), str(expected)],
                                         capture_output=True, timeout=10)
                status = "pass" if run.returncode == 0 and checked.returncode == 0 else "failure"
                detail = {"returncode": run.returncode, "checker_exit": checked.returncode,
                          "stderr": run.stderr.decode(errors="replace")[:1000],
                          "checker_stderr": checked.stderr.decode(errors="replace")[:1000]}
            except subprocess.TimeoutExpired:
                elapsed = time.monotonic() - start
                after = resource.getrusage(resource.RUSAGE_CHILDREN)
                status, detail = "local_timeout", {}
            row = {"ids": numbers, "source_sha256": digest, "status": status, "seconds": elapsed,
                   "user_seconds": after.ru_utime - before.ru_utime,
                   "system_seconds": after.ru_stime - before.ru_stime, **detail}
            case["runs"].append(row)
            if status != "pass":
                print(label, numbers, status, round(elapsed, 3), flush=True)
        results["cases"].append(case)
        (output / "results.json").write_text(json.dumps(results, indent=2) + "\n")
        if args.mode == "large" or len(results["cases"]) % 100 == 0:
            print("completed", len(results["cases"]), "/", len(cases), label, flush=True)
    print("DONE", len(cases), "inputs", len(ranking), "submissions", flush=True)


if __name__ == "__main__":
    main()
