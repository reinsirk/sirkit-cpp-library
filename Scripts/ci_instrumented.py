#!/usr/bin/env python3
# SPDX-License-Identifier: CC0-1.0
"""通常版のchecked VerifyをASan/UBSanまたはソースカバレッジで実行する。"""
from __future__ import annotations

import argparse
from collections import defaultdict
from concurrent.futures import ThreadPoolExecutor
import hashlib
import html
import json
import os
from pathlib import Path, PurePosixPath
import re
import shutil
import subprocess
import sys
import tempfile

try:
    from . import ci_contracts, prepare_verify
    from .ci_verify_summary import inline_code
except ImportError:
    import ci_contracts
    import prepare_verify
    from ci_verify_summary import inline_code

ROOT = Path(__file__).resolve().parents[1]
LIBRARY_DIRS = {"Algorithm", "DataStructure", "Geometry", "Graph", "Internal", "Math", "String"}
COVERAGE_TESTS = "Verify/ci/coverage_tests.json"
COVERAGE_EXCLUSIONS = "Verify/ci/coverage_exclusions.json"
X86_BASELINE_CPU = "Nehalem,-popcnt"
X86_AVX2_CPU = "Haswell-noTSX"
SANITIZER_ENV = {
    "ASAN_OPTIONS": "detect_leaks=1:halt_on_error=1:abort_on_error=1:allocator_may_return_null=0",
    "UBSAN_OPTIONS": "halt_on_error=1:print_stacktrace=1",
    "LSAN_OPTIONS": "exitcode=1",
}
CHECK_GUARD = ('#if defined(SKIP_RUNTIME_CHECKS) || defined(NDEBUG)\n'
               '#error "instrumented Verify requires enabled runtime checks and assertions"\n#endif\n')
UNCHECKED_GUARD = ('#if !defined(SKIP_RUNTIME_CHECKS) || defined(NDEBUG)\n'
                   '#error "supplemental coverage requires SKIP_RUNTIME_CHECKS and enabled assertions"\n#endif\n')


