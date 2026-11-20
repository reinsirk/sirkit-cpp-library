#!/usr/bin/env python3
# SPDX-License-Identifier: CC0-1.0
"""通常版の代表本体を計測し、基準commitからの退行を検査する。"""
from __future__ import annotations

import argparse
from collections.abc import Sequence
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

try:
    from . import ci_compile_profiles, performance_limits, performance_regression
except ImportError:
    import ci_compile_profiles
    import performance_limits
    import performance_regression

ROOT = Path(__file__).resolve().parents[1]
FIXTURES = ROOT / "Verify/ci/benchmarks"
LIBRARIES = {
    "fenwick": "DataStructure/Fenwick_Tree.cpp",
    "union_find": "DataStructure/union_find.cpp",
    "rolling_hash": "String/RollingHash.cpp",
}
FLAGS = ("-std=c++23", "-O2", "-DNDEBUG", "-DSKIP_RUNTIME_CHECKS")
# ライブラリの上限・オンライン問題の最大制約とは別の、CIで扱う規模上限。
STRESS_SIZE = 2_000_000
SCENARIOS = {
    "fenwick": ("random", "prefix", "empty_full_lowbit", "alternating_updates"),
    "union_find": ("random", "sequential", "balanced_before_compression", "redundant_disconnected"),
    "rolling_hash": ("random", "identical", "long_lcp_last_mismatch", "binary_bytes_boundaries"),
}


def case_plan(identifier: str, quick: bool) -> list[dict]:
    sizes = [(2048, "smoke")] if quick else [(1024, "small"), (262144, "large"),
                                                           (STRESS_SIZE, "ci_stress_limit")]
    cases = [{"size": size, "scale": scale, "pattern_id": pattern, "scenario": name}
             for size, scale in sizes for pattern, name in enumerate(SCENARIOS[identifier])]
    if not quick:
        # 冪ちょうどだけでなく、その直前・直後も同じ難ケースで照合する。
        cases += [{"size": size, "scale": "power_of_two_boundary", "pattern_id": 2,
                   "scenario": SCENARIOS[identifier][2]} for size in (262143, 262145)]
    return cases


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
          directory: Path, name: str, flags: Sequence[str] = FLAGS) -> Path:
    directory.mkdir(parents=True, exist_ok=True)
    (directory / "library.hpp").write_bytes(library)
    (directory / "benchmark.cpp").write_bytes(fixture)
    (directory / "benchmark_common.hpp").write_bytes(common)
    executable = directory / name
    command([*compiler, str(directory / "benchmark.cpp"), *flags, "-o", str(executable)])
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


def paired_samples(rows: list[dict]) -> dict[str, dict[str, list[int]]]:
    return {f"{row['size']}:{row['pattern_id']}": row["samples_ns"]
            for row in rows if row["status"] == "MEASURED"}


def code(value: str) -> str:
    return "<code>" + html.escape(" ".join(value.split())[:120]).replace("|", "&#124;").replace("`", "&#96;") + "</code>"


