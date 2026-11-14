#!/usr/bin/env python3
# SPDX-License-Identifier: CC0-1.0
"""通常版の代表本体を、同じ計測コードで基準commitと交互比較する。速度差は参考値。"""
from __future__ import annotations

import argparse
import hashlib
import html
import json
import os
from pathlib import Path
import platform
import re
import shutil
import statistics
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[1]
FIXTURES = ROOT / "Verify/ci/benchmarks"
LIBRARIES = {
    "fenwick": "DataStructure/Fenwick_Tree.cpp",
    "union_find": "DataStructure/union_find.cpp",
    "rolling_hash": "String/RollingHash.cpp",
}
FLAGS = ["-std=c++23", "-O2", "-DNDEBUG", "-DSKIP_RUNTIME_CHECKS"]


class BenchmarkError(ValueError):
    pass


def digest(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def command(args: list[str], *, cwd: Path = ROOT, timeout: int = 180) -> subprocess.CompletedProcess:
    try:
        result = subprocess.run(args, cwd=cwd, capture_output=True, timeout=timeout)
    except (OSError, subprocess.TimeoutExpired) as error:
        raise BenchmarkError(f"コマンドを完了できません: {args[0]}: {error}") from error
    if result.returncode:
        diagnostic = result.stderr.decode("utf-8", "replace")[-16000:]
        raise BenchmarkError(f"コマンド失敗({result.returncode}): {args[0]}\n{diagnostic}")
    return result


def resolve_base(ref: str, root: Path = ROOT) -> str:
    resolved = command(["git", "rev-parse", "--verify", "--end-of-options", ref + "^{commit}"],
                       cwd=root, timeout=15).stdout.decode().strip()
    if not re.fullmatch(r"(?:[0-9a-f]{40}|[0-9a-f]{64})", resolved):
        raise BenchmarkError("基準をcommitへ解決できません")
    return resolved


def baseline_source(revision: str, path: str, root: Path = ROOT) -> bytes | None:
    # 先に存在を調べ、欠落とgit自体の失敗を区別する。実行用スクリプトは読まない。
    tree = command(["git", "ls-tree", "-z", revision, "--", path], cwd=root, timeout=15).stdout
    if not tree:
        return None
    fields = tree.rstrip(b"\0").split(b"\t", 1)
    if len(fields) != 2 or fields[1].decode() != path or fields[0].split()[1] != b"blob":
        raise BenchmarkError("基準のライブラリが通常ファイルではありません: " + path)
    return command(["git", "show", revision + ":" + path], cwd=root, timeout=15).stdout


def compiler_command(compiler: str) -> list[str]:
    path = shutil.which(compiler)
    if not path:
        raise BenchmarkError("コンパイラがありません: " + compiler)
    result = [path]
    if "clang" in Path(path).name:
        gcc = shutil.which("g++")
        if gcc:
            library = Path(command([gcc, "-print-libgcc-file-name"], timeout=15).stdout.decode().strip())
            if library.is_absolute() and library.name == "libgcc.a" and library.is_file():
                result.append("--gcc-install-dir=" + str(library.parent))
    return result


def build(compiler: list[str], library: bytes, fixture: bytes, common: bytes,
          directory: Path, name: str) -> Path:
    directory.mkdir(parents=True, exist_ok=True)
    (directory / "library.hpp").write_bytes(library)
    (directory / "benchmark.cpp").write_bytes(fixture)
    (directory / "benchmark_common.hpp").write_bytes(common)
    executable = directory / name
    command([*compiler, *FLAGS, str(directory / "benchmark.cpp"), "-o", str(executable)])
    return executable


def parse_result(output: bytes) -> dict:
    try:
        value = json.loads(output)
    except (ValueError, UnicodeError) as error:
        raise BenchmarkError("計測結果がJSONではありません") from error
    if (not isinstance(value, dict) or type(value.get("elapsed_ns")) is not int
            or value["elapsed_ns"] <= 0):
        raise BenchmarkError("計測時間が不正です")
    for field in ("checksum", "expected"):
        text = value.get(field)
        if not isinstance(text, str) or not re.fullmatch(r"[0-9]{1,20}", text) or int(text) >= 1 << 64:
            raise BenchmarkError("照合値が不正です")
    if value["checksum"] != value["expected"]:
        raise BenchmarkError("独立参照と計測結果が一致しません")
    return value


def measure(executables: dict[str, Path], size: int, pattern: int, rounds: int,
            repetitions: int, warmups: int) -> dict:
    samples: dict[str, list[int]] = {stage: [] for stage in executables}
    checksum = None
    orders = []
    for repetition in range(warmups + repetitions):
        stages = list(executables)
        if repetition % 2:
            stages.reverse()
        orders.append(stages)
        for stage in stages:
            raw = command([str(executables[stage]), str(size), str(pattern), str(rounds)], timeout=90).stdout
            result = parse_result(raw)
            if checksum is not None and checksum != result["checksum"]:
                raise BenchmarkError("基準・現在版・反復の間で結果が一致しません")
            checksum = result["checksum"]
            if repetition >= warmups:
                samples[stage].append(result["elapsed_ns"])
    median = {stage: statistics.median(values) for stage, values in samples.items()}
    return {"status": "MEASURED" if "base" in executables else "NO_BASELINE",
            "size": size, "pattern": "random" if pattern == 0 else "structured",
            "rounds": rounds, "seed": 20261114, "checksum": checksum,
            "samples_ns": samples, "median_ns": median,
            "current_over_base": median["current"] / median["base"] if "base" in median else None,
            "orders": orders, "independent_reference": "passed"}


def code(value: str) -> str:
    return "<code>" + html.escape(" ".join(value.split())[:120]).replace("|", "&#124;").replace("`", "&#96;") + "</code>"


def markdown(report: dict) -> str:
    lines = ["## 本体の性能比較（参考値）", "",
             "同じ計測コード・コンパイラ・O2・検査省略設定で、基準と現在版を交互に測定します。",
             "入力生成・独立参照・プロセス起動・入出力は時間に含めません。本体の構築・操作・破棄と同じchecksum集約を含みます。",
             "Verifyのmainやオンライン提出時間の比較ではありません。速度差だけでは不合格にしません。", "",
             "状態: " + str(report["status"])]
    if report.get("base_revision"):
        lines += ["", "基準: " + code(report["base_revision"])]
    lines += ["", "| 本体 | 入力 | 要素数 | 基準 ms | 現在 ms | 現在/基準 |",
              "| --- | --- | ---: | ---: | ---: | ---: |"]
    for row in report.get("results", []):
        median = row["median_ns"]
        base = f"{median['base'] / 1e6:.3f}" if "base" in median else "N/A"
        ratio = f"{row['current_over_base']:.3f}" if row["current_over_base"] is not None else "N/A"
        lines.append(f"| {row['id']} | {row['pattern']} | {row['size']} | {base} | {median['current']/1e6:.3f} | {ratio} |")
    if any(row["status"] == "NO_BASELINE" for row in report.get("results", [])):
        lines += ["", "N/Aは基準commitに本体がないため比較未実施です。現在版の計測と独立照合だけを行いました。"]
    if report["status"] == "FAIL":
        lines += ["", "ビルド・照合・計測のいずれかが失敗しました。詳細はJSONを確認してください。"]
    return "\n".join(lines) + "\n"


def write_report(output: Path, report: dict) -> None:
    temporary = output / "result.json.tmp"
    temporary.write_text(json.dumps(report, ensure_ascii=False, indent=2) + "\n")
    temporary.replace(output / "result.json")
    (output / "summary.md").write_text(markdown(report))


def cpu_model() -> str:
    try:
        for line in Path("/proc/cpuinfo").read_text().splitlines():
            if line.startswith("model name"):
                return line.split(":", 1)[1].strip()
    except (OSError, IndexError):
        pass
    return platform.processor() or "unknown"


def run(args: argparse.Namespace) -> int:
    output = args.output.resolve()
    output.mkdir(parents=True, exist_ok=True)
    report = {"format": 1, "status": "running", "scope": "representative_normal_library_kernels",
              "timing_scope": "library construction, operations, destruction and shared digest only",
              "online_status": "not_evaluated", "flags": FLAGS,
              "repetitions": args.repetitions, "warmups": args.warmups,
              "platform": platform.platform(), "machine": platform.machine(), "cpu_model": cpu_model(),
              "affinity": sorted(os.sched_getaffinity(0)) if hasattr(os, "sched_getaffinity") else None,
              "results": [], "sources": {}}
    write_report(output, report)
    try:
        revision = resolve_base(args.base)
        report["base_revision"] = revision
        compiler = compiler_command(args.compiler)
        report["compiler"] = {"command": compiler,
                              "version": command([compiler[0], "--version"], timeout=15).stdout.decode().splitlines()[0]}
        common = (FIXTURES / "benchmark_common.hpp").read_bytes()
        selected = args.id or list(LIBRARIES)
        sizes = [2048] if args.quick else [1024, 262144]
        with tempfile.TemporaryDirectory(prefix="sirkit-benchmark-") as temporary:
            directory = Path(temporary)
            for identifier in selected:
                path = LIBRARIES[identifier]
                current = (ROOT / path).read_bytes()
                base = baseline_source(revision, path)
                fixture = (FIXTURES / (identifier + ".cpp")).read_bytes()
                snapshots = output / "sources" / identifier
                snapshots.mkdir(parents=True, exist_ok=True)
                (snapshots / "current-library.hpp").write_bytes(current)
                (snapshots / "benchmark.cpp").write_bytes(fixture)
                (snapshots / "benchmark_common.hpp").write_bytes(common)
                base_snapshot = snapshots / "base-library.hpp"
                if base is not None:
                    base_snapshot.write_bytes(base)
                elif base_snapshot.exists():
                    base_snapshot.unlink()
                report["sources"][identifier] = {
                    "path": path, "snapshots": str(snapshots.relative_to(output)), "current_sha256": digest(current),
                    "base_sha256": digest(base) if base is not None else None,
                    "harness_sha256": digest(fixture), "common_harness_sha256": digest(common)}
                # 同一のソースパスを再利用し、時点ごとに実行ファイルだけを保持する。
                executables = {}
                if base is not None:
                    executables["base"] = build(compiler, base, fixture, common,
                                                directory / identifier, "base")
                executables["current"] = build(compiler, current, fixture, common,
                                               directory / identifier, "current")
                for size in sizes:
                    rounds = 1 if args.quick else max(1, 262144 // size)
                    for pattern in (0, 1):
                        row = measure(executables, size, pattern, rounds, args.repetitions, args.warmups)
                        row["id"] = identifier
                        report["results"].append(row)
                        write_report(output, report)
        compared = sum(row["status"] == "MEASURED" for row in report["results"])
        report["compared_cases"] = compared
        report["uncompared_cases"] = len(report["results"]) - compared
        report["current_measurements_passed"] = bool(report["results"])
        report["status"] = "NO_BASELINE" if not compared else "PARTIAL" if report["uncompared_cases"] else "PASS"
        write_report(output, report)
        print(report["status"] + ": " + str(output / "summary.md"))
        return 2 if not compared else 0
    except (BenchmarkError, OSError, ValueError, KeyError) as error:
        report["status"] = "FAIL"
        report["error"] = str(error)
        write_report(output, report)
        print(str(error), file=sys.stderr)
        return 1


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--base", required=True, help="基準commit/ref（履歴から本体だけを読み出す）")
    parser.add_argument("--compiler", default="g++")
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--id", action="append", choices=tuple(LIBRARIES))
    parser.add_argument("--quick", action="store_true", help="小規模の動作確認。性能判定には使わない")
    parser.add_argument("--repetitions", type=int, default=7)
    parser.add_argument("--warmups", type=int, default=2)
    args = parser.parse_args(argv)
    if not 1 <= args.repetitions <= 100 or not 0 <= args.warmups <= 20:
        parser.error("repetitionsは1..100、warmupsは0..20")
    if args.id:
        args.id = list(dict.fromkeys(args.id))
    return run(args)


if __name__ == "__main__":
    raise SystemExit(main())