def digest(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def coverage_tests(root: Path) -> list[dict]:
    """提出用manifestとは別に、単独で結果を検査するテストを登録する。"""
    registry = root / COVERAGE_TESTS
    value = json.loads(registry.read_text(encoding="utf-8"))
    if (not isinstance(value, dict) or type(value.get("format")) is not int or value["format"] != 1
            or not isinstance(value.get("tests"), dict) or not value["tests"]):
        raise ValueError("補足カバレッジテストの登録形式が不正です")
    support = value.get("support", {})
    if not isinstance(support, dict) or not support.keys() <= value["tests"].keys():
        raise ValueError("補足カバレッジテストの依存ファイルの登録が不正です")
    failure_tests = value.get("failure_tests", {})
    if not isinstance(failure_tests, dict):
        raise ValueError("失敗系テストの登録形式が不正です")
    failures_by_test = {}
    for library, name in failure_tests.items():
        libraries = value["tests"].get(name) if isinstance(name, str) else None
        if not isinstance(libraries, list) or library not in libraries:
            raise ValueError("失敗系テストの対象原本が補足カバレッジと一致しません: " + library)
        failures_by_test.setdefault(name, []).append(library)
    gcc_line_tests = value.get("gcc_line_tests", [])
    if (not isinstance(gcc_line_tests, list)
            or any(not isinstance(name, str) or name not in value["tests"] for name in gcc_line_tests)
            or len(set(gcc_line_tests)) != len(gcc_line_tests)):
        raise ValueError("GCC行計測の対象テストが不正です")
    x86_baseline_tests = value.get("x86_baseline_tests", [])
    if (not isinstance(x86_baseline_tests, list)
            or any(not isinstance(name, str) or name not in value["tests"] for name in x86_baseline_tests)
            or len(set(x86_baseline_tests)) != len(x86_baseline_tests)):
        raise ValueError("x86代替経路の計測対象が不正です")
    i386_tests = value.get("i386_tests", [])
    if (not isinstance(i386_tests, list)
            or any(not isinstance(name, str) or name not in value["tests"] for name in i386_tests)
            or len(set(i386_tests)) != len(i386_tests)
            or set(i386_tests) & (set(gcc_line_tests) | set(x86_baseline_tests))):
        raise ValueError("i386計測の対象テストが不正です")
    gcc_unchecked_line_tests = value.get("gcc_unchecked_line_tests", [])
    if (not isinstance(gcc_unchecked_line_tests, list)
            or any(not isinstance(name, str) or name not in value["tests"] for name in gcc_unchecked_line_tests)
            or len(set(gcc_unchecked_line_tests)) != len(gcc_unchecked_line_tests)
            or set(gcc_unchecked_line_tests) & set(i386_tests)):
        raise ValueError("検査省略GCC行計測の対象テストが不正です")
    clang_unchecked_tests = value.get("clang_unchecked_tests", [])
    if (not isinstance(clang_unchecked_tests, list)
            or any(not isinstance(name, str) or name not in value["tests"] for name in clang_unchecked_tests)
            or len(set(clang_unchecked_tests)) != len(clang_unchecked_tests)
            or set(clang_unchecked_tests) & set(i386_tests)):
        raise ValueError("検査省略Clang計測の対象テストが不正です")
    x86_avx2_tests = value.get("x86_avx2_tests", [])
    if (not isinstance(x86_avx2_tests, list)
            or any(not isinstance(name, str) or name not in value["tests"] for name in x86_avx2_tests)
            or len(set(x86_avx2_tests)) != len(x86_avx2_tests)
            or set(x86_avx2_tests) & set(i386_tests)):
        raise ValueError("x86 AVX2計測の対象テストが不正です")
    x86_binary128_tests = value.get("x86_binary128_tests", [])
    if (not isinstance(x86_binary128_tests, list)
            or any(not isinstance(name, str) or name not in value["tests"] for name in x86_binary128_tests)
            or len(set(x86_binary128_tests)) != len(x86_binary128_tests)
            or set(x86_binary128_tests) & set(i386_tests)):
        raise ValueError("x86 binary128計測の対象テストが不正です")
    entries = []
    for name, libraries in value["tests"].items():
        if not re.fullmatch(r"[a-zA-Z0-9_]+_test\.cpp", name):
            raise ValueError("補足カバレッジテスト名が不正です: " + name)
        if (not isinstance(libraries, list) or not libraries or any(not isinstance(path, str) for path in libraries)
                or len(libraries) != len(set(libraries))):
            raise ValueError("補足カバレッジテストの対象原本がありません: " + name)
        dependencies = support.get(name, [])
        if not isinstance(dependencies, list) or any(
            not isinstance(path, str)
            or not path.startswith("Verify/current/")
            or ".." in PurePosixPath(path).parts
            or not (root / path).resolve(strict=True).is_relative_to(root.resolve())
            for path in dependencies
        ):
            raise ValueError("補足カバレッジテストの依存ファイルが不正です: " + name)
        entry = {
            "id": "coverage_test_" + Path(name).stem,
            "driver": "Verify/current/tests/" + name,
            "libraries": libraries,
            "coverage_test": "assertions",
            "support_source_hashes": {path: digest(root / path) for path in dependencies},
            "samples": [{"input": ""}],
        }
        if name in failures_by_test:
            entry["failure_libraries"] = failures_by_test[name]
        if name in gcc_line_tests:
            entry["gcc_line_coverage"] = True
        if name in gcc_unchecked_line_tests:
            entry["gcc_unchecked_line_coverage"] = True
        if name in clang_unchecked_tests:
            entry["clang_unchecked_coverage"] = True
        if name in x86_avx2_tests:
            entry["x86_avx2_coverage"] = True
        if name in x86_binary128_tests:
            entry["x86_binary128_coverage"] = True
        if name in x86_baseline_tests:
            entry["x86_baseline_coverage"] = True
        if name in i386_tests:
            entry["coverage_target"] = "i386"
        entries.append(entry)
    entries.append({
        "id": "coverage_test_api_contracts",
        "driver": ci_contracts.FIXTURE,
        "libraries": list(ci_contracts.LIBRARIES),
        "coverage_test": "contracts",
        "samples": [{"input": ""}],
    })
    return entries


def output_matches(entry: dict, sample: dict, output: str) -> bool:
    if entry.get("coverage_test") == "contracts":
        ci_contracts.parse_results(output)
        return True
    if entry.get("coverage_test") == "assertions":
        return True  # 独立参照・期待例外との不一致はテスト自身が失敗で終了する。
    return prepare_verify.matches(entry, sample, output)


def select_entries(entries: list[dict], requested: list[str] | None) -> list[dict]:
    identifiers = [entry["id"] for entry in entries]
    if len(set(identifiers)) != len(identifiers):
        raise ValueError("Verify IDが重複しています")
    if requested:
        unknown = set(requested) - set(identifiers)
        if unknown:
            raise ValueError("存在しないVerify ID: " + ", ".join(sorted(unknown)))
        return [entry for entry in entries if entry["id"] in requested]
    return list(entries)


def library_paths(entries: list[dict], root: Path) -> list[Path]:
    paths = set()
    root = root.resolve()
    for entry in entries:
        for name in entry["libraries"]:
            relative = PurePosixPath(name)
            if (relative.is_absolute() or ".." in relative.parts or not relative.parts
                    or relative.parts[0] not in LIBRARY_DIRS or relative.suffix != ".cpp"
                    or any(part in {"Fast", "Variants", "vendor", "submissions"} for part in relative.parts)):
                raise ValueError("通常版以外のlibrary pathは対象にできません")
            path = (root / name).resolve(strict=True)
            if not path.is_relative_to(root) or not path.is_file():
                raise ValueError("library pathがリポジトリ外を参照しています")
            paths.add(path)
    return sorted(paths)


def translation_unit(entry: dict, root: Path, *, unchecked: bool = False) -> str:
    library_paths([entry], root)
    # include順は提出版とそろえる。coverageの位置は貼付コードでなく原本へ帰属する。
    paths = [(root / name).resolve() for name in entry["libraries"]]
    driver = (root / entry["driver"]).resolve(strict=True)
    if not driver.is_relative_to(root.resolve()) or not driver.is_file():
        raise ValueError("driverはリポジトリ内の通常fixtureを指定してください")
    if any(part in {"Fast", "Variants", "vendor", "submissions"} for part in driver.relative_to(root.resolve()).parts):
        raise ValueError("配布派生版・連結提出版のdriverは対象外です")
    if entry.get("coverage_test"):
        # 単独テストが指定したinclude順と設定を保持する。
        guard = UNCHECKED_GUARD if unchecked else CHECK_GUARD
        return guard + '#include ' + json.dumps(str(driver), ensure_ascii=False) + "\n" + guard
    if unchecked:
        raise ValueError("検査省略の補足計測には単独テストを指定してください")
    libraries = "".join('#include ' + json.dumps(str(path), ensure_ascii=False) + "\n" + CHECK_GUARD
                        for path in paths)
    return (CHECK_GUARD + libraries + "namespace sirkit {}\nusing namespace sirkit;\n" +
            '#include ' + json.dumps(str(driver), ensure_ascii=False) + "\n" + CHECK_GUARD)


def runtime_environment(mode: str, profile_pattern: str | None = None,
                        base: dict[str, str] | None = None) -> dict[str, str]:
    environment = dict(os.environ if base is None else base)
    if mode == "sanitizers":
        environment.update(SANITIZER_ENV)
        environment.pop("ASAN_SYMBOLIZER_PATH", None)
    elif mode == "coverage" and profile_pattern:
        environment["LLVM_PROFILE_FILE"] = profile_pattern
    else:
        raise ValueError("計測modeまたはprofile出力先が不正です")
    return environment


def command_version(command: str, clang: bool = False) -> tuple[str, int]:
    result = subprocess.run([command, "--version"], check=True, capture_output=True,
                            text=True, timeout=30)
    text = result.stdout + result.stderr
    match = re.search(r"clang version\s+(\d+)" if clang else r"LLVM version\s+(\d+)", text, re.I)
    if not match:
        raise ValueError("指定されたClang/LLVMの版を確認できません: " + command)
    return text.strip(), int(match[1])


def gcc_install_arguments(compiler: str, explicit: str | None) -> list[str]:
    if explicit:
        path = Path(explicit).resolve(strict=True)
        if not path.is_dir():
            raise ValueError("gcc-install-dirはディレクトリを指定してください")
        return ["--gcc-install-dir=" + str(path)]
    found = subprocess.run([compiler, "-print-file-name=libstdc++.so"], check=True,
                           capture_output=True, text=True, timeout=30).stdout.strip()
    if Path(found).is_absolute() and Path(found).is_file():
        return []
    # Clangの自動選択が未完成のGCCを拾う環境では、利用可能なsystem g++へ合わせる。
    driver = shutil.which("g++", path=os.defpath) or shutil.which("g++")
    if driver:
        found = subprocess.run([driver, "-print-libgcc-file-name"], check=True,
                               capture_output=True, text=True, timeout=30).stdout.strip()
        if Path(found).is_absolute() and Path(found).is_file():
            return ["--gcc-install-dir=" + str(Path(found).parent)]
    return []


def make_acl_overlay(root: Path, destination: Path) -> None:
    for header in (root / "Verify/current/vendor/atcoder").glob("*.hpp"):
        wrapper = destination / "atcoder" / header.stem
        wrapper.parent.mkdir(parents=True, exist_ok=True)
        wrapper.write_text('#include ' + json.dumps(str(header.resolve()), ensure_ascii=False) + "\n", encoding="utf-8")


def filter_coverage(export: dict, allowed: list[Path]) -> dict:
    """原本の許可リストだけを残し、分母もそのfile summaryから再集計する。"""
    selected = {str(path.resolve()) for path in allowed}
    files = {}
    for data in export["data"]:
        for item in data["files"]:
            name = str(Path(item["filename"]).resolve())
            if name not in selected:
                continue
            if name in files:
                raise ValueError("coverageの原本fileが重複しています")
            files[name] = {**item, "filename": name}
    totals = {}
    for item in files.values():
        for name, metric in item["summary"].items():
            total = totals.setdefault(name, {"count": 0, "covered": 0})
            total["count"] += metric["count"]
            total["covered"] += metric["covered"]
    for total in totals.values():
        total["percent"] = 100 * total["covered"] / total["count"] if total["count"] else 0
    return {"type": export["type"], "version": export["version"],
            "data": [{"files": [files[name] for name in sorted(files)], "totals": totals}]}


def source_coverage(export: dict, lcov: str, allowed: list[Path]) -> dict:
    """行はLLVMのDA、分岐は原本regionのtrue/false。"""
    if not isinstance(export.get("version"), str) or export["version"].split(".")[0] not in {"2", "3"}:
        raise ValueError("未対応のLLVM coverage JSON形式です")
    filtered = filter_coverage(export, allowed)
    files = {item["filename"]: {"filename": item["filename"], "lines": {}, "branches": {}}
             for item in filtered["data"][0]["files"]}
    current = None
    for record in lcov.splitlines():
        if record.startswith("SF:"):
            current = files.get(str(Path(record[3:]).resolve()))
        elif record == "end_of_record":
            current = None
        elif current is not None and record.startswith("DA:"):
            line, count, *_ = record[3:].split(",")
            line, count = int(line), int(count)
            if line <= 0 or count < 0:
                raise ValueError("coverageの行counterが不正です")
            current["lines"][line] = current["lines"].get(line, False) or count > 0
    for item in filtered["data"][0]["files"]:
        branches = files[item["filename"]]["branches"]
        # expansions内部のbranchは除外する。直接原本に記述されたconditionだけを集計。
        for branch in item.get("branches", []):
            if branch[6] != branch[7]:
                continue
            region = tuple(branch[:4])
            old = branches.get(region, (False, False))
            branches[region] = (old[0] or branch[4] > 0, old[1] or branch[5] > 0)
    result = []
    for name, item in sorted(files.items()):
        branches = item["branches"]
        result.append({"filename": name, "lines": sorted(item["lines"].items()),
                       "branches": [list(region) + list(hits) for region, hits in sorted(branches.items())]})

    # LLVMのファイル別JSONはFoldedも0/0で出力する。関数別JSONには実行時分岐だけが残る。
    # getCoverageForFileと同じfile IDの条件でマクロ展開内部を除き、原本座標へ戻す。
    path_cache = {}
    runtime = {name: set() for name in files}
    for data in export["data"]:
        if not isinstance(data.get("functions"), list):
            raise ValueError("LLVMの関数別分岐がありません")
        for function in data["functions"]:
            filenames = []
            for name in function["filenames"]:
                if name not in path_cache:
                    path_cache[name] = str(Path(name).resolve())
                filenames.append(path_cache[name])
            for branch in function["branches"]:
                file_id, expanded_id = branch[6:8]
                if file_id == expanded_id and filenames[file_id] in runtime:
                    runtime[filenames[file_id]].add(tuple(branch[:4]))
    for row in result:
        name = row["filename"]
        active = runtime[name]
        branch_map = files[name]["branches"]
        folded = branch_map.keys() - active
        if active - branch_map.keys() or any(any(branch_map[region]) for region in folded):
            raise ValueError("LLVMのファイル別と関数別の分岐が一致しません: " + name)
        if folded:
            row["folded_branches"] = [list(region) for region in sorted(folded)]
    return {"files": result}


def record_gcov_lines(measured: dict, report: dict, *, unchecked: bool = False) -> None:
    """別のGCC実行で得た正の行カウンタを、LLVMの生データと一緒に記録する。"""
    if (not isinstance(report, dict) or report.get("format_version") not in ("1", "2")
            or not isinstance(report.get("current_working_directory"), str)
            or not isinstance(report.get("gcc_version"), str) or not report["gcc_version"]
            or not isinstance(report.get("files"), list)):
        raise ValueError("gcovのJSON形式が不正です")
    directory = Path(report["current_working_directory"])
    llvm_files = {item["filename"]: item for item in measured["files"] if item["lines"]}
    matched = set()
    for file in report["files"]:
        if not isinstance(file, dict) or not isinstance(file.get("file"), str):
            raise ValueError("gcovの原本パスが不正です")
        name = str((directory / file["file"]).resolve())
        if name not in llvm_files:
            continue
        if name in matched or not isinstance(file.get("lines"), list) or not file["lines"]:
            raise ValueError("gcovの原本行が空または原本が重複しています: " + name)
        matched.add(name)
        counts = {}
        for row in file["lines"]:
            if not isinstance(row, dict):
                raise ValueError("gcovの行形式が不正です: " + name)
            line, count = row.get("line_number"), row.get("count")
            if type(line) is not int or line <= 0 or type(count) is not int or count < 0:
                raise ValueError("gcovの行counterが不正です: " + name)
            counts[line] = max(counts.get(line, 0), count)
        key = "gcov_unchecked_line_counts" if unchecked else "gcov_line_counts"
        llvm_files[name][key] = sorted((line, count) for line, count in counts.items() if count)
    if not matched or matched != llvm_files.keys():
        raise ValueError("gcovとLLVMの原本の対応が不完全です")


def shared_coverage_blocks(root: Path) -> list[list[tuple[str, int, int]]]:
    """スタンドアロン版に複製した同一ガード内の原本座標を照合する。"""
    directive = re.compile(rb"\s*#\s*(if|ifdef|ifndef|elif|else|endif)\b(.*)")
    guard = re.compile(rb"SIRKIT_[A-Z0-9_]+_DEFINED")
    groups = defaultdict(list)
    sources = sorted(path for folder in LIBRARY_DIRS for path in (root / folder).rglob("*.cpp"))
    for source in sources:
        lines = source.read_bytes().splitlines(keepends=True)
        stack = []
        for index, line in enumerate(lines):
            match = directive.fullmatch(line.rstrip(b"\r\n"))
            if not match:
                continue
            kind, condition = match.groups()
            if kind in (b"if", b"ifdef", b"ifndef"):
                stack.append((index, kind, condition.strip(), [line.strip()]))
            elif stack and kind == b"endif":
                start, opening, name, arms = stack.pop()
                if opening == b"ifndef" and guard.fullmatch(name) and len(arms) == 1:
                    body = lines[start:index + 1]
                    definition = next(row for row in body[1:] if row.strip())
                    if not re.fullmatch(rb"\s*#\s*define\s+" + name + rb"\s*", definition):
                        continue
                    context = tuple(tuple(frame[3]) for frame in stack)
                    groups[context, b"".join(body)].append((str(source.resolve()), start + 1, index + 1))
            elif stack:
                stack[-1][3].append(line.strip())
    return [copies for copies in groups.values() if len({name for name, _, _ in copies}) > 1]


def relative_region(region: tuple[int, int, int, int], start: int) -> tuple[int, int, int, int]:
    first, column, last, end_column = region
    return first - start, column, last - start, end_column


def union_shared_coverage(combined: dict, shared: list[list[tuple[str, int, int]]],
                          direct_exclusions: dict[str, set[tuple]]) -> None:
    for copies in shared:
        measured = [(name, combined[name], start, end) for name, start, end in copies if name in combined]
        if len(measured) < 2:
            continue
        hit_lines = set()
        branch_hits = defaultdict(lambda: (False, False))
        for _, item, start, end in measured:
            gcc_lines = item.get("gcov_line_counts", {}).keys() | item.get("gcov_unchecked_line_counts", {}).keys()
            hit_lines.update(line - start for line, hit in item["lines"].items()
                             if start <= line <= end and (hit or line in gcc_lines))
            for region, hits in item["branches"].items():
                if start <= region[0] <= region[2] <= end and any(hits):
                    key = relative_region(region, start)
                    branch_hits[key] = tuple(a or b for a, b in zip(branch_hits[key], hits))
        for name, item, start, end in measured:
            excluded = direct_exclusions.get(name, set())
            for line in item["lines"]:
                if start <= line <= end and line - start in hit_lines and (line,) not in excluded:
                    item["lines"][line] = True
            for region in item["runtime"]:
                if start <= region[0] <= region[2] <= end:
                    hits = branch_hits.get(relative_region(region, start))
                    if hits:
                        old = item["branches"][region]
                        item["branches"][region] = (
                            old[0] or (hits[0] and (*region, "true") not in excluded),
                            old[1] or (hits[1] and (*region, "false") not in excluded),
                        )


def coverage_union(exports: list[dict], shared: list[list[tuple[str, int, int]]] | None = None, *,
                   direct_exclusions: dict[str, set[tuple]] | None = None) -> dict:
    combined = {}
    for export in exports:
        for item in export["files"]:
            target = combined.setdefault(item["filename"], {"lines": {}, "branches": {}, "runtime": set()})
            folded = {tuple(region) for region in item.get("folded_branches", [])}
            for line, hit in item["lines"]:
                target["lines"][line] = target["lines"].get(line, False) or hit
            for *region, true_hit, false_hit in item["branches"]:
                key = tuple(region)
                old = target["branches"].get(key, (False, False))
                target["branches"][key] = (old[0] or true_hit, old[1] or false_hit)
                if key not in folded or true_hit or false_hit:
                    target["runtime"].add(key)
            for key in ("gcov_line_counts", "gcov_unchecked_line_counts"):
                for line, count in item.get(key, []):
                    counts = target.setdefault(key, {})
                    counts[line] = max(counts.get(line, 0), count)
    if shared:
        union_shared_coverage(combined, shared, direct_exclusions or {})

    def metric(count, covered):
        return {"count": count, "covered": covered, "percent": 100 * covered / count if count else 0}
    files = []
    for name, item in sorted(combined.items()):
        llvm_lines = item["lines"]
        checked = item.get("gcov_line_counts", {})
        unchecked = item.get("gcov_unchecked_line_counts", {})
        lines = sorted((line, hit or line in checked or line in unchecked) for line, hit in llvm_lines.items())
        summary = {"lines": metric(len(lines), sum(hit for _, hit in lines)),
                   "branches": metric(2 * len(item["branches"]), sum(sum(hits) for hits in item["branches"].values()))}
        file = {"filename": name, "lines": lines,
                "branches": [list(region) + list(hits) for region, hits in sorted(item["branches"].items())],
                "summary": summary}
        for key, counts in (("gcov_line_counts", checked), ("gcov_unchecked_line_counts", unchecked)):
            confirmed = sorted((line, counts[line]) for line, hit in llvm_lines.items() if not hit and line in counts)
            if confirmed:
                file[key] = confirmed
        folded = item["branches"].keys() - item["runtime"]
        if folded:
            file["folded_branches"] = [list(region) for region in sorted(folded)]
        files.append(file)
    totals = {key: metric(sum(item["summary"][key]["count"] for item in files),
                          sum(item["summary"][key]["covered"] for item in files))
              for key in ("lines", "branches")}
    definition = ("Per-TU LLVM LCOV DA lines and direct-source runtime condition regions; selected GCC runs can confirm "
                  "LLVM-mapped lines with positive gcov counts; boolean union across TUs and template instances; "
                  "macro expansion interiors and confirmed compiler-folded conditions excluded from the gate.")
    if shared:
        definition += " Identical shared guard blocks also share hits at matching measured coordinates."
    return {"format": 1, "type": "sirkit.source-coverage-union", "files": files, "totals": totals,
            "definition": definition}


def exclusion_key(exclusion: dict) -> tuple:
    if "line" in exclusion:
        return (exclusion["line"],)
    return (*exclusion["region"], exclusion["side"])


def direct_exclusion_coordinates(root: Path, exclusions: dict) -> dict[str, set[tuple]]:
    return {str((root / name).resolve()): {exclusion_key(row) for row in file["regions"] if "shared_from" not in row}
            for name, file in exclusions.items()}


def shared_exclusions(root: Path, registered: dict, blocks: list[list[tuple[str, int, int]]]) -> dict:
    sources = {name: file["regions"] for name, file in registered.items() if file["sha256"] == digest(root / name)}
    result = {name: {**file, "regions": list(file["regions"])} for name, file in registered.items()}
    names = {name: Path(name).relative_to(root.resolve()).as_posix()
             for copies in blocks for name, _, _ in copies}
    seen = {name: {exclusion_key(row) for row in file["regions"]} for name, file in result.items()}
    for copies in blocks:
        for original, first, last in copies:
            source = names[original]
            for row in sources.get(source, []):
                start, end = (row["line"], row["line"]) if "line" in row else (row["region"][0], row["region"][2])
                if not first <= start <= end <= last:
                    continue
                for embedded, embedded_start, _ in copies:
                    if embedded == original:
                        continue
                    name = names[embedded]
                    offset = embedded_start - first
                    if "line" in row:
                        moved = {**row, "line": start + offset}
                    else:
                        moved = {**row, "region": list(relative_region(tuple(row["region"]), -offset))}
                    key = exclusion_key(moved)
                    keys = seen.setdefault(name, set())
                    if key in keys:
                        continue
                    keys.add(key)
                    if name not in result:
                        result[name] = {"sha256": digest(Path(embedded)), "regions": []}
                    result[name]["regions"].append({**moved, "shared_from": source})
    return result


def load_coverage_exclusions(root: Path, shared: list[list[tuple[str, int, int]]] | None = None) -> dict:
    path = root / COVERAGE_EXCLUSIONS
    if not path.exists():
        return {}
    value = json.loads(path.read_text(encoding="utf-8"))
    if (not isinstance(value, dict) or type(value.get("format")) is not int or value["format"] != 1
            or not isinstance(value.get("files"), dict)):
        raise ValueError("到達不能箇所の登録形式が不正です")
    for name, file in value["files"].items():
        source, = library_paths([{"libraries": [name]}], root)
        if source.relative_to(root.resolve()).as_posix() != name:
            raise ValueError("到達不能箇所の原本パスが不正です: " + name)
        line_count = len(source.read_text(encoding="utf-8").splitlines())
        if (not isinstance(file, dict) or not isinstance(file.get("sha256"), str)
                or not re.fullmatch(r"[0-9a-f]{64}", file["sha256"])
                or not isinstance(file.get("regions"), list) or not file["regions"]):
            raise ValueError("到達不能箇所の原本SHA・座標がありません: " + name)
        seen = set()
        for entry in file["regions"]:
            if not isinstance(entry, dict) or not isinstance(entry.get("reason"), str) or not entry["reason"].strip():
                raise ValueError("到達不能の理由がありません: " + name)
            if set(entry) == {"line", "reason"} and type(entry["line"]) is int and 0 < entry["line"] <= line_count:
                key = (entry["line"],)
            elif (set(entry) == {"region", "side", "reason"} and isinstance(entry["region"], list)
                    and len(entry["region"]) == 4 and all(type(n) is int and n > 0 for n in entry["region"])
                    and entry["region"][:2] < entry["region"][2:] and entry["region"][2] <= line_count
                    and entry["side"] in ("true", "false")):
                key = (*entry["region"], entry["side"])
            else:
                raise ValueError("行または分岐片側の座標が不正です: " + name)
            if key in seen:
                raise ValueError("到達不能箇所の座標が重複しています: " + name)
            seen.add(key)
    if shared is None:
        shared = shared_coverage_blocks(root)
    return shared_exclusions(root, value["files"], shared)


def coverage_file_summary(item: dict, name: str, source_sha256: str, exclusions: dict) -> tuple[dict, list[dict]]:
    """座標が一致する未到達箇所だけを分母から引く。生の計測値は保持する。"""
    lines = dict(item["lines"])
    branches = {tuple(row[:4]): row[4:] for row in item["branches"]}
    folded = set()
    folded_rows = item.get("folded_branches", [])
    if not isinstance(folded_rows, list):
        raise ValueError("コンパイル時確定分岐の形式が不正です: " + name)
    for row in folded_rows:
        if not isinstance(row, list) or len(row) != 4 or any(type(n) is not int for n in row):
            raise ValueError("コンパイル時確定分岐の座標が不正です: " + name)
        key = tuple(row)
        if key in folded or key not in branches or any(branches[key]):
            raise ValueError("コンパイル時確定分岐が原本の計測と一致しません: " + name)
        folded.add(key)
    registered = exclusions.get(name)
    applied = []
    removed = {"lines": 0, "branches": 2 * len(folded)}
    if registered:
        if registered["sha256"] != source_sha256:
            raise ValueError("到達不能箇所の証明は現在の原本と一致しません: " + name)
        for exclusion in registered["regions"]:
            if "line" in exclusion:
                key = exclusion["line"]
                if key not in lines:
                    continue
                hit, kind = lines[key], "lines"
            else:
                key = tuple(exclusion["region"])
                if key not in branches:
                    continue
                side = 0 if exclusion["side"] == "true" else 1
                hit, kind = branches[key][side], "branches"
            if hit and "shared_from" in exclusion:
                # 共有部品の呼出し条件が異なり、移した座標が到達済みなら実測を使う。
                continue
            if hit:
                raise ValueError(f"到達不能とした箇所に到達しました: {name}:{key}")
            if kind == "branches" and key in folded:
                continue
            applied.append(exclusion)
            removed[kind] += 1
    metrics = {}
    for kind, count, covered in (
        ("lines", len(lines), sum(lines.values())),
        ("branches", 2 * len(branches), sum(map(sum, branches.values()))),
    ):
        count -= removed[kind]
        metrics[kind] = {"count": count, "covered": covered, "percent": 100 * covered / count if count else 0}
    return metrics, applied


def apply_coverage_exclusions(coverage: dict, root: Path, source_hashes: dict, exclusions: dict) -> dict:
    files = []
    for item in coverage["files"]:
        name = Path(item["filename"]).relative_to(root).as_posix()
        summary, applied = coverage_file_summary(item, name, source_hashes[name], exclusions)
        files.append({**item, "summary": summary, "exclusions": applied})
    totals = {}
    for key in ("lines", "branches"):
        count = sum(item["summary"][key]["count"] for item in files)
        covered = sum(item["summary"][key]["covered"] for item in files)
        totals[key] = {"count": count, "covered": covered, "percent": 100 * covered / count if count else 0}
    return {**coverage, "files": files, "totals": totals, "raw_totals": coverage["totals"]}


def coverage_gate(coverage: dict, allowed: list[Path], *, expected_translation_units: int,
                  measured_translation_units: int) -> dict:
    """確定した分母に対し、整数の到達件数で100%を必須にする。"""
    metrics = {}
    for key in ("lines", "branches"):
        metric = coverage["totals"][key]
        count, covered = metric["count"], metric["covered"]
        if type(count) is not int or type(covered) is not int or not 0 <= covered <= count:
            raise ValueError("coverageの到達件数が不正です")
        metrics[key] = {"count": count, "covered": covered, "uncovered": count - covered}
    mapped = {item["filename"] for item in coverage["files"]
              if len(item["lines"]) > sum("line" in exclusion for exclusion in item.get("exclusions", []))}
    unmapped = sorted({str(path.resolve()) for path in allowed} - mapped)
    complete = expected_translation_units > 0 and measured_translation_units == expected_translation_units
    reasons = []
    if not metrics["lines"]["count"]:
        reasons.append("no_mapped_lines")
    for key in ("lines", "branches"):
        if metrics[key]["uncovered"]:
            reasons.append("uncovered_" + key)
    if unmapped:
        reasons.append("unmapped_sources")
    if not complete:
        reasons.append("incomplete_translation_units")
    return {"required_percent": 100, "status": "FAIL" if reasons else "PASS", **metrics,
            "unmapped_sources": unmapped, "expected_translation_units": expected_translation_units,
            "measured_translation_units": measured_translation_units,
            "complete_translation_units": complete,
            "nonzero_mapped_lines": bool(metrics["lines"]["count"]), "reasons": reasons}


def coverage_rate(metric: dict, kind: str) -> str:
    if not metric["count"]:
        return "直接分岐なし" if kind == "branches" else "計測なし"
    if metric["covered"] == metric["count"]:
        return "100%"
    percent = f"{metric['percent']:.3f}%"
    return "<100%" if percent == "100.000%" else percent


def write_coverage_html(coverage: dict, root: Path, directory: Path) -> None:
    def rate_cells(summary: dict) -> str:
        cells = []
        for kind in ("lines", "branches"):
            metric = summary[kind]
            covered, count = metric["covered"], metric["count"]
            state = "not-measured" if not count else "hit" if covered == count else "miss"
            cells.append(f'<td class="rate {state}"><strong>{html.escape(coverage_rate(metric, kind))}</strong> '
                         f'<span class="figures">({covered:,}/{count:,})</span></td>')
        return "".join(cells)

    directory.mkdir()
    style = ('<style>body{font-family:system-ui;margin:1.5rem;color:#18252e}'
             'table{border-collapse:collapse}td,th{padding:.25em .65em;border:1px solid #b5bec7}'
             'th{background:#eef2f6;text-align:left}'
             '.rate,.line-number{text-align:right;white-space:nowrap;font-variant-numeric:tabular-nums}'
             '.figures{font-size:.85em;color:#364655}'
             '.source-table thead th{position:sticky;top:0}'
             'tr[id]{scroll-margin-top:3rem}tr:target>td{border-top:2px solid #8b4513}'
             '.line-number a{color:inherit}.line-state{white-space:nowrap}'
             'pre{margin:0}.hit{background:#e0f4e7}.miss{background:#ffe3de}'
             '.not-measured{background:#eef2f6}</style>')
    header = ('<!doctype html><html lang="ja"><meta charset="utf-8">'
              '<meta name="viewport" content="width=device-width, initial-scale=1">' + style)
    index = [header, '<title>原本ソースの到達和集合</title><h1>原本ソースの到達和集合</h1>',
             '<p>括弧内は到達件数／計測件数です。赤いセルに未到達があります。</p>',
             '<table><thead><tr><th scope="col">原本</th><th scope="col">行カバレッジ</th>'
             '<th scope="col">分岐カバレッジ</th></tr></thead><tbody>',
             '<tr><th scope="row">全体</th>' + rate_cells(coverage["totals"]) + '</tr>']
    index_rows = []
    for number, item in enumerate(coverage["files"]):
        relative = html.escape(str(Path(item["filename"]).relative_to(root)))
        target = f"source_{number:04}.html"
        row = f'<tr><td><a href="{target}">{relative}</a></td>' + rate_cells(item["summary"]) + '</tr>'
        complete = all(metric["covered"] == metric["count"] for metric in item["summary"].values())
        index_rows.append((complete, row))
        lines = dict(item["lines"])
        gcov_lines = dict(item.get("gcov_line_counts", []))
        gcov_unchecked = dict(item.get("gcov_unchecked_line_counts", []))
        branches = {}
        excluded_lines = {e["line"]: e["reason"] for e in item.get("exclusions", []) if "line" in e}
        excluded_branches = {(*e["region"], e["side"]): e["reason"]
                             for e in item.get("exclusions", []) if "region" in e}
        folded = {tuple(region) for region in item.get("folded_branches", [])}
        missed_branches = set()
        for line, column, end_line, end_column, true_hit, false_hit in item["branches"]:
            coord = (line, column, end_line, end_column)
            location = f'{line}:{column}–{end_line}:{end_column} '
            if coord in folded:
                branches.setdefault(line, []).append(location + "コンパイル時に確定（実行時分岐なし）")
                continue
            sides = []
            for side, label, hit in (("true", "T", true_hit), ("false", "F", false_hit)):
                reason = excluded_branches.get((*coord, side))
                sides.append(f"{label}:除外（{reason}）" if reason else f'{label}:{"✓" if hit else "×"}')
                if not hit and reason is None:
                    missed_branches.add(line)
            branches.setdefault(line, []).append(location + " ".join(sides))
        page = [header, f'<title>{relative}</title><p><a href="index.html">一覧</a></p><h1>{relative}</h1>',
                '<table><thead><tr><th scope="col">行カバレッジ</th><th scope="col">分岐カバレッジ</th>'
                '</tr></thead><tbody><tr>' + rate_cells(item["summary"]) + '</tr></tbody></table>']
        first_miss = min(missed_branches | {line for line, hit in lines.items() if not hit and line not in excluded_lines},
                         default=None)
        if first_miss is not None:
            page.append(f'<p><a href="#L{first_miss}">最初の未到達へ（{first_miss}行）</a></p>')
        page += ['<p>Tは条件が真、Fは偽です。</p>',
                 '<table class="source-table"><thead><tr><th scope="col">行番号</th><th scope="col">行の到達</th>'
                 '<th scope="col">分岐の到達</th><th scope="col">原本</th></tr></thead><tbody>']
        for line, text in enumerate(Path(item["filename"]).read_text(encoding="utf-8").splitlines(), 1):
            state = 'hit' if lines.get(line) else 'miss' if line in lines else ''
            hit = '到達' if lines.get(line) else '未到達' if line in lines else '—'
            if line in excluded_lines:
                state = ''
                hit = html.escape(f"除外（{excluded_lines[line]}）")
            elif line in gcov_lines:
                hit = f"到達（gcov {gcov_lines[line]:,}回）"
            elif line in gcov_unchecked:
                hit = f"到達（検査省略gcov {gcov_unchecked[line]:,}回）"
            conditions = '<br>'.join(html.escape(condition) for condition in branches.get(line, []))
            branch_state = ' class="miss"' if line in missed_branches else ''
            page.append(f'<tr class="{state}" id="L{line}"><td class="line-number"><a href="#L{line}">{line}</a></td>'
                        f'<td class="line-state">{hit}</td><td{branch_state}>{conditions}</td>'
                        f'<td><pre>{html.escape(text)}</pre></td></tr>')
        (directory / target).write_text(''.join(page) + '</tbody></table></html>\n', encoding="utf-8")
    index.extend(row for _, row in sorted(index_rows, key=lambda entry: entry[0]))
    (directory / "index.html").write_text(''.join(index) + '</tbody></table></html>\n', encoding="utf-8")


def summary_markdown(report: dict) -> str:
    title = "ASan + UBSan" if report["mode"] == "sanitizers" else "C++ 分岐カバレッジ"
    executed = sum(bool(row.get("samples")) for row in report["results"])
    local = sum(bool(row.get("samples")) and row["local_fixture"] for row in report["results"])
    major = report.get("tools", {}).get("compiler", {}).get("major")
    compiler = f"Clang {major}・" if type(major) is int else ""
    lines = ["### " + title, "", "状態: " + report["status"], "",
             f"checked実行: {executed}/{len(report['selected_ids'])} ID（うちローカル補足 {local}件）。",
             compiler + "通常版・C++23・assert有効。オンライン提出の判定は含みません。"]
    if report["mode"] == "coverage":
        gate = report.get("coverage_gate")
        lines += ["", "ビルド・入力照合・計測: " + report.get("execution_status", "running")]
        if gate:
            lines += ["100%必須判定（行・直接分岐）: " + gate["status"],
                      f"未到達: 行 {gate['lines']['uncovered']}、直接分岐 {gate['branches']['uncovered']}。",
                      f"計測TU: {gate['measured_translation_units']}/{gate['expected_translation_units']}、"
                      f"未計測の選択原本: {len(gate['unmapped_sources'])}。"]
            if not gate["nonzero_mapped_lines"]:
                lines += ["計測可能な原本の行がありません。空の計測は合格にしません。"]
            if gate["unmapped_sources"]:
                lines += ["未計測原本: " + ", ".join(inline_code(name) for name in gate["unmapped_sources"])]
        if "coverage_shard" in report:
            shard = report["coverage_shard"]
            lines += [f"分割 {shard['index'] + 1}/{shard['count']}。100%判定は全分割の計測を集約して行います。"]
    if "coverage_totals" in report:
        lines += ["", "| 計測対象 | カバレッジ率 | 到達 / 計測 | 未到達 |", "| --- | ---: | ---: | ---: |"]
        for key, name in (("lines", "行"), ("branches", "分岐")):
            metric = report["coverage_totals"][key]
            covered, count = metric["covered"], metric["count"]
            lines.append(f"| {name} | {coverage_rate(metric, key)} | {covered:,} / {count:,} | {count - covered:,} |")
        lines += ["", "原本別の一覧は `html/index.html`、計測データは `coverage.json` にあります。"]
        if "coverage_raw_totals" in report:
            raw = report["coverage_raw_totals"]
            adjusted = report["coverage_totals"]
            folded = report.get("coverage_folded_sides", 0)
            lines += ["", f"証明済みの除外: 行 {raw['lines']['count'] - adjusted['lines']['count']}、"
                      f"直接分岐 {raw['branches']['count'] - adjusted['branches']['count'] - folded}。",
                      f"コンパイル時に確定した条件: {folded // 2} 件。"]
    if report.get("error"):
        lines += ["", "計測エラー: " + inline_code(report["error"])]
    failed = [row["id"] for row in report["results"] if row.get("status") != "PASS"]
    if failed:
        remaining = f"（ほか {len(failed) - 8} 件は `results.json`）" if len(failed) > 8 else ""
        lines += ["", f"未成功ID（{len(failed)}件）: " + ", ".join(inline_code(name) for name in failed[:8]) + remaining]
    if "coverage_totals" in report:
        lines += ["", "計測は原本の行と、直接記述した条件の真・偽を対象とします。"
                  "同じ位置を型やTUをまたいで1回だけ数えます。"]
    lines += ["", "登録サンプルとローカル補足（独立参照を含む）で実行した範囲の計測です。"
              "未実体化template・未選択ライブラリは分母に含みません。"
              "Fast/Variants・vendor・driver/main・連結提出コードも集計対象外です。"]
    return "\n".join(lines) + "\n"


def write_report(output: Path, report: dict) -> None:
    temporary = output / "results.json.tmp"
    temporary.write_text(json.dumps(report, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    temporary.replace(output / "results.json")
    (output / "summary.md").write_text(summary_markdown(report), encoding="utf-8")


def coverage_command(command: list[str], log: Path, timeout: float) -> subprocess.CompletedProcess:
    result = subprocess.run(command, capture_output=True, text=True, errors="replace", timeout=timeout)
    log.write_text(result.stderr + ("\nstdout:\n" + result.stdout if result.returncode else ""), encoding="utf-8")
    result.check_returncode()
    if re.search(r"mismatched data|hash mismatch|profile data may be out of date", result.stderr, re.I):
        raise ValueError("LLVM profileが一致しません: " + str(log))
    return result


def run_gcov(entry: dict, *, gcc: str, gcov: str, source: Path, binary: Path, include_flags: list[str],
             logs: Path, commands: list, timeout: float, compile_timeout: float) -> dict:
    obj = binary.with_suffix(".o")
    for command, suffix in (
        ([gcc, "-std=c++23", "-O0", "-g", "--coverage", *include_flags, "-c", str(source), "-o", str(obj)], "compile"),
        ([gcc, "--coverage", str(obj), "-o", str(binary)], "link"),
    ):
        commands.append(command)
        coverage_command(command, logs / f"{binary.name}_{suffix}.txt", compile_timeout)
    for index, sample in enumerate(entry["samples"]):
        result = subprocess.run([str(binary)], input=sample["input"], capture_output=True, text=True,
                                errors="replace", timeout=timeout)
        (logs / f"{binary.name}_sample_{index}.txt").write_text(
            "stdout:\n" + result.stdout + "\nstderr:\n" + result.stderr, encoding="utf-8")
        if result.returncode or not output_matches(entry, sample, result.stdout):
            raise ValueError("GCC行計測でテストが失敗しました")
    profile = obj.with_suffix(".gcda")
    if not profile.is_file() or not profile.stat().st_size:
        raise ValueError("GCC行計測のprofileがありません")
    command = [gcov, "--json-format", "--stdout", str(obj.with_suffix(".gcno"))]
    commands.append(command)
    result = coverage_command(command, logs / f"{binary.name}_export.txt", compile_timeout)
    return json.loads(result.stdout)


def export_clang_coverage(binary: Path, profiles: list[Path], sources: list[Path], logs: Path,
                          profdata: str, llvm_cov: str, commands: list, timeout: float) -> dict:
    name = binary.name
    merged = binary.with_suffix(".profdata")
    command = [profdata, "merge", "-sparse", *(str(path) for path in profiles), "-o", str(merged)]
    commands.append(command)
    coverage_command(command, logs / f"{name}_merge.txt", timeout)
    common = [str(binary), "-instr-profile=" + str(merged)]
    paths = [str(path) for path in sources]
    command = [llvm_cov, "export", *common, "-skip-expansions", "--sources", *paths]
    commands.append(command)
    regions = coverage_command(command, logs / f"{name}_export.txt", timeout)
    command = [llvm_cov, "export", *common, "-skip-functions", "-format=lcov", "--sources", *paths]
    commands.append(command)
    lcov = coverage_command(command, logs / f"{name}_lcov.txt", timeout)
    return source_coverage(json.loads(regions.stdout), lcov.stdout, sources)


def run_clang_supplement(entry: dict, output: Path, binary: Path, command: list[str],
                         execution: list[str], sources: list[Path], llvm_profdata: str, llvm_cov: str,
                         commands: list, timeout: float, compile_timeout: float) -> tuple[dict, dict]:
    label = binary.name
    logs = output / "logs"
    metadata = {"command": command, "execution_command": execution, "samples": []}
    commands.append(command)
    coverage_command(command, logs / f"{label}_compile.txt", compile_timeout)
    profiles = []
    for index, sample in enumerate(entry["samples"]):
        profile_prefix = f"{label}_{index}"
        pattern = str(binary.parent / (profile_prefix + "-%p.profraw"))
        result = subprocess.run(execution, input=sample["input"], capture_output=True,
                                text=True, errors="replace", env=runtime_environment("coverage", pattern),
                                timeout=timeout)
        log = f"logs/{profile_prefix}.txt"
        (output / log).write_text("stdout:\n" + result.stdout + "\nstderr:\n" + result.stderr, encoding="utf-8")
        generated = sorted(binary.parent.glob(profile_prefix + "-*.profraw"))
        written = bool(generated) and all(path.stat().st_size for path in generated)
        passed = result.returncode == 0 and written and output_matches(entry, sample, result.stdout)
        if not passed:
            raise ValueError("Clang補足計測でテストが失敗しました: " + entry["id"])
        metadata["samples"].append({"index": index, "returncode": result.returncode, "passed": passed,
                                    "profile_written": written, "log": log})
        profiles.extend(generated)
    measured = export_clang_coverage(binary, profiles, sources, logs, llvm_profdata, llvm_cov,
                                     commands, compile_timeout)
    for path in [binary, binary.with_suffix(".profdata"), *profiles]:
        path.unlink()
    return measured, metadata


def write_coverage(report: dict, exports: list[dict], root: Path, output: Path,
                   shared: list[list[tuple[str, int, int]]], exclusions: dict) -> dict:
    measured = coverage_union(exports, shared, direct_exclusions=direct_exclusion_coordinates(root, exclusions))
    coverage = apply_coverage_exclusions(measured, root, report["library_source_hashes"], exclusions)
    report["coverage_raw_totals"] = coverage["raw_totals"]
    report["coverage_folded_sides"] = 2 * sum(len(item.get("folded_branches", [])) for item in coverage["files"])
    report["coverage_totals"] = coverage["totals"]
    report["coverage_definition"] = coverage["definition"]
    report["coverage_complete"] = len(exports) == len(report["selected_ids"])
    (output / "coverage.json").write_text(json.dumps(coverage) + "\n", encoding="utf-8")
    write_coverage_html(coverage, root, output / "html")
    return coverage


def run(entries: list[dict], *, root: Path, output: Path, mode: str, compiler: str,
        llvm_profdata: str = "llvm-profdata", llvm_cov: str = "llvm-cov",
        gcc_install_dir: str | None = None, gcc: str = "g++", gcov: str = "gcov",
        qemu_x86: str = "qemu-x86_64", qemu_i386: str = "qemu-i386",
        x86_baseline_sysroot: Path | None = None,
        i386_compiler_args: list[str] | None = None, i386_sysroot: str | None = None, timeout: float = 120,
        compile_timeout: float = 180, available: list[dict] | None = None, jobs: int = 1,
        coverage_shard: tuple[int, int] | None = None) -> dict:
    root, output = root.resolve(), output.resolve()
    output.mkdir(parents=True, exist_ok=False)
    report = {"format": 1, "mode": mode, "status": "running", "execution_status": "running", "checked": True,
              "jobs": jobs,
              "selected_ids": [entry["id"] for entry in entries], "results": [], "commands": [],
              "available_entries": len(available if available is not None else entries),
              "available_local_fixtures": sum(not entry.get("problem_url")
                                              for entry in (available if available is not None else entries)),
              "online_status": "not_evaluated", "artifacts_exclude_binaries_and_raw_profiles": True}
    allowed = []
    coverage_exports = []
    coverage = coverage_union([])
    if coverage_shard is not None:
        shard_index, shard_count = coverage_shard
        report["coverage_shard"] = {"index": shard_index, "count": shard_count}
    if mode == "coverage" and coverage_shard is None:
        report["coverage_gate"] = coverage_gate(coverage, allowed, expected_translation_units=len(entries),
                                                measured_translation_units=0)
    if mode == "sanitizers":
        report["runtime_options"] = SANITIZER_ENV
    write_report(output, report)
    try:
        if not entries:
            raise ValueError("計測対象がありません")
        allowed = library_paths(entries, root)
        version, major = command_version(compiler, clang=True)
        report["tools"] = {"compiler": {"command": compiler, "version": version, "major": major}}
        baseline_command = [qemu_x86, "-cpu", X86_BASELINE_CPU]
        baseline_compiler_options = []
        if mode == "coverage":
            for name, tool in [("llvm_profdata", llvm_profdata), ("llvm_cov", llvm_cov)]:
                version, tool_major = command_version(tool)
                report["tools"][name] = {"command": tool, "version": version, "major": tool_major}
                if tool_major != major:
                    raise ValueError("Clang/llvm-profdata/llvm-covは同じmajor版が必要です")
            if any(entry.get("x86_baseline_coverage") for entry in entries):
                result = subprocess.run([qemu_x86, "--version"], check=True, capture_output=True, text=True, timeout=30)
                report["tools"]["qemu_x86"] = {"command": qemu_x86, "version": result.stdout.strip(),
                                                "cpu": X86_BASELINE_CPU}
                if x86_baseline_sysroot is not None:
                    sysroot = x86_baseline_sysroot.resolve()
                    libraries = sysroot / "usr/lib/x86_64-linux-gnu"
                    resource_dir = sysroot / f"usr/lib/llvm-{major}/lib/clang/{major}"
                    loader = libraries / "ld-linux-x86-64.so.2"
                    required = [loader, libraries / "Scrt1.o", resource_dir / "include/stddef.h",
                                resource_dir / "lib/linux/libclang_rt.profile-x86_64.a",
                                *(libraries / name for name in ("libc.so.6", "libm.so.6", "libstdc++.so.6", "libgcc_s.so.1"))]
                    for path in required:
                        if not path.is_file():
                            raise ValueError("QEMUの汎用AMD64ビルド環境が不足しています: " + str(path))
                    # 実行ファイルにAMD64v3の要件を引き継がないよう、起動用オブジェクトも分離する。
                    baseline_compiler_options = [f"--sysroot={sysroot}", f"--gcc-toolchain={sysroot / 'usr'}",
                                                 f"-resource-dir={resource_dir}", "-march=x86-64"]
                    baseline_command += [str(loader), "--inhibit-cache", "--library-path", str(libraries)]
                    report["tools"]["qemu_x86"]["command_prefix"] = baseline_command
            if any(entry.get("x86_avx2_coverage") for entry in entries):
                result = subprocess.run([qemu_x86, "--version"], check=True, capture_output=True, text=True, timeout=30)
                report["tools"]["qemu_x86_avx2"] = {"command": qemu_x86, "version": result.stdout.strip(),
                                                     "cpu": X86_AVX2_CPU}
            if any(entry.get("coverage_target") == "i386" for entry in entries):
                result = subprocess.run([qemu_i386, "--version"], check=True, capture_output=True, text=True, timeout=30)
                report["tools"]["qemu_i386"] = {"command": qemu_i386, "version": result.stdout.strip()}
        elif mode != "sanitizers":
            raise ValueError("不明な計測modeです")
        compiler_options = gcc_install_arguments(compiler, gcc_install_dir)
        report["library_source_hashes"] = {str(path.relative_to(root)): digest(path) for path in allowed}
        watched = set(allowed) | {(root / entry["driver"]).resolve() for entry in entries}
        proof_policy = root / COVERAGE_EXCLUSIONS
        shared = shared_coverage_blocks(root) if mode == "coverage" else []
        exclusions = {}
        if mode == "coverage" and proof_policy.is_file():
            watched.add(proof_policy)
            exclusions = load_coverage_exclusions(root, shared)
            watched.update(root / row["shared_from"] for name in report["library_source_hashes"]
                           for row in exclusions.get(name, {}).get("regions", []) if "shared_from" in row)
        if any(entry.get("coverage_test") for entry in entries):
            watched.add(root / COVERAGE_TESTS)
            watched.update((root / name).resolve()
                           for entry in entries if entry.get("coverage_test")
                           for name in entry.get("support_source_hashes", {}))
        pending = list(watched)
        while pending:
            path = pending.pop()
            for match in prepare_verify.COMMON_INCLUDE.finditer(path.read_text(encoding="utf-8")):
                dependency = (root / match[1]).resolve(strict=True)
                if not dependency.is_relative_to(root):
                    raise ValueError("共通ヘッダーがリポジトリ外を参照しています")
                if dependency not in watched:
                    watched.add(dependency)
                    pending.append(dependency)
        report["all_source_hashes"] = {str(path.relative_to(root)): digest(path) for path in sorted(watched)}
        report["vendor_source_hashes"] = {str(path.relative_to(root)): digest(path)
                                           for path in sorted((root / "Verify/current/vendor").rglob("*"))
                                           if path.is_file()}
        (output / "sources").mkdir()
        (output / "logs").mkdir()
        with tempfile.TemporaryDirectory(prefix="sirkit_instrumented_") as temporary:
            work = Path(temporary)
            overlay = work / "include"
            make_acl_overlay(root, overlay)
            if mode == "coverage":
                (output / "coverage_entries").mkdir()
            include_flags = ["-UNDEBUG", "-USKIP_RUNTIME_CHECKS", "-I", str(overlay), "-I", str(root),
                             "-I", str(root / "Verify/current/vendor")]
            def measure(index: int, entry: dict) -> tuple[dict, list, dict | None]:
                if coverage_shard is not None:
                    index = index * shard_count + shard_index
                label = f"entry_{index:04}"
                source = output / "sources" / (label + ".cpp")
                source.write_text(translation_unit(entry, root), encoding="utf-8")
                binary = work / label
                profiles = []
                merged = work / (label + ".profdata")
                commands = []
                measured = None
                flags = (["-O1", "-g", "-fsanitize=address,undefined", "-fno-sanitize-recover=all",
                          "-fno-omit-frame-pointer"] if mode == "sanitizers" else
                         ["-O0", "-g", "-fprofile-instr-generate", "-fcoverage-mapping"])
                i386 = mode == "coverage" and entry.get("coverage_target") == "i386"
                x86_baseline = mode == "coverage" and entry.get("x86_baseline_coverage")
                options = [] if i386 and i386_compiler_args else compiler_options
                if x86_baseline and baseline_compiler_options:
                    options = baseline_compiler_options
                target_flags = [*(i386_compiler_args or []), "-m32"] if i386 else []
                command = [compiler, *options, "-std=c++23", *flags, str(source),
                           *target_flags, *include_flags, "-o", str(binary)]
                run_command = [str(binary)]
                if i386:
                    run_command = [qemu_i386, *(["-L", i386_sysroot] if i386_sysroot else []), str(binary)]
                row = {"id": entry["id"], "local_fixture": not entry.get("problem_url"),
                       "status": "compiling", "command": command, "source_sha256": digest(source),
                       "driver": entry["driver"], "driver_sha256": digest(root / entry["driver"]),
                       "samples": []}
                if entry.get("coverage_test"):
                    row["coverage_test"] = entry["coverage_test"]
                if i386:
                    row["coverage_target"] = "i386"
                    row["execution_command"] = run_command
                try:
                    built = subprocess.run(command, capture_output=True, text=True, errors="replace",
                                           timeout=compile_timeout)
                    row["compile_returncode"] = built.returncode
                    (output / "logs" / (label + "_compile.txt")).write_text(built.stdout + built.stderr,
                                                                             encoding="utf-8")
                    row["compile_log"] = "logs/" + label + "_compile.txt"
                    if built.returncode:
                        row["status"] = "FAIL"
                        return row, commands, None
                    for sample_index, sample in enumerate(entry.get("samples", [])):
                        pattern = str(work / f"{label}_{sample_index}-%p.profraw")
                        environment = runtime_environment(mode, pattern)
                        result = subprocess.run(run_command, input=sample["input"], capture_output=True,
                                                text=True, errors="replace", env=environment, timeout=timeout)
                        log = f"logs/{label}_sample_{sample_index}.txt"
                        (output / log).write_text("stdout:\n" + result.stdout + "\nstderr:\n" + result.stderr,
                                                  encoding="utf-8")
                        sample_row = {"index": sample_index, "returncode": result.returncode,
                                      "passed": result.returncode == 0 and output_matches(entry, sample, result.stdout)}
                        sample_row["log"] = log
                        if mode == "coverage":
                            generated = list(work.glob(f"{label}_{sample_index}-*.profraw"))
                            sample_row["profile_written"] = bool(generated) and all(path.stat().st_size for path in generated)
                            sample_row["passed"] &= sample_row["profile_written"]
                            profiles.extend(generated)
                            if sample_row["passed"] and x86_baseline:
                                name = f"{label}_{sample_index}_x86"
                                command = [*baseline_command, str(binary)]
                                commands.append(command)
                                baseline = subprocess.run(command, input=sample["input"], capture_output=True, text=True,
                                                          errors="replace", timeout=timeout,
                                                          env=runtime_environment(mode, str(work / (name + "-%p.profraw"))))
                                log = "logs/" + name + ".txt"
                                (output / log).write_text("stdout:\n" + baseline.stdout + "\nstderr:\n" + baseline.stderr,
                                                          encoding="utf-8")
                                generated = list(work.glob(name + "-*.profraw"))
                                written = bool(generated) and all(path.stat().st_size for path in generated)
                                passed = baseline.returncode == 0 and output_matches(entry, sample, baseline.stdout) and written
                                sample_row["x86_baseline"] = {"returncode": baseline.returncode, "profile_written": written,
                                                              "passed": passed, "log": log}
                                sample_row["passed"] &= passed
                                profiles.extend(generated)
                        row["samples"].append(sample_row)
                    row["status"] = "PASS" if row["samples"] and all(item["passed"] for item in row["samples"]) else "FAIL"
                    if mode == "coverage" and row["status"] == "PASS":
                        entry_sources = library_paths([entry], root)
                        measured = export_clang_coverage(binary, profiles, entry_sources, output / "logs",
                                                         llvm_profdata, llvm_cov, commands, compile_timeout)
                        if entry.get("clang_unchecked_coverage"):
                            alternate_binary = work / (label + "_clang_unchecked")
                            alternate_source = source.with_name(alternate_binary.name + ".cpp")
                            alternate_source.write_text(translation_unit(entry, root, unchecked=True), encoding="utf-8")
                            alternate_command = [compiler, *options, "-std=c++23", *flags, str(alternate_source),
                                                 *include_flags, "-DSKIP_RUNTIME_CHECKS", "-o", str(alternate_binary)]
                            alternate, row["clang_unchecked_coverage"] = run_clang_supplement(
                                entry, output, alternate_binary, alternate_command, [str(alternate_binary)], entry_sources,
                                llvm_profdata, llvm_cov, commands, timeout, compile_timeout)
                            measured = coverage_union([measured, alternate])
                        if entry.get("x86_avx2_coverage"):
                            alternate_binary = work / (label + "_x86_avx2")
                            alternate_command = [compiler, *options, "-std=c++23", *flags, str(source),
                                                 *include_flags, "-mavx2", "-o", str(alternate_binary)]
                            execution = [qemu_x86, "-cpu", X86_AVX2_CPU, str(alternate_binary)]
                            alternate, row["x86_avx2_coverage"] = run_clang_supplement(
                                entry, output, alternate_binary, alternate_command, execution, entry_sources,
                                llvm_profdata, llvm_cov, commands, timeout, compile_timeout)
                            measured = coverage_union([measured, alternate])
                        if entry.get("x86_binary128_coverage"):
                            alternate_binary = work / (label + "_x86_binary128")
                            alternate_command = [compiler, *options, "-std=c++23", *flags, str(source),
                                                 *include_flags, "-mlong-double-128", "-o", str(alternate_binary)]
                            alternate, row["x86_binary128_coverage"] = run_clang_supplement(
                                entry, output, alternate_binary, alternate_command, [str(alternate_binary)], entry_sources,
                                llvm_profdata, llvm_cov, commands, timeout, compile_timeout)
                            measured = coverage_union([measured, alternate])
                        for unchecked, setting in ((False, "gcc_line_coverage"), (True, "gcc_unchecked_line_coverage")):
                            if not entry.get(setting):
                                continue
                            gcc_flags = include_flags
                            gcc_source = source
                            suffix = "_gcc"
                            if unchecked:
                                suffix = "_gcc_unchecked"
                                gcc_flags = [*include_flags, "-DSKIP_RUNTIME_CHECKS"]
                                gcc_source = source.with_name(label + suffix + ".cpp")
                                gcc_source.write_text(translation_unit(entry, root, unchecked=True), encoding="utf-8")
                            gcc_report = run_gcov(entry, gcc=gcc, gcov=gcov, source=gcc_source,
                                                  binary=work / (label + suffix), include_flags=gcc_flags,
                                                  logs=output / "logs", commands=commands,
                                                  timeout=timeout, compile_timeout=compile_timeout)
                            record_gcov_lines(measured, gcc_report, unchecked=unchecked)
                            row[setting] = {"version": gcc_report["gcc_version"]}
                        if entry.get("coverage_test"):
                            mapped = {item["filename"] for item in measured["files"] if item["lines"]}
                            if not mapped:
                                raise ValueError("補足テストに対象原本の計測結果がありません")
                        row["coverage_path"] = "coverage_entries/" + label + ".json"
                        (output / row["coverage_path"]).write_text(json.dumps(measured) + "\n", encoding="utf-8")
                except (OSError, ValueError, subprocess.SubprocessError) as error:
                    row.update(status="FAIL", error=str(error))
                finally:
                    for path in [binary, merged, *profiles]:
                        path.unlink(missing_ok=True)
                return row, commands, measured if row["status"] == "PASS" else None

            with ThreadPoolExecutor(max_workers=jobs) as pool:
                # 出力順は登録順にそろえ、レポートと全体の集計は親スレッドだけで更新する。
                for index, (row, commands, measured) in enumerate(pool.map(measure, range(len(entries)), entries), 1):
                    report["results"].append(row)
                    report["commands"].extend(commands)
                    if measured is not None:
                        coverage_exports.append(measured)
                    write_report(output, report)
                    print(f"{index}/{len(entries)} {row['id']}: {row['status']}", flush=True)
            if mode == "coverage":
                if coverage_shard is None:
                    coverage = write_coverage(report, coverage_exports, root, output, shared, exclusions)
                else:
                    report["coverage_complete"] = len(coverage_exports) == len(entries)
                    if not report["coverage_complete"]:
                        raise ValueError("この分割に未計測のテストがあります")
        report["source_hashes_after"] = {str(path.relative_to(root)): digest(path) for path in sorted(watched)}
        if report["source_hashes_after"] != report["all_source_hashes"]:
            raise ValueError("計測中に原本またはdriver/共通ヘッダーが変更されました")
        report["execution_status"] = "PASS" if len(report["results"]) == len(entries) and all(row["status"] == "PASS" for row in report["results"]) else "FAIL"
        report["status"] = report["execution_status"]
    except (OSError, ValueError, RuntimeError, subprocess.SubprocessError) as error:
        report.update(status="FAIL", execution_status="FAIL", error=str(error))
    finally:
        if mode == "coverage" and coverage_shard is None:
            report["coverage_gate"] = coverage_gate(coverage, allowed, expected_translation_units=len(entries),
                                                    measured_translation_units=len(coverage_exports))
            report["unmapped_sources"] = report["coverage_gate"]["unmapped_sources"]
            if report["coverage_gate"]["status"] != "PASS":
                report["status"] = "FAIL"
        if report["status"] == "running":
            report["status"] = "FAIL"
        if report["execution_status"] == "running":
            report["execution_status"] = "FAIL"
        write_report(output, report)
    return report


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--mode", choices=("sanitizers", "coverage"), required=True)
    parser.add_argument("--compiler", default="clang++")
    parser.add_argument("--llvm-profdata", default="llvm-profdata")
    parser.add_argument("--llvm-cov", default="llvm-cov")
    parser.add_argument("--gcc-install-dir")
    parser.add_argument("--gcc", default="g++", help="補足行計測用のGCC")
    parser.add_argument("--gcov", default="gcov", help="補足行計測用のgcov")
    parser.add_argument("--qemu-x86", default="qemu-x86_64", help="AVX2・POPCNTなしの実行時経路を測定するQEMU")
    parser.add_argument("--x86-baseline-sysroot", type=Path, help="QEMU補足テストのビルド・実行に使う汎用AMD64ルート")
    parser.add_argument("--qemu-i386", default="qemu-i386", help="i386カバレッジ用のQEMU")
    parser.add_argument("--i386-compiler-arg", action="append", help="別のi386クロス環境を使う場合のClang引数")
    parser.add_argument("--i386-sysroot", help="別のi386クロス環境を使う場合のQEMUのライブラリルート")
    parser.add_argument("--id", action="append", help="対象ID。省略時はローカル補足を含む全件")
    parser.add_argument("--shard-index", type=int, default=0)
    parser.add_argument("--shard-count", type=int, default=1)
    parser.add_argument("--output", type=Path, required=True, help="未作成の出力ディレクトリ")
    parser.add_argument("--timeout", type=float, default=120)
    parser.add_argument("--compile-timeout", type=float, default=180)
    parser.add_argument("--jobs", type=int, default=1, help="同時にビルド・実行するテストの数")
    parser.add_argument("--cpu", type=int)
    args = parser.parse_args(argv)
    try:
        if args.timeout <= 0 or args.compile_timeout <= 0:
            raise ValueError("timeoutは正数で指定してください")
        if args.jobs < 1:
            raise ValueError("jobsは正の整数で指定してください")
        if args.cpu is not None:
            os.sched_setaffinity(0, {args.cpu})
        available = prepare_verify.collect()
        if args.mode == "coverage":
            available += coverage_tests(ROOT)
        entries = select_entries(available, args.id)
        if not 0 <= args.shard_index < args.shard_count <= len(entries):
            raise ValueError("分割の番号または分割数が不正です")
        coverage_shard = None
        if args.shard_count > 1:
            if args.mode != "coverage" or args.id:
                raise ValueError("分割できるのは全件カバレッジ計測です")
            coverage_shard = (args.shard_index, args.shard_count)
            entries = entries[args.shard_index::args.shard_count]
        report = run(entries, root=ROOT, output=args.output, mode=args.mode, compiler=args.compiler,
                     llvm_profdata=args.llvm_profdata, llvm_cov=args.llvm_cov,
                     gcc_install_dir=args.gcc_install_dir, gcc=args.gcc, gcov=args.gcov,
                     qemu_x86=args.qemu_x86, qemu_i386=args.qemu_i386, x86_baseline_sysroot=args.x86_baseline_sysroot,
                     i386_compiler_args=args.i386_compiler_arg,
                     i386_sysroot=args.i386_sysroot, timeout=args.timeout,
                     compile_timeout=args.compile_timeout, available=available, jobs=args.jobs,
                     coverage_shard=coverage_shard)
        return int(report["status"] != "PASS")
    except (OSError, ValueError, subprocess.SubprocessError) as error:
        print(str(error), file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