def markdown(report: dict) -> str:
    compare = report.get("comparison_requested", False)
    method = ("基準commitと現在版を、同じ計測コード・コンパイラ・コンパイル引数で交互に測ります。"
              if compare else "現在版の測定値を表示します。")
    lines = ["## 本体の性能", "",
             method,
             "時間は本体の構築・操作・破棄とchecksum集約です。"]
    if compare:
        lines.append("本体ごとに、基準・現在版それぞれのケース中央値の最大値を比較します。"
                     "上限は基準の最大値に5%（最低5 µs）を加えた値です。超過時は本体の全比較ケースを再測定します。")
    lines += ["", "状態: " + str(report["status"])]
    lines += ["", "コンパイル設定: " + code(report.get("compile_profile", "standard")),
              "フラグ: " + " ".join(map(code, report.get("flags", FLAGS)))]
    lines += ["", "対象はFenwick Tree・UnionFind・RollingHashの代表3本です。",
              "通常実行の既定対象は各14ケース（計42）。--id指定時は対象分だけ実行します。",
              "2,000,000要素はCIのstress上限で、問題のMax制約やAPIの上限を意味しません。",
              "--quickは小規模の動作確認だけで、大規模・最大ケースの実施とは扱いません。"]
    if report.get("scale_scope"):
        lines += ["", "今回の規模: " + code(report["scale_scope"]) +
                  "、予定ケース数: " + str(report["expected_cases"])]
    if report.get("base_revision"):
        lines += ["", "基準: " + code(report["base_revision"])]
    worst = report.get("worst_case_regressions", {})
    if worst:
        descriptions = {(row["id"], f"{row['size']}:{row['pattern_id']}"):
                        f"{row['pattern']} ({row['size']:,})" for row in report["results"]}
        lines += ["", "| 本体 | 比較件数 | 基準の最大ケース | 現在の最大ケース | 基準の最大値 ms | 現在の最大値 ms | 上限 ms | 退行判定 |",
                  "| --- | ---: | --- | --- | ---: | ---: | ---: | --- |"]
        for identifier, gate in worst.items():
            before = code(descriptions[identifier, gate["baseline_case"]])
            after = code(descriptions[identifier, gate["observed_case"]])
            lines.append(f"| {identifier} | {gate['compared_cases']} | {before} | {after} | "
                         f"{gate['baseline_median_ns']/1e6:.3f} | {gate['observed_median_ns']/1e6:.3f} | "
                         f"{gate['limit_ns']/1e6:.3f} | {gate['status']} |")
    lines += (["", "| 本体 | 入力 | 要素数 | 現在 ms | 基準 ms | 現在/基準 | ケース参考上限 ms | ケース参考判定 |",
               "| --- | --- | ---: | ---: | ---: | ---: | ---: | --- |"] if compare else
              ["", "| 本体 | 入力 | 要素数 | 実行時間 ms | 固定予算 ms | 判定 |",
               "| --- | --- | ---: | ---: | ---: | --- |"])
    for row in report.get("results", []):
        median = row["median_ns"]
        line = f"| {row['id']} | {row['pattern']} | {row['size']} | {median['current']/1e6:.3f} |"
        if compare:
            base = f"{median['base'] / 1e6:.3f}" if "base" in median else "N/A"
            ratio = f"{row['current_over_base']:.3f}" if row["current_over_base"] is not None else "N/A"
            line += f" {base} | {ratio} |"
        gate = row.get("performance_regression") or row.get("performance_limit")
        if gate:
            line += f" {gate['limit_ns']/1e6:.3f} | {gate['status']} |"
        else:
            line += " — | 未照合 |"
        lines.append(line)
    if any(row["status"] == "NO_BASELINE" for row in report.get("results", [])):
        lines += ["", "N/Aは基準commitに本体がないため比較未実施です。現在版の計測と独立照合だけを行いました。"]
    if report["status"] == "FAIL":
        lines += ["", "ビルド・照合・計測・性能判定のいずれかが失敗しました。詳細はJSONを確認してください。"]
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
    profile = getattr(args, "compile_profile", "standard")
    flags = ci_compile_profiles.compile_flags(profile, FLAGS)
    report = {"format": 1, "status": "running", "scope": "representative_normal_library_kernels",
              "comparison_requested": args.base is not None,
              "timing_scope": "library construction, operations, destruction and shared digest only",
              "online_status": "not_evaluated", "compile_profile": profile, "flags": flags,
              "repetitions": args.repetitions, "warmups": args.warmups,
              "platform": platform.platform(), "machine": platform.machine(), "cpu_model": cpu_model(),
              "affinity": sorted(os.sched_getaffinity(0)) if hasattr(os, "sched_getaffinity") else None,
              "results": [], "sources": {}}
    if args.base is not None:
        report.update(regression_policy=performance_regression.WORST_CASE_POLICY, worst_case_regressions={},
                      worst_case_confirmations={})
    write_report(output, report)
    try:
        revision = resolve_base(args.base) if args.base is not None else None
        report["base_revision"] = revision
        report["comparison_requested"] = revision is not None
        limits = performance_limits.load(args.limits) if getattr(args, "limits", None) else None
        if limits is not None:
            report["performance_limits"] = limits.metadata
            report["performance_limits_passed"] = False
        if revision is not None:
            report["performance_regression_passed"] = False
        compiler = compiler_command(args.compiler)
        report["compiler"] = {"command": compiler,
                              "version": command([compiler[0], "--version"], timeout=15).stdout.decode().splitlines()[0]}
        common = (FIXTURES / "benchmark_common.hpp").read_bytes()
        selected = args.id or list(LIBRARIES)
        plans = {identifier: case_plan(identifier, args.quick) for identifier in selected}
        report["case_plan"] = plans
        report["expected_cases"] = sum(map(len, plans.values()))
        report["scale_scope"] = "smoke_only" if args.quick else "small_large_ci_stress_and_power_of_two_boundaries"
        report["ci_stress_size_limit"] = STRESS_SIZE
        budgets = {}
        if limits is not None:
            for identifier, cases in plans.items():
                for case in cases:
                    rounds = 1 if args.quick else max(1, 262144 // case["size"])
                    budgets[identifier, case["size"], case["pattern_id"]] = limits.kernel(
                        LIBRARIES[identifier], identifier, case["scenario"], case["size"], case["pattern_id"], rounds)
        with tempfile.TemporaryDirectory(prefix="sirkit-benchmark-") as temporary:
            directory = Path(temporary)
            for identifier in selected:
                path = LIBRARIES[identifier]
                current = (ROOT / path).read_bytes()
                base = baseline_source(revision, path) if revision is not None else None
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
                                                directory / identifier, "base", flags)
                executables["current"] = build(compiler, current, fixture, common,
                                               directory / identifier, "current", flags)

                def measure_row(case: dict, order: dict[str, Path]) -> dict:
                    size, pattern = case["size"], case["pattern_id"]
                    rounds = 1 if args.quick else max(1, 262144 // size)
                    row = measure(order, size, pattern, rounds, args.repetitions, args.warmups)
                    if row["status"] == "MEASURED":
                        row["performance_regression"] = performance_regression.evaluate(
                            "kernel_elapsed_ns", row["samples_ns"])
                    if revision is None:
                        row["status"] = "CURRENT_ONLY"
                    row.update(id=identifier, pattern=case["scenario"], pattern_id=pattern, scale=case["scale"])
                    if limits is not None:
                        row["performance_limit"] = limits.evaluate(
                            budgets[identifier, size, pattern], row["samples_ns"]["current"])
                    return row

                first_index = len(report["results"])
                for case in plans[identifier]:
                    report["results"].append(measure_row(case, executables))
                    write_report(output, report)
                samples = paired_samples(report["results"][first_index:])
                if samples:
                    worst = performance_regression.evaluate_worst("kernel_elapsed_ns", samples)
                    if worst["status"] == "FAIL":
                        report["worst_case_confirmations"][identifier] = samples
                        reversed_order = dict(reversed(tuple(executables.items())))
                        confirmed = [measure_row(case, reversed_order) for case in plans[identifier]]
                        worst = performance_regression.evaluate_worst("kernel_elapsed_ns", paired_samples(confirmed))
                        report["results"][first_index:] = confirmed
                    report["worst_case_regressions"][identifier] = worst
                    write_report(output, report)
        expected = {(identifier, case["size"], case["pattern_id"])
                    for identifier, cases in plans.items() for case in cases}
        completed = [(row["id"], row["size"], row["pattern_id"]) for row in report["results"]]
        if len(completed) != len(expected) or set(completed) != expected:
            raise BenchmarkError("予定した性能ケースの省略または重複があります")
        compared = sum(row["status"] == "MEASURED" for row in report["results"])
        report["compared_cases"] = compared
        report["uncompared_cases"] = len(report["results"]) - compared
        report["current_measurements_passed"] = bool(report["results"])
        if limits is not None:
            limits.check_unchanged()
            report["performance_limits_passed"] = all(row["performance_limit"]["status"] == "PASS" for row in report["results"])
        report["status"] = ("CURRENT_ONLY" if revision is None else "NO_BASELINE" if not compared
                            else "PARTIAL" if report["uncompared_cases"] else "PASS")
        regressed = any(gate["status"] == "FAIL" for gate in report.get("worst_case_regressions", {}).values())
        if revision is not None:
            report["performance_regression_passed"] = compared == len(expected) and not regressed
        if regressed:
            report["status"] = "FAIL"
            report["error"] = "再測定でも基準からの性能退行を検出しました。"
        elif limits is not None and not report["performance_limits_passed"]:
            report["status"] = "FAIL"
            report["error"] = "固定性能予算を超過しました。正しさの照合結果と測定値はresultsへ保存しています。"
        write_report(output, report)
        print(report["status"] + ": " + str(output / "summary.md"))
        return 1 if report["status"] == "FAIL" else 2 if revision is not None and not compared else 0
    except (BenchmarkError, OSError, ValueError, KeyError) as error:
        report["status"] = "FAIL"
        report["error"] = str(error)
        write_report(output, report)
        print(str(error), file=sys.stderr)
        return 1


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--base", help="比較が必要な場合だけ指定する基準commit/ref")
    parser.add_argument("--compiler", default="g++")
    parser.add_argument("--compile-profile", choices=ci_compile_profiles.COMPILE_PROFILES, default="standard",
                        help="基準と現在版の両方に使うコンパイル設定")
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--limits", type=Path, help="明示した場合だけ従来の固定性能予算も検査する")
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
