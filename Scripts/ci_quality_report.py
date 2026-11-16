#!/usr/bin/env python3
# SPDX-License-Identifier: CC0-1.0
"""CIの実行記録を原本SHAへ結び付け、ページ用の品質JSONと表を作る。"""
from __future__ import annotations

import argparse
from contextlib import contextmanager
from datetime import datetime, timezone
import hashlib
import html
import json
import math
import os
from pathlib import Path, PurePosixPath
import re
import statistics
import subprocess
import sys

sys.dont_write_bytecode = True
try:
    from . import prepare_verify
except ImportError:
    import prepare_verify

ROOT = Path(__file__).resolve().parents[1]
LIBRARY_DIRS = {"Algorithm", "DataStructure", "Geometry", "Graph", "Internal", "Math", "String"}
CATEGORIES = ("normal", "failure", "library_checker", "performance", "sanitizers", "coverage")
LABELS = dict(normal="正常系", failure="失敗系", library_checker="Library Checker公式ケース", performance="本体の性能測定", sanitizers="ASan/UBSan", coverage="カバレッジ")
STATUS_LABELS = dict(PASS="通過", FAIL="失敗", STALE="ソース変更", UNAVAILABLE="未測定", PARTIAL="一部のみ")
SHA = re.compile(r"[0-9a-f]{64}\Z")
IDENTIFIER = re.compile(r"[A-Za-z0-9_-]+\Z")


class ReportError(ValueError):
    pass


def decode_json(data: bytes | str):
    def pairs(items):
        output = {}
        for key, value in items:
            if key in output:
                raise ReportError("JSONキーが重複しています")
            output[key] = value
        return output
    def constant(_):
        raise ReportError("JSONに非有限の数値があります")
    return json.loads(data, object_pairs_hook=pairs, parse_constant=constant)


def read_json(path: Path):
    return decode_json(path.read_bytes())


