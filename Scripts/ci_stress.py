#!/usr/bin/env python3
# SPDX-License-Identifier: CC0-1.0
"""専用jobでMax・構造ケースを逐次実行する。全出力を独立参照と照合する。"""
from __future__ import annotations

import argparse
import hashlib
import itertools
import json
import os
from pathlib import Path
import re
import resource
import shutil
import signal
import subprocess
import sys
import tempfile
import time

try:
    from . import prepare_verify
except ImportError:
    import prepare_verify

ROOT = Path(__file__).resolve().parents[1]
if str(ROOT) not in sys.path:
    sys.path.insert(0, str(ROOT))
from Verify.ci import stress_cases

FLAGS = ["-std=c++23", "-O2", "-DNDEBUG"]
INTEGER = re.compile(rb"[+-]?[0-9]{1,32}\Z")


class StressError(ValueError):
    pass


def digest(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def frozen_sources(entries: list[dict], root: Path = ROOT) -> dict[str, str]:
    names = {"Scripts/ci_stress.py", "Scripts/prepare_verify.py",
             "Verify/ci/stress_cases.py", "Verify/ci/library_checker.json"}
    for entry in entries:
        names.update(entry["libraries"])
        names.add(entry["driver"])
        if entry.get("id"):
            names.add("Verify/current/submissions/" + entry["id"] + ".cpp")
        if entry.get("group"):
            names.add("Verify/current/groups/" + entry["group"] + ".json")
        names.update(prepare_verify.common_source_hashes((root / entry["driver"]).read_text()))
    result = {}
    for name in sorted(names):
        path = (root / name).resolve(strict=True)
        if not path.is_relative_to(root.resolve()) or not path.is_file():
            raise StressError("source outside repository")
        result[name] = digest(path)
    return result


def assert_sources_unchanged(frozen: dict[str, str], root: Path = ROOT) -> None:
    for name, expected in frozen.items():
        path = (root / name).resolve(strict=True)
        if not path.is_relative_to(root.resolve()) or not path.is_file() or digest(path) != expected:
            raise StressError("source changed during stress verification: " + name)


def selected_entries(plans: list[dict]) -> list[dict]:
    wanted = {plan["verify_id"] for plan in plans}
    entries = [entry for entry in prepare_verify.collect() if entry["id"] in wanted]
    if len(entries) != len(wanted) or {entry["id"] for entry in entries} != wanted:
        raise StressError("missing or duplicate Verify entry")
    return entries


def validate_constraints() -> None:
    pin = json.loads((ROOT / "Verify/ci/library_checker.json").read_text())
    if pin["revision"] != stress_cases.LIBRARY_CHECKER_REVISION:
        raise StressError("official revision changed; review Max constraints and citations")


def compare_output(actual: Path, expected: Path, expected_count: int) -> dict:
    """全整数を逐次照合し、欠落・余分・空の正解も失敗とする。"""
    def tokens(path: Path):
        with path.open("rb") as stream:
            for line in stream:
                for token in line.split():
                    if not INTEGER.fullmatch(token):
                        raise StressError("noninteger or oversized output token")
                    yield int(token)
    if expected_count <= 0 or not actual.is_file() or not expected.is_file():
        return {"passed": False, "answer_count": 0, "error": "missing output or empty oracle"}
    if actual.stat().st_size > stress_cases.MAX_OUTPUT_BYTES:
        return {"passed": False, "answer_count": 0, "error": "output size cap exceeded"}
    missing = object()
    count = 0
    try:
        for wanted, got in itertools.zip_longest(tokens(expected), tokens(actual), fillvalue=missing):
            if wanted is missing or got is missing or wanted != got:
                return {"passed": False, "answer_count": count, "error": "output mismatch or missing/extra answer"}
            count += 1
    except (OSError, StressError) as error:
        return {"passed": False, "answer_count": count, "error": str(error)}
    return {"passed": count == expected_count, "answer_count": count,
            **({} if count == expected_count else {"error": "oracle count mismatch"})}


def child_limits(memory_mib: int):
    def apply():
        memory = memory_mib * 1024 * 1024
        resource.setrlimit(resource.RLIMIT_AS, (memory, memory))
        resource.setrlimit(resource.RLIMIT_FSIZE, (stress_cases.MAX_INPUT_BYTES, stress_cases.MAX_INPUT_BYTES))
        resource.setrlimit(resource.RLIMIT_CORE, (0, 0))
    return apply


def execute(command: list[str], *, stdout: Path, stderr: Path, timeout: float,
            memory_mib: int, stdin: Path | None = None) -> dict:
    started = time.perf_counter_ns()
    with (stdin.open("rb") if stdin is not None else open(os.devnull, "rb")) as inp, stdout.open("wb") as out, stderr.open("wb") as err:
        process = subprocess.Popen(command, stdin=inp, stdout=out, stderr=err,
                                   start_new_session=True, preexec_fn=child_limits(memory_mib))
        timed_out = False
        try:
            returncode = process.wait(timeout=timeout)
        except subprocess.TimeoutExpired:
            timed_out = True
            try:
                os.killpg(process.pid, signal.SIGKILL)
            except ProcessLookupError:
                pass
            returncode = process.wait()
    return {"returncode": returncode, "timed_out": timed_out,
            "elapsed_ns": time.perf_counter_ns() - started}


def complete(report: dict) -> bool:
    expected = report.get("selected_case_ids", [])
    rows = report.get("results", [])
    builds = report.get("builds", [])
    if (not isinstance(expected, list) or not expected
            or any(not isinstance(name, str) for name in expected)
            or len(expected) != len(set(expected)) or not isinstance(rows, list)
            or any(not isinstance(row, dict) for row in rows)
            or [row.get("name") for row in rows] != expected
            or report.get("source_freeze_passed") is not True
            or not isinstance(builds, list) or any(not isinstance(row, dict) for row in builds)):
        return False
    wanted_builds = {row.get("verify_id") for row in rows}
    if (None in wanted_builds or len(builds) != len(wanted_builds)
            or {row.get("verify_id") for row in builds} != wanted_builds
            or any(row.get("status") != "PASS" for row in builds)):
        return False
    for row in rows:
        expected_count, actual_count = row.get("expected_answer_count"), row.get("answer_count")
        operations, dimensions = row.get("generated_operations"), row.get("dimensions")
        if (row.get("status") != "PASS" or type(expected_count) is not int or expected_count <= 0
                or type(actual_count) is not int or actual_count != expected_count
                or type(operations) is not int or not isinstance(dimensions, dict)
                or type(dimensions.get("q")) is not int or operations != dimensions["q"]):
            return False
    return True


def markdown(report: dict) -> str:
    lines = ["## Max・構造ケースの専用Verify", "",
             "状態: " + report["status"], "",
             "quickは縮小した生成器検査です。問題Maxの実行とは表示しません。",
             "固定seed・閉形式の独立参照で全出力を照合します。オンラインACとは別のCI実行です。",
             "時間は起動・入出力を含み、生成・正解計算・コンパイルは含みません。本体性能比較ではありません。", "",
             "| ケース | N | Q | 上限制約と一致 | 結果 | ms |",
             "| --- | ---: | ---: | --- | --- | ---: |"]
    for row in report.get("results", []):
        dimensions = row["dimensions"]
        elapsed = f"{row['elapsed_ns']/1e6:.3f}" if row.get("elapsed_ns") is not None else "未実施"
        maximum = ", ".join(row["maximum_dimensions"]) or "構造検査／縮小版"
        lines.append(f"| {row['name']} | {dimensions['n']} | {dimensions['q']} | {maximum} | {row['status']} | {elapsed} |")
    if report.get("error"):
        lines += ["", "エラー: " + str(report["error"]).replace("\n", " ")[:800]]
    lines += ["", "対象はUnionFindとYuki2295用PersistentUnionFindです。他の全ライブラリのMax網羅は主張しません。"]
    return "\n".join(lines) + "\n"


def write_report(output: Path, report: dict) -> None:
    temporary = output / "result.json.tmp"
    temporary.write_text(json.dumps(report, ensure_ascii=False, indent=2) + "\n")
    temporary.replace(output / "result.json")
    (output / "summary.md").write_text(markdown(report))


def run(args: argparse.Namespace) -> int:
    output = args.output.resolve()
    if output.is_relative_to(ROOT.resolve()):
        raise StressError("--output must be outside the repository")
    output.mkdir(parents=True, exist_ok=True)
    plans = stress_cases.case_plans(args.quick)
    if args.case:
        plans = [plan for plan in plans if plan["name"] in args.case]
    report = {"type": "sirkit.stress", "format": 1, "status": "running",
              "scope": "dedicated_max_and_structural_verify", "online_status": "not_evaluated",
              "size_profile": "quick_smoke" if args.quick else "full",
              "selected_case_ids": [plan["name"] for plan in plans],
              "results": [{**plan, "status": "NOT_RUN"} for plan in plans], "builds": [],
              "source_hashes": {}, "source_freeze_passed": False,
              "flags": FLAGS, "memory_limit_mib": args.memory_mib,
              "case_timeout_seconds": args.case_timeout,
              "timing_scope": "whole_program_wall_time_including_startup_and_io; generator/oracle/compiler excluded"}
    frozen = None
    write_report(output, report)
    try:
        if not plans:
            raise StressError("no selected cases")
        validate_constraints()
        compiler = shutil.which(args.compiler)
        if compiler is None:
            raise StressError("compiler not found")
        entries = selected_entries(plans)
        frozen = frozen_sources(entries)
        report["source_hashes"] = frozen
        by_id = {entry["id"]: entry for entry in entries}
        with tempfile.TemporaryDirectory(prefix="sirkit-ci-stress-") as temporary:
            work = Path(temporary)
            version_file, version_error = work / "compiler-version.txt", work / "compiler-version.stderr"
            version_result = execute([compiler, "--version"], stdout=version_file, stderr=version_error,
                                     timeout=15, memory_mib=1536)
            if version_result["returncode"] != 0 or version_result["timed_out"]:
                raise StressError("cannot read compiler version")
            version_lines = version_file.read_text(errors="replace").splitlines()
            if not version_lines:
                raise StressError("empty compiler version")
            report["compiler"] = {"path": compiler, "version": version_lines[0],
                                  "sha256": digest(Path(compiler).resolve())}
            binaries, hashes = {}, {}
            for identifier in sorted(by_id):
                entry = by_id[identifier]
                source, library_hashes = prepare_verify.render(entry, skip_runtime_checks=True)
                canonical = ROOT / "Verify/current/submissions" / (identifier + ".cpp")
                if canonical.read_bytes() != source:
                    raise StressError("submission out of sync: " + identifier)
                snapshot = work / (identifier + ".cpp")
                snapshot.write_bytes(source)
                binary = work / identifier
                command = [compiler, *FLAGS, str(snapshot), "-o", str(binary)]
                result = execute(command, stdout=work / (identifier + ".compile.stdout"),
                                 stderr=work / (identifier + ".compile.stderr"),
                                 timeout=args.compile_timeout, memory_mib=1536)
                diagnostic = (work / (identifier + ".compile.stderr")).read_bytes()[-16000:].decode("utf-8", "replace")
                row = {"verify_id": identifier, "source_sha256": hashlib.sha256(source).hexdigest(),
                       "source_hashes": library_hashes, "driver_sha256": frozen[entry["driver"]],
                       "command": command, "diagnostic": diagnostic, **result,
                       "status": "PASS" if result["returncode"] == 0 and not result["timed_out"] else "FAIL"}
                report["builds"].append(row)
                write_report(output, report)
                if row["status"] != "PASS" or not binary.is_file():
                    raise StressError("submission compile failed: " + identifier)
                binaries[identifier], hashes[identifier] = binary, row["source_sha256"]
            for plan, row in zip(plans, report["results"]):
                directory = work / plan["name"]
                generated = stress_cases.generate_case(plan, directory, generation_timeout=args.generation_timeout)
                row.update(generated, submission_sha256=hashes[plan["verify_id"]],
                           libraries=by_id[plan["verify_id"]]["libraries"],
                           source_hashes={name: frozen[name] for name in by_id[plan["verify_id"]]["libraries"]})
                actual = directory / "actual.txt"
                result = execute([str(binaries[plan["verify_id"]])], stdin=directory / "input.txt",
                                 stdout=actual, stderr=directory / "stderr.txt", timeout=args.case_timeout,
                                 memory_mib=args.memory_mib)
                row.update(result)
                diagnostic = (directory / "stderr.txt").read_bytes()[-16000:].decode("utf-8", "replace")
                if diagnostic:
                    row["diagnostic"] = diagnostic
                if result["returncode"] == 0 and not result["timed_out"]:
                    row.update(compare_output(actual, directory / "expected.txt", generated["expected_answer_count"]))
                else:
                    row.update(passed=False, answer_count=0, error="runtime failure or timeout")
                row["actual_sha256"] = digest(actual)
                row["status"] = "PASS" if row["passed"] else "FAIL"
                write_report(output, report)
                # 1ケースずつ生成・実行・破棄し、全Max入力を同時保持しない。
                shutil.rmtree(directory)
                if row["status"] != "PASS":
                    raise StressError("stress output verification failed: " + plan["name"])
        assert_sources_unchanged(frozen)
        report["source_freeze_passed"] = True
        report["status"] = "PASS" if complete(report) else "FAIL"
    except (OSError, ValueError, subprocess.SubprocessError) as error:
        report["status"], report["error"] = "FAIL", str(error)
    finally:
        if frozen is not None:
            try:
                assert_sources_unchanged(frozen)
            except (OSError, StressError) as error:
                report["source_freeze_passed"] = False
                report["status"], report["error"] = "FAIL", str(error)
        write_report(output, report)
    return 0 if report["status"] == "PASS" else 1


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", type=Path)
    parser.add_argument("--plan", action="store_true", help="入力生成・コンパイルなしで計画JSONだけ表示")
    parser.add_argument("--quick", action="store_true", help="縮小入力。問題Maxの実行とはみなさない")
    parser.add_argument("--case", action="append", choices=[p["name"] for p in stress_cases.case_plans()])
    parser.add_argument("--compiler", default="g++")
    parser.add_argument("--case-timeout", type=float, default=60)
    parser.add_argument("--compile-timeout", type=float, default=300)
    parser.add_argument("--generation-timeout", type=float, default=60)
    parser.add_argument("--memory-mib", type=int, default=512)
    args = parser.parse_args(argv)
    if not all(0 < value <= 600 for value in (args.case_timeout, args.compile_timeout, args.generation_timeout)):
        parser.error("timeouts must be in (0,600] seconds")
    if not 64 <= args.memory_mib <= 2048:
        parser.error("memory-mib must be in [64,2048]")
    if args.plan:
        validate_constraints()
        plans = stress_cases.case_plans(args.quick)
        if args.case:
            plans = [plan for plan in plans if plan["name"] in args.case]
        print(json.dumps({"status": "PLAN_ONLY", "executed": False, "cases": plans}, ensure_ascii=False, indent=2))
        return 0
    if args.output is None:
        parser.error("--output is required unless --plan is used")
    try:
        return run(args)
    except (OSError, StressError) as error:
        print(str(error), file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
