#!/usr/bin/env python3
# SPDX-License-Identifier: CC0-1.0
"""本体込みの提出コードを、公式入力例と独立した逐次参照で確認する。"""
import argparse
import hashlib
import importlib.util
import json
from pathlib import Path
import random
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[3]
SPEC = importlib.util.spec_from_file_location("sirkit_prepare_verify", ROOT / "Scripts/prepare_verify.py")
PREPARE = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(PREPARE)
ENTRIES = [
    {"id": "segment_tree_beats", "libraries": ["DataStructure/Segment_Tree_Beats.cpp"],
     "driver": "Verify/current/drivers/segment_tree_beats_main.cpp",
     "problem_url": "https://judge.yosupo.jp/problem/range_chmin_chmax_add_range_sum"},
    {"id": "sliding_window_aggregation", "libraries": ["DataStructure/Sliding_Window_Aggregation.cpp"],
     "driver": "Verify/current/drivers/sliding_window_aggregation_main.cpp",
     "problem_url": "https://judge.yosupo.jp/problem/queue_operate_all_composite"},
    {"id": "xor_basis", "libraries": ["Math/Xor_Basis.cpp"],
     "driver": "Verify/current/drivers/xor_basis_main.cpp",
     "problem_url": "https://atcoder.jp/contests/abc141/tasks/abc141_f"},
]
# Library Checker公式 example_00.in（コミット73876f1cbcf082d2b22722924b924d3d65a2f00e）。
# 出力は下の逐次参照と同じ定義で独立計算。AtCoderは公式入力例1・2。
SAMPLES = {
    "segment_tree_beats": [("5 7\n1 2 3 4 5\n3 0 5\n2 2 4 100\n3 0 3\n0 1 3 10\n3 2 5\n1 2 5 20\n3 0 5\n", "15\n106\n119\n147\n")],
    "sliding_window_aggregation": [("6\n0 4 5\n2 3\n0 2 1\n2 2\n1\n2 3\n", "17\n27\n7\n")],
    "xor_basis": [("3\n3 6 5\n", "12\n"), ("4\n23 36 66 65\n", "188\n")],
}


def random_beats(rng):
    n, q = rng.randrange(1, 70), 350
    values = [rng.randrange(-1000000, 1000001) for _ in range(n)]
    lines = [f"{n} {q}", " ".join(map(str, values))]
    output = []
    for _ in range(q):
        kind = rng.randrange(4)
        l, r = sorted([rng.randrange(n + 1), rng.randrange(n + 1)])
        if kind == 3:
            lines.append(f"3 {l} {r}")
            output.append(str(sum(values[l:r])))
        else:
            x = rng.randrange(-1000000, 1000001)
            lines.append(f"{kind} {l} {r} {x}")
            for i in range(l, r):
                if kind == 0:
                    values[i] = min(values[i], x)
                elif kind == 1:
                    values[i] = max(values[i], x)
                else:
                    values[i] += x
    return "\n".join(lines) + "\n", "\n".join(output) + "\n"


def random_swag(rng):
    modulus, q = 998244353, 500
    queue, lines, output = [], [str(q)], []
    for _ in range(q):
        kind = rng.randrange(3)
        if kind == 1 and not queue:
            kind = 0
        if kind == 0:
            a, b = rng.randrange(modulus), rng.randrange(modulus)
            lines.append(f"0 {a} {b}")
            queue.append((a, b))
        elif kind == 1:
            lines.append("1")
            queue.pop(0)
        else:
            x = rng.randrange(modulus)
            lines.append(f"2 {x}")
            for a, b in queue:
                x = (a * x + b) % modulus
            output.append(str(x))
    return "\n".join(lines) + "\n", "\n".join(output) + "\n"


def random_xor(rng):
    n = rng.randrange(2, 16)
    values = [rng.getrandbits(rng.choice([1, 7, 25, 60])) for _ in range(n)]
    total = 0
    for value in values:
        total ^= value
    maximum = 0
    # 赤側も青側も非空として全部分集合を列挙する。
    for mask in range(1, (1 << n) - 1):
        left = 0
        for i, value in enumerate(values):
            if mask >> i & 1:
                left ^= value
        maximum = max(maximum, left + (left ^ total))
    return f"{n}\n" + " ".join(map(str, values)) + "\n", f"{maximum}\n"


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--compiler", default="/usr/bin/g++")
    parser.add_argument("--compiler-arg", action="append", default=[])
    parser.add_argument("--portable", action="store_true")
    parser.add_argument("--checked", action="store_true")
    parser.add_argument("--native", action="store_true")
    parser.add_argument("--report", type=Path)
    args = parser.parse_args()
    report = {"compiler": args.compiler, "flags": args.compiler_arg, "portable": args.portable,
              "checked": args.checked, "native": args.native, "entries": []}
    rng = random.Random(20261031)
    with tempfile.TemporaryDirectory(prefix="sirkit-beats-swag-") as directory:
        directory = Path(directory)
        for entry, generate in zip(ENTRIES, [random_beats, random_swag, random_xor]):
            data, hashes = PREPARE.render(entry, skip_runtime_checks=not args.checked)
            source, binary = directory / (entry["id"] + ".cpp"), directory / entry["id"]
            source.write_bytes(data)
            flags = ["-std=c++23", "-O2", *args.compiler_arg]
            if args.portable:
                flags.append("-DSIRKIT_FORCE_STANDARD_ARITHMETIC")
            if args.native:
                flags.append("-march=native")
            command = [args.compiler, *flags, str(source), "-o", str(binary)]
            subprocess.run(command, check=True)
            cases = SAMPLES[entry["id"]] + [generate(rng) for _ in range(100)]
            for input_text, expected in cases:
                result = subprocess.run([str(binary)], input=input_text, text=True, capture_output=True, check=True)
                assert result.stdout.split() == expected.split(), (entry["id"], input_text, expected, result.stdout)
            record = {"id": entry["id"], "source_hashes": hashes, "submission_sha256": hashlib.sha256(data).hexdigest(),
                      "cases": len(cases), "random_seed": 20261031, "compile_command": command}
            report["entries"].append(record)
            print(entry["id"], len(cases), "official/independent driver cases OK", flush=True)
    if args.report:
        args.report.write_text(json.dumps(report, ensure_ascii=False, indent=2) + "\n")


if __name__ == "__main__":
    main()