def digest(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def sha(value: object) -> str:
    if not isinstance(value, str) or not SHA.fullmatch(value):
        raise ReportError("SHA-256の形式が不正です")
    return value


def relative(value: object, *, library: bool = False) -> str:
    if not isinstance(value, str) or "\\" in value or any(ord(c) < 32 for c in value):
        raise ReportError("ソースの相対パスが不正です")
    path = PurePosixPath(value)
    if path.is_absolute() or not path.parts or ".." in path.parts or path.as_posix() != value:
        raise ReportError("ソースの相対パスが不正です")
    if library and (path.parts[0] not in LIBRARY_DIRS or path.suffix != ".cpp"):
        raise ReportError("通常版本体以外をlibraryに指定できません")
    return value


def file_path(root: Path, name: str) -> Path:
    path = (root / relative(name)).resolve()
    if not path.is_relative_to(root.resolve()):
        raise ReportError("ソースがリポジトリ外を参照しています")
    return path


def hashes(value: object, *, nonempty: bool = True) -> dict[str, str]:
    if not isinstance(value, dict) or (nonempty and not value):
        raise ReportError("ソースSHAの対応表がありません")
    return {relative(name): sha(checksum) for name, checksum in value.items()}


def mismatches(root: Path, recorded: dict[str, str]) -> list[str]:
    result = []
    for name, checksum in recorded.items():
        path = file_path(root, name)
        if not path.is_file() or digest(path.read_bytes()) != checksum:
            result.append(name)
    return sorted(result)


def integer(value: object, *, minimum: int = 0) -> int:
    if type(value) is not int or value < minimum:
        raise ReportError("件数・時刻の値が不正です")
    return value


def text_list(value: object) -> list[str]:
    if not isinstance(value, list) or not value or not all(isinstance(x, str) and x for x in value):
        raise ReportError("実行コマンド・設定の記録が不正です")
    return value


def checked_flags(command: list[str], *, explicit: bool = True) -> None:
    if explicit and ("-UNDEBUG" not in command or "-USKIP_RUNTIME_CHECKS" not in command):
        raise ReportError("checked指定を確認できません")
    for i, value in enumerate(command):
        macro = value[2:] if value.startswith("-D") else ""
        if value == "-D" and i + 1 < len(command):
            macro = command[i + 1]
        if macro.split("=", 1)[0] in {"NDEBUG", "SKIP_RUNTIME_CHECKS"}:
            raise ReportError("checked実行に検査無効化が混在しています")


def provenance(root: Path) -> dict:
    result = {"commit": None, "dirty": None}
    try:
        result["commit"] = subprocess.run(["git", "rev-parse", "HEAD"], cwd=root,
                                           capture_output=True, text=True, check=True).stdout.strip()
        result["dirty"] = bool(subprocess.run(["git", "status", "--porcelain"], cwd=root,
                                             capture_output=True, text=True, check=True).stdout)
    except (OSError, subprocess.SubprocessError):
        pass
    for key in ("GITHUB_SHA", "GITHUB_RUN_ID", "GITHUB_RUN_ATTEMPT", "GITHUB_REPOSITORY", "GITHUB_JOB"):
        if os.environ.get(key):
            result[key.lower()] = os.environ[key]
    return result


@contextmanager
def verify_root(root: Path):
    old_root, old_base = prepare_verify.ROOT, prepare_verify.BASE
    prepare_verify.ROOT, prepare_verify.BASE = root, root / "Verify/current"
    try:
        yield
    finally:
        prepare_verify.ROOT, prepare_verify.BASE = old_root, old_base


def manifest_entries(root: Path) -> dict[str, dict]:
    value = read_json(root / "Verify/current/manifest.json")
    if type(value.get("format")) is not int or value["format"] != 1 or not isinstance(value.get("entries"), list):
        raise ReportError("Verify manifestの形式が不正です")
    entries = {}
    for entry in value["entries"]:
        identifier = entry.get("id") if isinstance(entry, dict) else None
        if not isinstance(identifier, str) or not IDENTIFIER.fullmatch(identifier) or identifier in entries:
            raise ReportError("Verify IDの形式・重複が不正です")
        libraries = entry.get("libraries")
        if not isinstance(libraries, list) or not libraries or len(set(libraries)) != len(libraries):
            raise ReportError("Verifyの対象本体が不正です")
        for library in libraries:
            relative(library, library=True)
        entries[identifier] = entry
    return entries


def entry_snapshot(entry: dict) -> dict[str, str]:
    result = hashes(entry.get("source_hashes"))
    if not set(entry["libraries"]) <= result.keys():
        raise ReportError("Verify manifestの本体SHAが不足しています")
    result[relative(entry.get("driver"))] = sha(entry.get("driver_sha256"))
    result.update(hashes(entry.get("support_source_hashes", {}), nonempty=False))
    result[relative(entry.get("submission"))] = sha(entry.get("submission_sha256"))
    return result


def discover(inputs: list[Path]) -> list[Path]:
    files = set()
    for path in inputs:
        if path.is_dir():
            files.update(p.resolve() for p in path.rglob("*.json")
                         if p.name in {"result.json", "results.json", "contracts.json"} and p.is_file())
        elif path.is_file():
            files.add(path.resolve())
        else:
            raise ReportError("入力レポートが存在しません: " + str(path))
    return sorted(files)


def combine(evidence: list[dict]) -> str:
    if not evidence:
        return "UNAVAILABLE"
    values = {item["status"] for item in evidence}
    if "FAIL" in values:
        return "FAIL"
    if values == {"STALE"}:
        return "STALE"
    if values == {"UNAVAILABLE"}:
        return "UNAVAILABLE"
    return "PASS" if values == {"PASS"} else "PARTIAL"


def collect_normal(value: dict, root: Path, entries: dict, append) -> None:
    compiler = value.get("compiler")
    if not isinstance(compiler, str) or not compiler or type(value.get("standard_fallback")) is not bool:
        raise ReportError("入力例照合のコンパイラ・設定が不正です")
    identifiers = value.get("plan", {}).get("selected_ids")
    rows = value.get("results")
    if not isinstance(identifiers, list) or not identifiers or len(set(identifiers)) != len(identifiers):
        raise ReportError("入力例照合の対象が空または重複しています")
    if not all(isinstance(x, str) and x in entries for x in identifiers) or not isinstance(rows, list):
        raise ReportError("入力例照合に未知のIDがあります")
    by_id = {}
    for row in rows:
        if not isinstance(row, dict) or row.get("id") not in identifiers or type(row.get("skip_runtime_checks")) is not bool:
            raise ReportError("入力例照合の行が不正です")
        key = (row["id"], row["skip_runtime_checks"])
        if key in by_id or row.get("compiler") != compiler or row.get("standard_fallback") is not value["standard_fallback"]:
            raise ReportError("入力例照合の重複・設定不一致があります")
        by_id[key] = row
    if value.get("status") not in {"PASS", "FAIL", "running"}:
        raise ReportError("入力例照合の状態が不正です")
    for identifier in identifiers:
        entry = entries[identifier]
        changed = mismatches(root, entry_snapshot(entry))
        selected = [by_id.get((identifier, skip)) for skip in (False, True)]
        valid_rows = []
        for skip, row in zip((False, True), selected):
            if row is None:
                continue
            sha(row.get("submission_sha256")); sha(row.get("tested_source_sha256"))
            if row["submission_sha256"] != entry["submission_sha256"]:
                changed.append(entry["submission"])
            command = text_list(row.get("command"))
            if not skip:
                checked_flags(command, explicit=False)
            if not changed:
                with verify_root(root):
                    source, _ = prepare_verify.render(entry, skip_runtime_checks=skip)
                if digest(source) != row["tested_source_sha256"]:
                    changed.append(entry["submission"])
            if type(row.get("compile_ok")) is not bool or type(row.get("passed")) is not bool or not isinstance(row.get("samples"), list):
                raise ReportError("入力例照合のPASS形式が不正です")
            ok = row["compile_ok"] and bool(row["samples"])
            for sample in row["samples"]:
                if not isinstance(sample, dict) or type(sample.get("passed")) is not bool:
                    raise ReportError("入力例照合のsampleが不正です")
                if "returncode" in sample and type(sample["returncode"]) is not int:
                    raise ReportError("入力例照合のreturncodeが不正です")
                ok &= sample["passed"] and sample.get("returncode", 0) == 0
            if row["passed"] != ok:
                raise ReportError("入力例照合の集約PASSと実行結果が不一致です")
            valid_rows.append(ok)
        status = "STALE" if changed else "FAIL" if False in valid_rows else "PASS" if len(valid_rows) == 2 else "UNAVAILABLE"
        if value["status"] == "PASS" and len(valid_rows) != 2:
            raise ReportError("PASS入力例照合にchecked/skipの結果が不足しています")
        for library in entry["libraries"]:
            append(library, "normal", dict(id=identifier, status=status, scope="入力例（checked/skip）",
                   checks=sum(len(row["samples"]) for row in selected if row), compiler=compiler,
                   standard_fallback=value["standard_fallback"], source_mismatches=sorted(set(changed)),
                   tested_source_sha256=[row["tested_source_sha256"] for row in selected if row]))


def collect_contracts(value: dict, root: Path, entries: dict, append) -> None:
    if value.get("checked") is not True:
        raise ReportError("失敗系はchecked実行だけを採用します")
    command = text_list(value.get("command")); checked_flags(command)
    recorded = hashes(value.get("source_hashes"))
    after = hashes(value.get("source_hashes_after"))
    if recorded != after:
        raise ReportError("契約検査中にソースが変更されています")
    changed = mismatches(root, recorded)
    rows = value.get("results")
    if not isinstance(rows, list):
        raise ReportError("契約検査の結果が不正です")
    if not rows:
        if value.get("status") != "FAIL":
            raise ReportError("契約検査が空です")
        # ビルド・異常終了は、期待する例外を捕捉した成功とは別の失敗として表示する。
        for library in recorded:
            if PurePosixPath(library).parts[0] in LIBRARY_DIRS:
                relative(library, library=True)
                for category in ("normal", "failure"):
                    append(library, category, dict(status="STALE" if changed else "FAIL", checks=0,
                           scope="契約検査のビルド・実行失敗", command=command,
                           source_mismatches=changed))
        return
    seen = set()
    for row in rows:
        if not isinstance(row, dict):
            raise ReportError("契約検査の行が不正です")
        library = relative(row.get("library"), library=True)
        if library in seen or library not in recorded:
            raise ReportError("契約検査の本体SHA欠落・重複があります")
        seen.add(library)
        success = integer(row.get("normal_checks"), minimum=1)
        failures = integer(row.get("expected_failures"), minimum=1)
        cases = row.get("cases")
        allowed_exceptions = {"std::out_of_range", "std::invalid_argument", "std::length_error",
                              "std::logic_error", "std::domain_error", "std::overflow_error",
                              "std::underflow_error", "std::bad_alloc"}
        if not isinstance(cases, list) or len(cases) != failures:
            raise ReportError("期待する例外のケースが不足しています")
        names = set()
        for case in cases:
            if (not isinstance(case, dict) or not isinstance(case.get("name"), str) or not IDENTIFIER.fullmatch(case["name"])
                    or case["name"] in names or case.get("expected_exception") not in allowed_exceptions
                    or case.get("status") != "PASS" or case.get("post_failure_state_checked") is not True):
                raise ReportError("期待する例外の型・失敗後の状態を確認できません")
            names.add(case["name"])
        if row.get("status") not in {"PASS", "FAIL"}:
            raise ReportError("契約検査の状態が不正です")
        ok = (value.get("status") == row["status"] == "PASS" and
              value.get("compile_returncode") == 0 and type(value.get("compile_returncode")) is int and
              value.get("returncode") == 0 and type(value.get("returncode")) is int)
        status = "STALE" if changed else "PASS" if ok else "FAIL"
        common = dict(status=status, scope="checked API契約（期待する例外・失敗後の状態）",
                      command=command, compiler=value.get("compiler"), source_mismatches=changed,
                      source_hashes=recorded, driver=value.get("driver"))
        append(library, "normal", dict(common, checks=success))
        append(library, "failure", dict(common, checks=failures, cases=row.get("cases", [])))
    if {name for name in recorded if PurePosixPath(name).parts[0] in LIBRARY_DIRS} != seen:
        raise ReportError("契約検査の対象本体と結果が一致しません")


def collect_instrumented(value: dict, root: Path, entries: dict, append) -> None:
    mode = value["mode"]
    if value.get("checked") is not True or value.get("status") not in {"PASS", "FAIL", "running"}:
        raise ReportError("計測はchecked実行が必要です")
    recorded = hashes(value.get("all_source_hashes")); library_hashes = hashes(value.get("library_source_hashes"))
    if any(recorded.get(name) != checksum for name, checksum in library_hashes.items()):
        raise ReportError("計測結果の本体SHAと全ソースSHAに矛盾があります")
    for name in library_hashes:
        relative(name, library=True)
    if recorded != hashes(value.get("source_hashes_after")):
        raise ReportError("計測中にソースが変更されています")
    changed = mismatches(root, recorded)
    vendor = hashes(value.get("vendor_source_hashes", {}), nonempty=False)
    changed += mismatches(root, vendor)
    rows = value.get("results"); identifiers = value.get("selected_ids")
    if not isinstance(rows, list) or not rows or not isinstance(identifiers, list) or len(set(identifiers)) != len(identifiers):
        raise ReportError("計測対象が空・重複しています")
    if any(not isinstance(x, str) or x not in entries for x in identifiers):
        raise ReportError("計測に未知のIDがあります")
    seen = set()
    for row in rows:
        if not isinstance(row, dict) or row.get("id") not in identifiers or row["id"] in seen:
            raise ReportError("計測結果のIDが不正です")
        seen.add(row["id"]); entry = entries[row["id"]]
        command = text_list(row.get("command")); checked_flags(command)
        required_flags = {"-fsanitize=address,undefined"} if mode == "sanitizers" else {"-fprofile-instr-generate", "-fcoverage-mapping"}
        if not required_flags <= set(command):
            raise ReportError("計測modeのコンパイラ設定がありません")
        required = set(entry["libraries"]) | {entry["driver"]} | set(entry.get("support_source_hashes", {}))
        if not required <= recorded.keys() or row.get("driver_sha256") != recorded.get(entry["driver"]):
            raise ReportError("計測結果の原本・driver SHAが不足しています")
        if row.get("status") not in {"PASS", "FAIL"} or not isinstance(row.get("samples"), list):
            raise ReportError("計測結果の形式が不正です")
        ok = row["status"] == "PASS" and type(row.get("compile_returncode")) is int and row["compile_returncode"] == 0 and bool(row["samples"])
        for sample in row["samples"]:
            if not isinstance(sample, dict) or type(sample.get("passed")) is not bool or type(sample.get("returncode")) is not int:
                raise ReportError("計測結果のsampleが不正です")
            ok &= sample["passed"] and sample["returncode"] == 0
        if row["status"] == "PASS" and not ok:
            raise ReportError("計測結果の集約PASSと実行結果が一致しません")
        status = "STALE" if changed else "PASS" if ok else "FAIL"
        for library in entry["libraries"]:
            metrics = coverage_metrics(value, library, root) if mode == "coverage" and ok and not changed else None
            actual_status = "UNAVAILABLE" if mode == "coverage" and ok and metrics is None and not changed else status
            append(library, mode, dict(id=row["id"], status=actual_status, checks=len(row["samples"]),
                   scope="checked入力例とローカル補足（coverageは原本ごとの到達和集合）", command=command,
                   compiler=value.get("tools", {}).get("compiler"), source_mismatches=sorted(set(changed)),
                   metrics=metrics))
    if value["status"] == "PASS" and seen != set(identifiers):
        raise ReportError("PASS計測結果の対象が不足しています")


def coverage_metrics(value: dict, library: str, root: Path) -> dict | None:
    directory = Path(value["_quality_report_directory"])
    path = file_path(directory, "coverage.json")
    if not path.is_file():
        return None
    coverage = read_json(path)
    if not isinstance(coverage, dict) or coverage.get("type") != "sirkit.source-coverage-union" or not isinstance(coverage.get("files"), list):
        raise ReportError("原本coverageの形式が不正です")
    matched = [row for row in coverage["files"] if isinstance(row, dict) and isinstance(row.get("filename"), str)
               and (row["filename"] == library or row["filename"].endswith("/" + library))]
    if len(matched) > 1:
        raise ReportError("原本coverageの対象が重複しています")
    if not matched:
        return None
    row = matched[0]
    lines, branches = row.get("lines"), row.get("branches")
    if not isinstance(lines, list) or not isinstance(branches, list) or not lines:
        return None
    seen_lines, seen_branches = set(), set()
    source_lines = len(file_path(root, library).read_text().splitlines())
    for item in lines:
        if not isinstance(item, list) or len(item) != 2 or type(item[1]) is not bool:
            raise ReportError("coverage行の形式が不正です")
        number = integer(item[0], minimum=1)
        if number > source_lines:
            raise ReportError("coverage行が原本の範囲外です")
        if number in seen_lines:
            raise ReportError("coverage行が重複しています")
        seen_lines.add(number)
    for item in branches:
        if (not isinstance(item, list) or len(item) != 6 or
                any(type(x) is not bool for x in item[4:])):
            raise ReportError("coverage分岐の形式が不正です")
        key = tuple(integer(x, minimum=1) for x in item[:4])
        if key[0] > source_lines or key[2] > source_lines:
            raise ReportError("coverage分岐が原本の範囲外です")
        if key in seen_branches:
            raise ReportError("coverage分岐が重複しています")
        seen_branches.add(key)
    metrics = {"lines": {"count": len(lines), "covered": sum(item[1] for item in lines)},
               "branches": {"count": 2 * len(branches), "covered": sum(sum(item[4:]) for item in branches)}}
    for key, metric in metrics.items():
        metric["percent"] = 100 * metric["covered"] / metric["count"] if metric["count"] else 0
        actual = row.get("summary", {}).get(key)
        if not isinstance(actual, dict) or any(actual.get(name) != metric[name] for name in ("count", "covered")):
            raise ReportError("原本coverageの集約値が一致しません")
    return metrics


def collect_performance(value: dict, root: Path, entries: dict, append) -> None:
    sources = value.get("sources"); rows = value.get("results")
    if not isinstance(sources, dict) or not sources or not isinstance(rows, list) or not rows:
        raise ReportError("性能測定が空です")
    repetitions = integer(value.get("repetitions"), minimum=1)
    if "warmups" in value:
        integer(value["warmups"])
    compiler = value.get("compiler")
    if not isinstance(compiler, dict) or not isinstance(compiler.get("version"), str) or not compiler["version"]:
        raise ReportError("性能測定のコンパイラ版がありません")
    text_list(compiler.get("command")); text_list(value.get("flags"))
    revision = value.get("base_revision")
    if not isinstance(revision, str) or not re.fullmatch(r"(?:[0-9a-f]{40}|[0-9a-f]{64})", revision):
        raise ReportError("性能測定の比較元commitがありません")
    if value.get("status") not in {"PASS", "PARTIAL", "NO_BASELINE", "FAIL"}:
        raise ReportError("性能測定の状態が不正です")
    seen, compared = set(), 0
    for row in rows:
        if not isinstance(row, dict) or row.get("id") not in sources:
            raise ReportError("性能測定に未知の対象があります")
        identifier = row["id"]
        if not IDENTIFIER.fullmatch(identifier):
            raise ReportError("性能測定IDが不正です")
        source = sources[identifier]; library = relative(source.get("path"), library=True)
        recorded = {library: sha(source.get("current_sha256")),
                    f"Verify/ci/benchmarks/{identifier}.cpp": sha(source.get("harness_sha256")),
                    "Verify/ci/benchmarks/benchmark_common.hpp": sha(source.get("common_harness_sha256"))}
        changed = mismatches(root, recorded)
        samples = row.get("samples_ns"); medians = row.get("median_ns")
        if not isinstance(samples, dict) or not isinstance(medians, dict) or set(samples) not in ({"current"}, {"base", "current"}) or set(medians) != set(samples):
            raise ReportError("性能測定の生データが不足しています")
        for stage, times in samples.items():
            if not isinstance(times, list) or len(times) != repetitions:
                raise ReportError("性能測定の反復数が一致しません")
            for time in times:
                integer(time, minimum=1)
            if type(medians[stage]) not in (int, float) or not math.isfinite(medians[stage]) or medians[stage] != statistics.median(times):
                raise ReportError("性能測定の中央値が生データと一致しません")
        ratio = medians["current"] / medians["base"] if "base" in samples else None
        if ratio is None:
            if row.get("status") != "NO_BASELINE" or row.get("current_over_base") is not None:
                raise ReportError("比較元なしの性能測定が不正です")
        else:
            compared += 1
            actual_ratio = row.get("current_over_base")
            if (type(actual_ratio) not in (int, float) or not math.isfinite(actual_ratio) or
                    not math.isclose(actual_ratio, ratio, rel_tol=1e-12) or row.get("status") != "MEASURED"):
                raise ReportError("性能比が生データと一致しません")
        if (row.get("independent_reference") != "passed" or not isinstance(row.get("checksum"), str)
                or not re.fullmatch(r"[0-9]{1,20}", row["checksum"]) or int(row["checksum"]) >= 1 << 64):
            raise ReportError("性能測定の独立照合を確認できません")
        if "base" in samples:
            sha(source.get("base_sha256"))
        key = (identifier, integer(row.get("size"), minimum=1), row.get("pattern"), integer(row.get("rounds"), minimum=1))
        if key in seen or key[2] not in {"random", "structured"}:
            raise ReportError("性能測定の条件が重複・不正です")
        seen.add(key)
        status = "STALE" if changed else "PASS" if value["status"] != "FAIL" and value.get("current_measurements_passed") is True else "FAIL"
        append(library, "performance", dict(status=status, id=identifier, scope="代表本体（入出力・Verify mainは除外）",
               size=key[1], pattern=key[2], rounds=key[3], repetitions=repetitions, warmups=value.get("warmups"), samples_ns=samples,
               median_ns=medians, current_over_base=ratio, comparison="MEASURED" if ratio is not None else "NO_BASELINE",
               compiler=value.get("compiler"), flags=value["flags"], cpu_model=value.get("cpu_model"),
               base_revision=value.get("base_revision"), source_mismatches=changed,
               advisory=True))
    if compared != value.get("compared_cases") or len(rows) - compared != value.get("uncompared_cases"):
        raise ReportError("性能測定の集約件数と実行結果が不一致です")
    if {row["id"] for row in rows} != set(sources):
        raise ReportError("性能測定の対象に結果がありません")


def collect_library_checker(value: dict, root: Path, entries: dict, append) -> None:
    selection = value.get("selection")
    rows = value.get("results")
    if not isinstance(selection, dict) or not isinstance(rows, list):
        raise ReportError("Library Checkerの選択・結果が不正です")
    selected = selection.get("selected_ids")
    if not isinstance(selected, list) or len(set(selected)) != len(selected) or any(x not in entries for x in selected):
        raise ReportError("Library Checkerの対象IDが不正です")
    if not selected:
        if value.get("status") != "SKIPPED" or rows:
            raise ReportError("対象なしのLibrary Checkerを通過として扱えません")
        return
    if value.get("status") not in {"PASS", "FAIL", "running"}:
        raise ReportError("Library Checkerの状態が不正です")
    official = value.get("official")
    if not isinstance(official, dict) or official.get("repository", "").removesuffix(".git") != "https://github.com/yosupo06/library-checker-problems":
        raise ReportError("公式問題集のリポジトリが一致しません")
    revision = official.get("revision")
    if not isinstance(revision, str) or not re.fullmatch(r"(?:[0-9a-f]{40}|[0-9a-f]{64})", revision):
        raise ReportError("公式問題集の固定revisionがありません")
    compiler = value.get("compiler")
    if not isinstance(compiler, dict) or not isinstance(compiler.get("version"), str) or not compiler["version"]:
        raise ReportError("Library Checkerのコンパイラ版がありません")
    text_list(compiler.get("command")); text_list(compiler.get("flags"))
    runner = value.get("runner")
    runner_changes = []
    if runner is not None:
        if not isinstance(runner, dict):
            raise ReportError("Library Checker runnerの来歴が不正です")
        runner_changes = mismatches(root, {relative(runner.get("source")): sha(runner.get("sha256"))})
    seen = set()
    for row in rows:
        if not isinstance(row, dict) or row.get("id") not in selected or row["id"] in seen:
            raise ReportError("Library Checkerの結果IDが不正です")
        seen.add(row["id"]); entry = entries[row["id"]]
        if row.get("libraries") != entry["libraries"] or row.get("driver") != entry["driver"] or row.get("submission") != entry["submission"]:
            raise ReportError("Library Checkerの対象ソースがmanifestと一致しません")
        recorded = hashes(row.get("source_hashes"))
        if set(recorded) != set(entry["libraries"]):
            raise ReportError("Library Checkerの本体SHAが不足しています")
        recorded[entry["driver"]] = sha(row.get("driver_sha256"))
        recorded[entry["submission"]] = sha(row.get("submission_sha256"))
        recorded.update(hashes(row.get("support_source_hashes", {}), nonempty=False))
        io_hashes = hashes(row.get("io_source_hashes", {}), nonempty=False)
        if any(recorded.get(name) != checksum for name, checksum in io_hashes.items()):
            raise ReportError("Library Checkerの入出力SHAに矛盾があります")
        if not set(entry.get("support_source_hashes", {})) <= recorded.keys():
            raise ReportError("Library Checkerの共通部品SHAが不足しています")
        changed = sorted(set(mismatches(root, recorded) + mismatches(root, entry_snapshot(entry)) + runner_changes))
        if row.get("status") not in {"PASS", "FAIL"}:
            raise ReportError("Library Checkerの結果状態が不正です")
        cases = row.get("cases")
        if not isinstance(cases, list):
            raise ReportError("Library Checkerのケース結果がありません")
        run_official = row.get("official", {})
        expected = run_official.get("expected_cases", [])
        if row["status"] == "PASS":
            if not isinstance(run_official, dict) or run_official.get("revision") != revision or run_official.get("repository", "").removesuffix(".git") != official["repository"].removesuffix(".git"):
                raise ReportError("Library Checkerの問題別revisionが一致しません")
            for name in ("info_sha256", "hash_json_sha256", "checker_sha256"):
                sha(run_official.get(name))
            problem = entry.get("problem_url", "")
            if problem != "https://judge.yosupo.jp/problem/" + str(run_official.get("problem_id")):
                raise ReportError("Library Checkerの問題がmanifestと一致しません")
            if not isinstance(expected, list) or not expected or len(set(expected)) != len(expected) or any(not isinstance(x, str) or not re.fullmatch(r"[A-Za-z0-9_.-]+", x) for x in expected):
                raise ReportError("Library Checkerの全ケース集合がありません")
        actual, passed, elapsed = set(), 0, 0
        for case in cases:
            if not isinstance(case, dict) or case.get("name") not in expected or case["name"] in actual:
                raise ReportError("Library Checkerのケースが未知・重複しています")
            actual.add(case["name"])
            if case.get("status") not in {"PASS", "FAIL"}:
                raise ReportError("Library Checkerのケース状態が不正です")
            if case["status"] == "PASS":
                if type(case.get("returncode")) is not int or case["returncode"] != 0 or type(case.get("checker_returncode")) is not int or case["checker_returncode"] != 0:
                    raise ReportError("Library CheckerのPASSとrun/checker結果が矛盾します")
                integer(case.get("elapsed_ns"), minimum=1)
                passed += 1
            if case.get("elapsed_ns") is not None:
                elapsed += integer(case["elapsed_ns"], minimum=1)
        counts = row.get("case_counts")
        if not isinstance(counts, dict) or counts != {"expected": len(expected), "completed": len(actual), "passed": passed, "failed": len(actual)-passed}:
            raise ReportError("Library Checkerのケース件数が一致しません")
        if row["status"] == "PASS" and (actual != set(expected) or passed != len(expected)):
            raise ReportError("Library Checkerの全ケースを通過していません")
        if cases or row["status"] == "PASS":
            if row.get("elapsed_ns") != elapsed:
                raise ReportError("Library Checkerの全体時間の集約が一致しません")
        elif row.get("elapsed_ns") not in (None, 0):
            raise ReportError("未実行Library Checkerの時間が不正です")
        compile_command = text_list(row.get("compile_command")) if row["status"] == "PASS" else row.get("compile_command", [])
        status = "STALE" if changed else row["status"]
        for library in entry["libraries"]:
            append(library, "library_checker", dict(id=row["id"], status=status,
                   scope="外部の公式ケースをローカル実行（オンラインACではない）", compiler=compiler,
                   cases=cases, case_counts=counts, elapsed_ns=elapsed if cases else None, official=run_official,
                   compile_command=compile_command, embedded_compile_settings=row.get("embedded_compile_settings", {}),
                   submission=row["submission"], submission_sha256=row["submission_sha256"],
                   source_hashes=recorded, runner=runner,
                   performance_scope=row.get("performance_scope"), source_mismatches=changed))
    if value["status"] == "PASS" and seen != set(selected):
        raise ReportError("PASS Library Checkerの選択対象に未実行があります")


def aggregate(root: Path, inputs: list[Path], *, source_manifest: dict[str, dict] | None = None) -> dict:
    root = root.resolve()
    entries = manifest_entries(root) if source_manifest is None else source_manifest
    libraries = {p.relative_to(root).as_posix(): {"path": p.relative_to(root).as_posix(),
                 "source_sha256": digest(p.read_bytes()),
                 "categories": {category: {"status": "UNAVAILABLE", "evidence": []} for category in CATEGORIES}}
                 for directory in sorted(LIBRARY_DIRS) for p in sorted((root / directory).rglob("*.cpp")) if p.is_file() and p.resolve().is_relative_to(root)}
    reports, errors = [], []
    for path in discover(inputs):
        evidence = []
        snapshot = path.read_bytes()
        descriptor = {"path": str(path), "sha256": digest(snapshot), "status": "UNAVAILABLE"}
        def append(library, category, row):
            relative(library, library=True)
            if library not in libraries:
                raise ReportError("測定した本体が現在の原本一覧にありません")
            evidence.append((library, category, dict(row, report=descriptor["sha256"])))
        try:
            value = decode_json(snapshot)
            if not isinstance(value, dict) or type(value.get("format")) is not int or value["format"] != 1:
                raise ReportError("レポート形式が不正です")
            if value.get("online_status") != "not_evaluated":
                raise ReportError("ローカル結果にオンライン判定が混在しています")
            if value.get("type") == "sirkit.library-checker":
                kind, collector = "library_checker", collect_library_checker
            elif value.get("type") == "sirkit.contracts":
                kind, collector = "contracts", collect_contracts
            elif value.get("mode") in {"sanitizers", "coverage"}:
                kind, collector = value["mode"], collect_instrumented
            elif value.get("scope") == "representative_normal_library_kernels":
                kind, collector = "performance", collect_performance
            elif "plan" in value:
                kind, collector = "normal", collect_normal
            else:
                raise ReportError("対応しないレポートです")
            descriptor.update(kind=kind, recorded_status=value.get("status"),
                              provenance=value.get("provenance", {}), compiler=value.get("compiler", value.get("tools", {}).get("compiler")))
            value["_quality_report_directory"] = str(path.parent)
            collector(value, root, entries, append)
            descriptor["status"] = combine([row for _, _, row in evidence])
            if value.get("status") == "FAIL":
                descriptor["status"] = "FAIL"
            for library, category, row in evidence:
                libraries[library]["categories"][category]["evidence"].append(row)
        except (ReportError, ValueError, KeyError, TypeError, OSError, AttributeError, OverflowError, IndexError) as error:
            descriptor.update(status="FAIL", error=str(error))
            errors.append({"report": descriptor["sha256"], "message": str(error)})
        reports.append(descriptor)
    for library in libraries.values():
        for category in library["categories"].values():
            category["status"] = combine(category["evidence"])
    global_states = {item["status"] for item in reports}
    run_state = "FAIL" if "FAIL" in global_states else "UNAVAILABLE" if not global_states else "PASS" if global_states == {"PASS"} else "PARTIAL"
    covered = [library["categories"][name]["status"] for library in libraries.values() for name in CATEGORIES]
    state = "FAIL" if run_state == "FAIL" else "UNAVAILABLE" if not global_states else "PASS" if covered and set(covered) == {"PASS"} else "PARTIAL"
    return {"format": 1, "type": "sirkit.quality-report", "generated_at_utc": datetime.now(timezone.utc).isoformat(),
            "status": state, "input_report_status": run_state, "online_status": "not_evaluated", "provenance": provenance(root),
            "scope": "各欄は記録されたケースだけの結果。未測定のAPIや入力を含む完全な保証ではない。性能差は参考値。",
            "categories": LABELS, "libraries": [libraries[key] for key in sorted(libraries)],
            "reports": reports, "errors": errors}


def inline(value: object) -> str:
    text = " ".join(str(value).split())
    text = "".join(c for c in text if ord(c) >= 32 and not 0x202A <= ord(c) <= 0x202E and not 0x2066 <= ord(c) <= 0x2069)
    return "<code>" + html.escape(text).replace("|", "&#124;").replace("`", "&#96;").replace("{", "&#123;").replace("}", "&#125;") + "</code>"


def markdown(report: dict) -> str:
    lines = ["# ローカルの品質検査", "", "オンラインACと分け、原本・テストのSHAが現在版と一致する結果だけを通過として表示します。",
             "正常系は入力例・明示的なAPI契約、失敗系はchecked版の期待する例外と失敗後の状態です。異常終了を失敗系の成功には数えません。",
             "Library Checker公式ケースは外部の固定問題集を使うローカル照合です。ケースの全体時間は起動・入出力を含み、本体だけの性能測定とは分けます。",
             "未測定・ソース変更・一部のみを区別します。性能は同じ条件内の参考値で、速度差だけでは合否を決めません。", "",
             "実行状態: " + STATUS_LABELS[report["status"]], "",
             "| 通常版本体 | " + " | ".join(LABELS.values()) + " |", "| --- | " + " | ".join("---" for _ in CATEGORIES) + " |"]
    for library in report["libraries"]:
        cells = []
        for category in CATEGORIES:
            value = library["categories"][category]
            checks = sum(item.get("checks", 0) for item in value["evidence"] if type(item.get("checks")) is int)
            detail = (f"のべ{checks}照合・{len(value['evidence'])}記録" if checks else f"{len(value['evidence'])}記録") if value["evidence"] else ""
            cells.append(STATUS_LABELS[value["status"]] + (" (" + detail + ")" if detail else ""))
        lines.append("| " + inline(library["path"]) + " | " + " | ".join(cells) + " |")
    official_rows = [(library, row) for library in report["libraries"] for row in library["categories"]["library_checker"]["evidence"]]
    if official_rows:
        lines += ["", "## Library Checker公式ケースのローカル実行", "",
                  "固定revisionの問題集を外部キャッシュに生成し、公式checkerで全ケースを照合します。オンラインACではありません。",
                  "時間は各ケースのプログラム起動・入出力を含む合計で、checkerは除外します。本体だけの性能値・過去のオンライン時間とは比較しません。",
                  "", "| 本体 | Verify ID | 通過/予定ケース | 全体 ms | 状態 |", "| --- | --- | ---: | ---: | --- |"]
        for library, row in official_rows:
            count = row["case_counts"]
            elapsed = f"{row['elapsed_ns']/1e6:.3f}" if row["elapsed_ns"] is not None else "未実行"
            lines.append(f"| {inline(library['path'])} | {inline(row['id'])} | {count['passed']}/{count['expected']} | {elapsed} | {STATUS_LABELS[row['status']]} |")
    measured = [(library, row) for library in report["libraries"] for row in library["categories"]["performance"]["evidence"]]
    if measured:
        lines += ["", "## 本体の性能測定の条件と結果", "",
                  "入出力・プロセス起動を除く代表カーネルです。CI環境の速度差は参考で、オンライン提出時間とは比較しません。",
                  "", "| コンパイラ | 設定 | CPU | 比較元commit | 反復/除外warmup |",
                  "| --- | --- | --- | --- | ---: |"]
        environments = set()
        for _, row in measured:
            key = json.dumps([row.get(name) for name in ("compiler", "flags", "cpu_model", "base_revision", "repetitions", "warmups")], sort_keys=True)
            if key in environments:
                continue
            environments.add(key)
            compiler = row.get("compiler")
            compiler = compiler.get("version", "記録なし") if isinstance(compiler, dict) else compiler
            lines.append("| " + " | ".join(inline(x) for x in (compiler, " ".join(row.get("flags", [])), row.get("cpu_model") or "記録なし", row.get("base_revision") or "記録なし")) + f" | {row['repetitions']}/{row.get('warmups') if row.get('warmups') is not None else '記録なし'} |")
        lines += ["", "| 本体 | 入力 | 要素数 | 現在 ms | 現在/基準 | 状態 |", "| --- | --- | ---: | ---: | ---: | --- |"]
        for library, row in measured:
            ratio = f"{row['current_over_base']:.3f}" if row["current_over_base"] is not None else "比較元なし"
            lines.append(f"| {inline(library['path'])} | {inline(row['pattern'])} | {row['size']} | {row['median_ns']['current']/1e6:.3f} | {ratio} | {STATUS_LABELS[row['status']]} |")
    if report["errors"]:
        lines += ["", "未完了・不正なレポートがあります。JSONのerrorsと元の実行ログを確認してください。"]
    return "\n".join(lines) + "\n"


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=Path, default=ROOT)
    parser.add_argument("--input", type=Path, action="append", default=[])
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args(argv)
    try:
        report = aggregate(args.root, args.input)
        args.output.mkdir(parents=True, exist_ok=True)
        (args.output / "result.json").write_text(json.dumps(report, ensure_ascii=False, indent=2) + "\n")
        (args.output / "summary.md").write_text(markdown(report))
        print(f"{report['status']}: {args.output / 'result.json'}")
        return int(report["status"] == "FAIL")
    except (OSError, ValueError, TypeError) as error:
        print(str(error), file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
