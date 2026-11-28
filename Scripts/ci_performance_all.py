#!/usr/bin/env python3
# SPDX-License-Identifier: CC0-1.0
"""全通常ライブラリのVerifyプログラムを計測し、基準commitからの退行を検査する。"""
from __future__ import annotations

import argparse
from collections import Counter
from contextlib import contextmanager, nullcontext
from functools import partial
import hashlib
import importlib
import json
import math
import platform
import os
from pathlib import Path
import re
import resource
import shutil
import signal
import statistics
import subprocess
import sys
import time
import threading

sys.dont_write_bytecode = True
try:
    from . import ci_benchmark, ci_compile_profiles, ci_instrumented, ci_library_checker as official
    from . import prepare_verify, performance_limits, performance_regression
except ImportError:
    import ci_benchmark
    import ci_compile_profiles
    import ci_instrumented
    import ci_library_checker as official
    import prepare_verify
    import performance_limits
    import performance_regression

ROOT = Path(__file__).resolve().parents[1]
FLAGS = ["-std=c++23", "-O2", "-DNDEBUG"]
SCOPE = "whole_verify_program_fixed_main_io"
IDENTIFIER = re.compile(r"[A-Za-z0-9_][A-Za-z0-9_.-]*\Z")
SPECIAL_GROUP = re.compile(r"conchon|intersect|hand|overflow|killer|tle|anti|chain|star|sparse|dense|monotone|max", re.I)
BASE_PATH_ALIASES = {"Geometry/angle_sort.cpp": "Geometry/偏角ソート.cpp"}


class PerformanceError(ValueError):
    pass


def digest(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def file_digest(path: Path) -> str:
    with path.open("rb") as stream:
        return hashlib.file_digest(stream, "sha256").hexdigest()


def nonlc_module():
    if str(ROOT) not in sys.path:
        sys.path.insert(0, str(ROOT))
    return importlib.import_module("Verify.ci.performance_cases")


def catalog(entries: list[dict], nonlc, root: Path = ROOT) -> list[dict]:
    """一度もAPIを呼ばない無関係なfixtureは使わず、登録済みのdriverを選ぶ。"""
    paths = [path.relative_to(root.resolve()).as_posix()
             for path in ci_instrumented.library_paths(entries, root)]
    by_id = {entry["id"]: entry for entry in entries}
    if len(by_id) != len(entries):
        raise PerformanceError("duplicate Verify ID")
    mapping = nonlc.LIBRARY_VERIFY_IDS
    result = []
    for library in paths:
        matches = [entry for entry in entries if library in entry["libraries"]]
        lc = [entry for entry in matches if official.problem_id(entry)]
        if lc:
            entry = min(lc, key=lambda e: (len(e["libraries"]) != 1, e["id"].startswith("coverage_"), e["id"]))
            source = "official_library_checker"
            cases = None
        else:
            identifier = mapping.get(library)
            entry = by_id.get(identifier)
            if entry not in matches or not (entry.get("problem_url") or entry.get("verification_kind") == "local_regression"):
                raise PerformanceError("missing non-LC performance fixture: " + library)
            source = "deterministic_non_lc"
            cases = nonlc.plans_for_library(library, quick=False)
            if not cases or any(c.get("verify_id") != identifier for c in cases):
                raise PerformanceError("non-LC case catalog mismatch: " + library)
        if not IDENTIFIER.fullmatch(entry["id"]):
            raise PerformanceError("invalid Verify ID")
        result.append({"library": library, "verify_id": entry["id"],
                       "libraries": list(entry["libraries"]), "case_source": source,
                       "problem_id": official.problem_id(entry), "problem_url": entry.get("problem_url"),
                       "combined_library_scope": len(entry["libraries"]) != 1,
                       "case_selection": "smallest + largest + every generator family + dedicated first cases" if lc else "all configured deterministic profiles",
                       "configured_cases": cases})
    if len(result) != len(paths) or {row["library"] for row in result} != set(paths):
        raise PerformanceError("library coverage incomplete")
    return sorted(result, key=lambda row: row["library"])


def select_catalog(rows: list[dict], requested: list[str] | None, index: int, count: int) -> list[dict]:
    if count < 1 or not 0 <= index < count:
        raise PerformanceError("invalid shard")
    if requested and (len(requested) != len(set(requested)) or set(requested) - {r["library"] for r in rows}):
        raise PerformanceError("unknown or duplicate requested library")
    chosen = [row for row in rows if not requested or row["library"] in requested]
    return chosen[index::count]


def official_selection(sizes: dict[str, int], quick: bool) -> dict[str, list[str]]:
    names = list(sizes)
    if not names or len(names) != len(set(names)):
        raise PerformanceError("empty or duplicate official cases")
    selected: dict[str, list[str]] = {}
    def add(name: str, reason: str):
        selected.setdefault(name, []).append(reason)
    examples = [name for name in names if name.startswith("example_")]
    add(min(examples or names, key=lambda n: (sizes[n], n)), "small_example")
    if not quick:
        add(max(names, key=lambda n: (sizes[n], n)), "largest_official_input_bytes")
        groups: dict[str, list[str]] = {}
        for name in names:
            group = re.sub(r"_\d+\Z", "", name)
            groups.setdefault(group, []).append(name)
        for group, members in sorted(groups.items()):
            add(max(members, key=lambda n: (sizes[n], n)), "generator_family_largest:" + group)
            if SPECIAL_GROUP.search(group):
                add(min(members), "dedicated_group_first:" + group)
    return selected


def select_official_cases(directory: Path, details: dict, quick: bool, *,
                          sizes: dict[str, int] | None = None) -> list[dict]:
    names = details["expected_cases"]
    if not names or len(names) != len(set(names)):
        raise PerformanceError("empty or duplicate official cases")
    if sizes is None:
        sizes = {name: (directory / "in" / (name + ".in")).stat().st_size for name in names}
    streaming = details.get("generation_strategy") == "one_case_at_a_time"
    hashes = official.official_hashes(directory, names, manifest_sha256=details["hash_json_sha256"]) if streaming else {}
    cases = []
    for name, reasons in official_selection(sizes, quick).items():
        source = directory / "in" / (name + ".in")
        expected = directory / "out" / (name + ".out")
        metadata = {"selection_reasons": reasons,
                    "scale_profile": "quick_example" if quick else "official_representative",
                    "maximum_claim": "Input byte size and generator names do not prove every parameter reaches its maximum.",
                    "official_revision": details["revision"], "problem_id": details["problem_id"]}
        if not streaming:
            metadata["input_header"] = input_header(source)
        cases.append({"name": name, "input_path": str(source), "expected_path": str(expected),
                      "checker": "official", "checker_path": str(directory / "checker"),
                      "checker_sha256": details["checker_sha256"], "input_bytes": sizes[name],
                      "input_sha256": hashes[source.name] if streaming else file_digest(source),
                      "expected_sha256": hashes[expected.name] if streaming else file_digest(expected),
                      "metadata": metadata})
    return cases


def official_input_sizes(checkout: Path, directory: Path, details: dict,
                         cache: Path, timeout: float) -> dict[str, int]:
    sizes = {}
    for name in details["expected_cases"]:
        with official.case_files(checkout, directory, details, name, cache, timeout, input_only=True):
            sizes[name] = (directory / "in" / (name + ".in")).stat().st_size
    return sizes


@contextmanager
def staged_official_case(case: dict, checkout: Path, directory: Path, details: dict,
                         cache: Path, timeout: float):
    with official.case_files(checkout, directory, details, case["name"], cache, timeout):
        case["metadata"]["input_header"] = input_header(Path(case["input_path"]))
        yield case


def input_header(path: Path) -> dict:
    # N/Q等の呼び名を推測せず、固定入力の先頭を補助的に記録する。
    with path.open("rb") as stream:
        data = stream.readline(161)
    return {"first_line_prefix": data[:160].decode("utf-8", errors="replace").rstrip("\r\n"),
            "truncated": len(data) > 160}


@contextmanager
def repository(root: Path):
    before = prepare_verify.ROOT, prepare_verify.BASE
    prepare_verify.ROOT, prepare_verify.BASE = root, root / "Verify/current"
    try:
        yield
    finally:
        prepare_verify.ROOT, prepare_verify.BASE = before


def render_program(entry: dict, libraries: dict[str, bytes], driver: str, root: Path = ROOT,
                   *, legacy_adapter: bool = False) -> tuple[bytes, dict]:
    aliases = []
    if legacy_adapter:
        code = "\n".join(prepare_verify.CXX_NON_CODE.sub(" ", data.decode()) for data in libraries.values())
        driver_code = prepare_verify.CXX_NON_CODE.sub(" ", driver)
        unwrapped = not re.search(r"\bnamespace\s+sirkit\s*(?:::|\{)", code)
        if unwrapped and re.search(r"\bnamespace\s+geo\s*\{", code):
            aliases.append("namespace sirkit { namespace geo = ::geo; }\n")
        if (unwrapped and re.search(r"\bkInf\b", driver_code) and not re.search(r"\bkInf\b", code)
                and re.search(r"\binline\s+constexpr\s+std::int64_t\s+INF\s*=", code)):
            aliases.append("namespace sirkit { static_assert(::INF == (std::int64_t{1} << 60)); "
                           "inline constexpr std::int64_t kInf = ::INF; }\n")
        if (unwrapped and "sirkit::circle_intersection_kind" in driver_code
                and re.search(r"\binline\s+geo::CircleIntersectionKind\s+circle_intersection_kind\s*\(", code)):
            aliases.append("namespace sirkit { using ::circle_intersection_kind; }\n")
        if (unwrapped and "sirkit::detail::InternalHashMap" in driver_code
                and re.search(r"\bnamespace\s+reinsirk::detail\s*\{\s*template[^;]+\bclass\s+InternalHashMap\b", code)):
            aliases.append("namespace sirkit::detail { using ::reinsirk::detail::InternalHashMap; }\n")
    adapter = "".join(aliases)
    with repository(root):
        source, _ = prepare_verify.render(entry, skip_runtime_checks=True,
                    source_overrides=libraries, driver_override=adapter + driver)
    return source, {"kind": "legacy_cpp_api_aliases" if adapter else "none",
                    "source": adapter, "sha256": digest(adapter.encode())}


def baseline_libraries(revision: str, names: list[str], root: Path = ROOT) -> tuple[dict[str, bytes] | None, dict]:
    contents, paths, missing = {}, {}, []
    for name in names:
        actual = name
        value = ci_benchmark.baseline_source(revision, actual, root)
        if value is None and name in BASE_PATH_ALIASES:
            actual = BASE_PATH_ALIASES[name]
            value = ci_benchmark.baseline_source(revision, actual, root)
        if value is None:
            missing.append(name)
        else:
            contents[name], paths[name] = value, actual
    proof = {"resolved_paths": paths, "source_hashes": {name: digest(data) for name, data in contents.items()}}
    if missing:
        return None, {**proof, "missing_path": missing[0], "missing_paths": missing}
    return contents, proof


def archived_driver(revision: str, entry: dict, current_sha: str, support: dict[str, str],
                    root: Path = ROOT) -> tuple[str, dict] | None:
    """同じ問題・入出力契約が確認できる旧driverだけを使う。"""
    manifest_path = "Verify/current/manifest.json"
    manifest = ci_benchmark.baseline_source(revision, manifest_path, root)
    if manifest is None:
        return None
    records = [row for row in json.loads(manifest)["entries"] if row.get("id") == entry["id"]]
    if len(records) != 1:
        return None
    record = records[0]
    path = entry["driver"]
    if (record.get("driver") != path or record.get("problem_url") != entry.get("problem_url")
            or record.get("libraries") != entry["libraries"]
            or record.get("sample_checker") != entry.get("sample_checker")
            or record.get("support_source_hashes", {}) != support
            or [(sample["input"], sample["output"]) for sample in record.get("samples", [])]
            != [(sample["input"], sample["output"]) for sample in entry.get("samples", [])]):
        return None
    previous = ci_benchmark.baseline_source(revision, path, root)
    if previous is None or digest(previous) != record.get("driver_sha256") or digest(previous) == current_sha:
        return None
    driver = previous.decode()
    with repository(root):
        if prepare_verify.common_source_hashes(driver) != support:
            return None
    return driver, {"kind": "archived_same_problem_and_io", "path": path, "sha256": digest(previous),
                    "manifest_sha256": digest(manifest), "same_as_current": False}


def freeze_sources(rows: list[dict], entries: dict, root: Path = ROOT) -> dict[str, str]:
    names = {"Scripts/ci_performance_all.py", "Scripts/performance_limits.py", "Scripts/performance_regression.py",
             "Verify/ci/performance_cases.py", "Scripts/prepare_verify.py",
             "Scripts/ci_library_checker.py", "Scripts/generate_official_case.py", "Scripts/ci_benchmark.py",
             "Scripts/ci_compile_profiles.py", "Verify/ci/library_checker.json"}
    names.update(Path(module.__file__).relative_to(ROOT).as_posix() for module in nonlc_module()._EXPANSIONS)
    for row in rows:
        entry = entries[row["verify_id"]]
        names.update(entry["libraries"])
        names.add(entry["driver"])
        names.update(prepare_verify.common_source_hashes((root / entry["driver"]).read_text()))
    return {name: file_digest(root / name) for name in sorted(names)}


def check_frozen(frozen: dict[str, str], root: Path = ROOT) -> None:
    if any(file_digest(root / name) != sha for name, sha in frozen.items()):
        raise PerformanceError("source changed during performance measurement")


def _limits(memory_mib: int, output_bytes: int, stack_bytes: int | None = None):
    def apply():
        memory = memory_mib * 1024 * 1024
        resource.setrlimit(resource.RLIMIT_AS, (memory, memory))
        resource.setrlimit(resource.RLIMIT_FSIZE, (output_bytes, output_bytes))
        resource.setrlimit(resource.RLIMIT_CORE, (0, 0))
        if stack_bytes is not None:
            resource.setrlimit(resource.RLIMIT_STACK, official.generation_stack_limit(stack_bytes))
    return apply


def execute(command: list[str], *, work: Path, output: Path, log: Path, timeout: float,
            stdin: Path | None = None, memory_mib: int = 2048, stack_bytes: int | None = None) -> dict:
    # wait(timeout)の指数的pollingを使わず、wait4で子のCPU時間も分離する。
    started = time.perf_counter_ns()
    done, timed_out = threading.Event(), threading.Event()
    with (stdin.open("rb") if stdin else open(os.devnull, "rb")) as inp, output.open("wb") as out, log.open("ab") as err:
        process = subprocess.Popen(command, cwd=work, stdin=inp, stdout=out, stderr=err,
                                   start_new_session=True, preexec_fn=_limits(memory_mib, 512 * 1024 * 1024, stack_bytes))
        def expire():
            if done.is_set():
                return
            try:
                os.killpg(process.pid, signal.SIGKILL)
            except ProcessLookupError:
                return
            timed_out.set()
        timer = threading.Timer(timeout, expire)
        timer.daemon = True
        timer.start()
        try:
            while True:
                try:
                    _, status, usage = os.wait4(process.pid, 0)
                    break
                except InterruptedError:
                    continue
            process.returncode = os.waitstatus_to_exitcode(status)
            elapsed = time.perf_counter_ns() - started
            cpu_ns = max(0, round((usage.ru_utime + usage.ru_stime) * 1e9))
        except BaseException:
            expire()
            process.wait()
            raise
        finally:
            done.set()
            timer.cancel()
            timer.join()
    return {"returncode": process.returncode, "timed_out": timed_out.is_set(),
            "elapsed_ns": elapsed, "cpu_ns": cpu_ns}


def build(source: bytes, compiler: str, directory: Path, timeout: float, flags: list[str]) -> dict:
    directory.mkdir(parents=True, exist_ok=True)
    snapshot, binary = directory / "submission.cpp", directory / "solution"
    snapshot.write_bytes(source)
    command = [compiler, str(snapshot), *flags, "-o", str(binary)]
    diagnostics = directory / "compile.stderr"
    result = execute(command, work=directory, output=directory / "compile.stdout", log=diagnostics,
                     timeout=timeout, memory_mib=2048)
    passed = result["returncode"] == 0 and not result["timed_out"] and binary.is_file()
    if not passed:
        result["diagnostic_excerpt"] = diagnostics.read_bytes()[-16000:].decode(errors="replace")
    return {"status": "PASS" if passed else "FAIL", "command": command,
            "source_sha256": digest(source), "binary": str(binary),
            "executable_sha256": file_digest(binary) if passed else None, **result}


def baseline_build_comparison(build: dict, profile: str) -> str:
    if build["status"] == "PASS":
        return "MEASURED"
    if build.get("timed_out") is True:
        return "BASELINE_BUILD_TIMEOUT"
    diagnostics = build.get("diagnostic_excerpt")
    if profile == "atcoder" and isinstance(diagnostics, str):
        errors = re.findall(r"(?m)^.*\berror: (.+)$", diagnostics)
        limit_error = f"evaluation operation count exceeds limit of {ci_compile_profiles.ATCODER_CONSTEXPR_OPS_LIMIT}"
        if errors and all(limit_error in error
                          and "-fconstexpr-ops-limit=" in error for error in errors):
            return "BASELINE_CONSTEXPR_LIMIT"
    return "BASELINE_API_INCOMPATIBLE"


def output_matches(actual: Path, case: dict, entry: dict, work: Path) -> bool:
    expected, source = Path(case["expected_path"]), Path(case["input_path"])
    if file_digest(source) != case["input_sha256"] or file_digest(expected) != case["expected_sha256"]:
        raise PerformanceError("benchmark input/oracle changed")
    if case["checker"] == "official":
        if file_digest(Path(case["checker_path"])) != case["checker_sha256"]:
            raise PerformanceError("official checker changed")
        result = execute([case["checker_path"], str(source), str(actual), str(expected)],
                         work=work, output=work / "checker.stdout", log=work / "checker.stderr", timeout=60,
                         stack_bytes=official.CHECKER_STACK_BYTES)
        # testlibの解答不一致と、チェッカー自身の異常終了を区別する。
        if result["timed_out"] or result["returncode"] not in (0, 1, 2, 4):
            raise PerformanceError("official checker failed to judge: " + case["name"])
        return result["returncode"] == 0
    if case["checker"] == "tokens":
        with actual.open("rb") as a, expected.open("rb") as e:
            import itertools
            missing = object()
            left = (token for line in a for token in line.split())
            right = (token for line in e for token in line.split())
            first = next(right, missing)
            if first is missing:
                return case.get("allow_empty_output") is True and next(left, missing) is missing
            return all(x == y and x is not missing for x, y in itertools.zip_longest(left, itertools.chain([first], right), fillvalue=missing))
    custom = {**entry, "sample_checker": case["checker"]}
    try:
        return prepare_verify.matches(custom, {"input": source.read_text(), "output": expected.read_text()}, actual.read_text())
    except (UnicodeError, ValueError, IndexError):
        return False


def inline_cases(entry: dict, work: Path) -> list[dict]:
    """quickで公式generate.pyを呼ばず、既存の小さい登録例だけを使う。"""
    result = []
    for index, sample in enumerate(entry.get("samples", [])):
        directory = work / "inline" / str(index)
        directory.mkdir(parents=True, exist_ok=True)
        source, expected = directory / "input.txt", directory / "expected.txt"
        source.write_text(sample["input"])
        expected.write_text(sample["output"])
        result.append({"name": "registered_sample_" + str(index).zfill(2),
            "input_path": str(source), "expected_path": str(expected),
            "input_sha256": file_digest(source), "expected_sha256": file_digest(expected),
            "checker": entry.get("sample_checker", "tokens"),
            "allow_empty_output": sample.get("allow_empty_output") is True,
            "metadata": {"size_profile": "quick_smoke", "source": "registered_inline_sample",
                         "maximum_claim": "not a Max or large performance case"}})
    return result


def measure_case(binaries: dict[str, Path], case: dict, entry: dict, work: Path, *, repetitions: int,
                 warmups: int, timeout: float, memory_mib: int) -> dict:
    samples = {name: [] for name in binaries}
    cpu_samples = {name: [] for name in binaries}
    output_hashes = {name: [] for name in binaries}
    comparison = "MEASURED" if "base" in binaries else "BASELINE_UNAVAILABLE"
    baseline_failure = None
    orders = []
    for trial in range(warmups + repetitions):
        order = list(binaries)
        if trial % 2:
            order.reverse()
        if baseline_failure is not None:
            order = [role for role in order if role != "base"]
        orders.append(order)
        for role in order:
            actual = work / (role + ".actual")
            run = execute([str(binaries[role])], stdin=Path(case["input_path"]), work=work, output=actual,
                          log=work / (role + ".runtime.log"), timeout=timeout, memory_mib=memory_mib)
            executed = run["returncode"] == 0 and not run["timed_out"]
            correct = executed and output_matches(actual, case, entry, work)
            if not correct:
                if role == "current":
                    raise PerformanceError("current program failed correctness or execution: " + case["name"])
                baseline_failure = {"trial": trial, "returncode": run["returncode"], "timed_out": run["timed_out"],
                                    "oracle_passed": False if executed else None,
                                    "output_sha256": file_digest(actual)}
                comparison = "BASELINE_INVALID"
                samples.pop("base", None)
                cpu_samples.pop("base", None)
                output_hashes.pop("base", None)
                continue
            if trial >= warmups:
                if type(run.get("cpu_ns")) is not int or run["cpu_ns"] < 0:
                    raise PerformanceError("child CPU timing unavailable: " + case["name"])
                samples[role].append(run["elapsed_ns"])
                cpu_samples[role].append(run["cpu_ns"])
                output_hashes[role].append(file_digest(actual))
    if len(samples.get("current", [])) != repetitions:
        raise PerformanceError("current measurement repetition missing")
    if "base" in samples and len(samples["base"]) != repetitions:
        raise PerformanceError("baseline measurement repetition missing")
    medians = {role: statistics.median(values) for role, values in samples.items()}
    result = {"name": case["name"], "status": "PASS", "correctness_passed": True,
            "input_sha256": case["input_sha256"], "expected_sha256": case["expected_sha256"],
            "input_bytes": Path(case["input_path"]).stat().st_size, "metadata": case.get("metadata", {}),
            "pattern": case.get("category", "official_representative" if case["checker"] == "official" else case["name"]),
            "checker": case["checker"], "samples_ns": samples, "median_ns": medians,
            "output_sha256": output_hashes,
            "cpu_samples_ns": cpu_samples, "cpu_median_ns": {role: statistics.median(values) for role, values in cpu_samples.items()},
            "current_over_base": medians["current"] / medians["base"] if "base" in medians else None,
            "comparison": comparison, "orders": orders}
    if baseline_failure is not None:
        result["baseline_failure"] = baseline_failure
    return result


def regression_result(case: dict, *, regression_profile: str = performance_regression.LOCAL_PROFILE) -> dict:
    metric = "child_cpu_ns"
    reason = case["comparison"]
    if reason == "MEASURED":
        return performance_regression.evaluate(metric, case["cpu_samples_ns"], profile=regression_profile)
    failure = case.get("baseline_failure", {})
    prior_wrong_answer = (reason == "BASELINE_INVALID" and type(failure.get("returncode")) is int
                          and failure["returncode"] == 0
                          and failure.get("timed_out") is False and failure.get("oracle_passed") is False)
    status = ("NOT_COMPARABLE" if reason in ("BASELINE_NEW_LIBRARY", "BASELINE_CONSTEXPR_LIMIT")
              or prior_wrong_answer else "FAIL")
    return {"status": status, "metric": metric, "reason": reason}


def measure_with_comparison(binaries: dict[str, Path], case: dict, entry: dict, work: Path,
                            baseline_comparison: str, *,
                            regression_profile: str = performance_regression.LOCAL_PROFILE, **options) -> dict:
    result = measure_case(binaries, case, entry, work, **options)
    if "base" not in binaries:
        result["comparison"] = baseline_comparison
    if baseline_comparison != "NOT_REQUESTED":
        result["performance_regression"] = regression_result(result, regression_profile=regression_profile)
    return result


def paired_cpu_samples(cases: list[dict]) -> dict[str, dict[str, list[int]]]:
    return {case["name"]: case["cpu_samples_ns"] for case in cases if case["comparison"] == "MEASURED"}


def confirm_worst_case(row: dict, binaries: dict[str, Path], cases: list[dict],
                       entry: dict, work: Path, *,
                       stage_case=nullcontext,
                       regression_profile: str = performance_regression.LOCAL_PROFILE, **options) -> None:
    paired = paired_cpu_samples(row["cases"])
    if not paired:
        return
    worst = performance_regression.evaluate_worst("child_cpu_ns", paired, profile=regression_profile)
    if worst["status"] == "FAIL":
        # 最大のケースは再計測で変わり得るため、比較できた全ケースを取り直す。
        row["worst_case_confirmation"] = paired
        reverse_order = dict(reversed(tuple(binaries.items())))
        for index, case in enumerate(cases):
            if case["name"] not in paired:
                continue
            with stage_case(case):
                result = measure_with_comparison(reverse_order, case, entry, work, "MEASURED",
                                                 regression_profile=regression_profile, **options)
            if result["comparison"] != "MEASURED":
                result["performance_regression"].update(status="FAIL", reason="BASELINE_CONFIRMATION_UNAVAILABLE")
            row["cases"][index] = result
        paired = paired_cpu_samples(row["cases"])
        if not paired:
            return
        worst = performance_regression.evaluate_worst("child_cpu_ns", paired, profile=regression_profile)
    row["worst_case_regression"] = worst


def library_regression_status(row: dict) -> str:
    status = row.get("worst_case_regression", {}).get("status", "NOT_COMPARABLE")
    if row.get("status") == "FAIL" or any(
            case["comparison"] != "MEASURED" and case["performance_regression"]["status"] == "FAIL"
            for case in row["cases"]):
        return "FAIL"
    return status


def complete(report: dict, *, entries: dict | None = None, nonlc=None) -> bool:
    planned = report["selected_library_paths"]
    rows = report["results"]
    if (not planned or len(planned) != len(set(planned)) or [r["library"] for r in rows] != planned
            or report.get("source_freeze_passed") is not True):
        return False
    for row in rows:
        cases = row.get("cases", [])
        # 生成・ビルドに失敗した行にはケース一覧がない。元の失敗を保持する。
        if row.get("status") != "PASS" or row.get("current_measurements_passed") is not True:
            return False
        if entries is not None:
            entry = entries[row["verify_id"]]
            quick = report["size_profile"] == "quick_smoke"
            if row["case_source"] == "deterministic_non_lc":
                expected = [c["name"] for c in nonlc.plans_for_library(row["library"], quick=quick)]
            elif quick:
                expected = ["registered_sample_" + str(i).zfill(2) for i, _ in enumerate(entry.get("samples", []))]
            else:
                manifest = row.get("official_case_input_bytes", {})
                if not manifest or set(manifest) != set(row.get("official", {}).get("expected_cases", [])):
                    return False
                expected = list(official_selection(manifest, False))
            if row.get("expected_case_names") != expected:
                return False
        if (not row.get("expected_case_names")
                or len(row["expected_case_names"]) != len(set(row["expected_case_names"]))
                or [c["name"] for c in cases] != row["expected_case_names"]):
            return False
        for case in cases:
            samples = case.get("samples_ns", {}).get("current", [])
            if (case.get("status") != "PASS" or case.get("correctness_passed") is not True
                    or len(samples) != report["repetitions"] or any(type(n) is not int or n <= 0 for n in samples)):
                return False
    return True


def write_report(output: Path, report: dict) -> None:
    output.mkdir(parents=True, exist_ok=True)
    temp = output / "result.json.tmp"
    temp.write_text(json.dumps(report, ensure_ascii=False, indent=2) + "\n")
    temp.replace(output / "result.json")
    regression_profile = report.get("regression_profile")
    bounds = []
    if regression_profile is not None:
        tolerance = performance_regression.tolerance_description("child_cpu_ns", regression_profile)
        bounds.append(f"上限は基準の最大値に{tolerance}を加えた値です。超過時は全比較ケースを再測定します。")
    lines = ["## 全ライブラリのVerifyプログラム性能", "", "状態: " + report["status"], "",
             "現在版の入力規模・wall時間・子プロセスCPU時間の中央値と生データを保存します。",
             "wall時間はプロセス起動・入出力・runnerの待機遅延を含みます。本体だけのkernel測定・オンライン時間とは区別します。",
             "基準指定時は共通のケースごとにCPU時間の中央値を求め、ライブラリごとに各版の最大値を比較します。",
             *bounds,
             "ケースごとの時間比較は参考です。実行・比較エラーと、明示指定した固定予算は別途検査します。", "",
             "入力プロファイル: " + report["size_profile"], "",
             "CPU: " + str(report.get("cpu_model", "unknown")),
             "環境: " + str(report.get("platform", "unknown")),
             "CPU affinity: " + str(report.get("affinity")),
             "コンパイラ: " + str(report.get("compiler", {}).get("version", "未取得")),
             "コンパイル設定: " + report.get("compile_profile", "standard"),
             "フラグ: " + " ".join(report.get("flags", [])), "",
             f"全対象 {len(report['all_library_paths'])}、このshard {len(report['selected_library_paths'])}。", "",
             "| 本体 | Verify | ケース | 入力規模・先頭行 | wall ms | CPU ms | 基準CPU ms | 現行/基準 | 固定予算CPU ms | 比較の詳細 | 入力 bytes | 繰り返し |",
             "| --- | --- | --- | --- | ---: | ---: | ---: | ---: | ---: | --- | ---: | ---: |"]
    for row in report["results"]:
        for case in row.get("cases", []):
            med = case["median_ns"]
            cpu = case["cpu_median_ns"]["current"] / 1e6
            metadata = case.get("metadata", {})
            dimensions = metadata.get("dimensions", metadata.get("input_header", "登録入力例"))
            description = json.dumps(dimensions, ensure_ascii=False) if isinstance(dimensions, dict) else str(dimensions)
            description = description.replace("|", "&#124;").replace("\n", " ")
            comparison = case.get("performance_regression", {})
            fixed = case.get("performance_limit", {})
            base_cpu = comparison.get("baseline_median_ns")
            baseline_cpu = base_cpu if base_cpu is not None else fixed.get("baseline_median_ns")
            baseline = f"{baseline_cpu/1e6:.3f}" if baseline_cpu is not None else "—"
            ratio = f"{comparison['observed_median_ns'] / base_cpu:.3f}×" if base_cpu else "—"
            budget = f"{fixed['limit_ns']/1e6:.3f} ({fixed['status']})" if fixed else "—"
            judgement = comparison.get("reason", "個別比較 " + comparison["status"] + "（参考）"
                                       if comparison else "計測のみ")
            lines.append(f"| {row['library']} | {row['verify_id']} | {case['name']} | {description} | {med['current']/1e6:.3f} | {cpu:.3f} | {baseline} | {ratio} | {budget} | {judgement} | {case['input_bytes']} | {report['repetitions']} |")
    if report.get("base_revision"):
        counts = report.get("performance_regression_counts", {})
        lines += ["", "比較元 " + report["base_revision"],
                  f"ライブラリごとの比較は合格 {counts.get('PASS', 0)}、失敗 {counts.get('FAIL', 0)}、比較不可 {counts.get('NOT_COMPARABLE', 0)}。",
                  "", "| 本体 | 基準の最大ケース | 基準CPU ms | 現行の最大ケース | 現行CPU ms | 上限CPU ms | 共通ケース数 | 退行判定 |",
                  "| --- | --- | ---: | --- | ---: | ---: | ---: | --- |"]
        for row in report["results"]:
            worst = row.get("worst_case_regression")
            status = library_regression_status(row) if row["status"] in ("PASS", "FAIL") else "計測中"
            if worst is None:
                lines.append(f"| {row['library']} | — | — | — | — | — | — | {status} |")
                continue
            lines.append(f"| {row['library']} | {worst['baseline_case']} | {worst['baseline_median_ns']/1e6:.3f} | "
                         f"{worst['observed_case']} | {worst['observed_median_ns']/1e6:.3f} | "
                         f"{worst['limit_ns']/1e6:.3f} | {worst['compared_cases']} | {status} |")
        lines += ["",
                  "旧driverを使った理由とソースSHA、再計測前の値はresult.jsonで確認できます。"]
    if report.get("error"):
        lines += ["", str(report["error"]).replace("\n", " ")[:1000]]
    (output / "summary.md").write_text("\n".join(lines) + "\n")


def record_failure(target: dict, error: Exception, *, key: str = "error") -> None:
    target["status"], target[key] = "FAIL", str(error)
    details = getattr(error, "details", None)
    if isinstance(details, dict):
        target[key + "_details"] = dict(details)


def run(args: argparse.Namespace) -> int:
    root = ROOT.resolve()
    cache, output = official.external_cache(args.cache, args.output, root)
    if output.is_relative_to(root):
        raise PerformanceError("output must be outside repository")
    nonlc = nonlc_module()
    entries = prepare_verify.collect()
    by_id = {entry["id"]: entry for entry in entries}
    all_rows = catalog(entries, nonlc, root)
    selected = select_catalog(all_rows, args.library, args.shard_index, args.shard_count)
    profile = getattr(args, "compile_profile", "standard")
    flags = ci_compile_profiles.compile_flags(profile, FLAGS)
    plan = {"type": "sirkit.performance-all", "format": 1, "scope": SCOPE,
            "compile_profile": profile, "flags": flags,
            "all_library_paths": [r["library"] for r in all_rows],
            "selected_library_paths": [r["library"] for r in selected],
            "shard_index": args.shard_index, "shard_count": args.shard_count,
            "selection_kind": "explicit_libraries" if args.library else "all_libraries",
            "size_profile": "quick_smoke" if args.quick else "full_representative", "catalog": selected}
    if args.plan:
        print(json.dumps({**plan, "status": "PLAN_ONLY", "executed": False}, ensure_ascii=False, indent=2))
        return 0
    output.mkdir(parents=True, exist_ok=True)
    report = {**plan, "status": "running", "online_status": "not_evaluated",
              "performance_scope": SCOPE,
              "timing_scope": "whole_program_wall_time_including_startup_io_timer_setup_and_wait4_return; wait_timeout_polling_not_used; child_cpu_time_recorded_separately; checker_excluded",
              "platform": platform.platform(), "machine": platform.machine(), "cpu_model": ci_benchmark.cpu_model(),
              "affinity": sorted(os.sched_getaffinity(0)) if hasattr(os, "sched_getaffinity") else None,
              "repetitions": args.repetitions, "warmups": args.warmups,
              "base_revision": None, "source_freeze_passed": False,
              "runner": {"source": "Scripts/ci_performance_all.py", "sha256": file_digest(Path(__file__))},
              "compiler": {}, "source_hashes": {}, "results": [dict(row, status="NOT_RUN", cases=[], current_measurements_passed=False) for row in selected]}
    write_report(output, report)
    frozen = None
    try:
        limits_path = getattr(args, "limits", None)
        limits = performance_limits.load(limits_path) if limits_path is not None else None
        if limits is not None:
            report.update(performance_limits=limits.metadata, performance_limits_passed=False)
        revision = ci_benchmark.resolve_base(args.base, root) if args.base else None
        report["base_revision"] = revision
        regression_profile = getattr(args, "regression_profile", performance_regression.LOCAL_PROFILE)
        if revision is not None:
            report.update(regression_policy=performance_regression.WORST_CASE_POLICY, regression_profile=regression_profile,
                          performance_regression_passed=False,
                          performance_regression_counts={})
        compiler = shutil.which(args.compiler)
        if compiler is None:
            raise PerformanceError("compiler missing")
        report["compiler"] = {"command": [compiler], "version": subprocess.check_output([compiler, "--version"], timeout=30, text=True).splitlines()[0],
                              "sha256": file_digest(Path(compiler).resolve())}
        frozen = freeze_sources(selected, by_id, root)
        report["source_hashes"] = frozen
        pin = official.load_pin(root / "Verify/ci/library_checker.json")
        cache.mkdir(parents=True, exist_ok=True)
        with official.cache_lock(cache):
            prepared, builds = {}, {}
            checkout = None
            for row in report["results"]:
                entry = by_id[row["verify_id"]]
                name = digest(row["library"].encode())[:16]
                work = official.cache_path(cache, "performance", name)
                work.mkdir(parents=True, exist_ok=True)
                try:
                    driver = (root / entry["driver"]).read_text()
                    current_libraries = {p: (root / p).read_bytes() for p in entry["libraries"]}
                    current_source, _ = render_program(entry, current_libraries, driver, root)
                    support = prepare_verify.common_source_hashes(driver)
                    row.update(embedded_compile_settings=prepare_verify.compile_settings(entry, current_source.decode(), True),
                               source_hashes={p: digest(b) for p, b in current_libraries.items()},
                               driver=entry["driver"], driver_sha256=digest(driver.encode()),
                               support_source_hashes=support, submission_sha256=digest(current_source),
                               performance_scope=SCOPE)
                    source_key = digest(current_source)
                    if source_key not in builds:
                        builds[source_key] = build(current_source, compiler, work / "current", args.compile_timeout, flags)
                    row["current_build"] = builds[source_key]
                    if row["current_build"]["status"] != "PASS":
                        raise PerformanceError("current build failed")
                    binaries = {"current": Path(row["current_build"]["binary"])}
                    base, baseline_proof = baseline_libraries(revision, list(current_libraries), root) if revision else (None, {})
                    row["baseline_sources"] = baseline_proof
                    row["baseline_comparison"] = "NOT_REQUESTED"
                    if revision and base is None:
                        row["baseline_comparison"] = ("BASELINE_NEW_LIBRARY" if row["library"] in baseline_proof["missing_paths"]
                                                      else "BASELINE_DEPENDENCY_MISSING")
                    elif revision and base is not None:
                        baseline_source, adapter = render_program(entry, base, driver, root, legacy_adapter=True)
                        row["baseline_adapter"] = adapter
                        row["baseline_driver"] = {"kind": "current", "path": row["driver"],
                                                  "sha256": row["driver_sha256"], "same_as_current": True}
                        base_key = digest(baseline_source)
                        if base_key not in builds:
                            builds[base_key] = build(baseline_source, compiler, work / "base", args.compile_timeout, flags)
                        if builds[base_key]["status"] != "PASS" and not builds[base_key].get("timed_out"):
                            previous = archived_driver(revision, entry, row["driver_sha256"], support, root)
                            if previous is not None:
                                row["baseline_initial_build"] = builds[base_key]
                                previous_source, row["baseline_driver"] = previous
                                baseline_source, row["baseline_adapter"] = render_program(
                                    entry, base, previous_source, root, legacy_adapter=True)
                                base_key = digest(baseline_source)
                                if base_key not in builds:
                                    builds[base_key] = build(baseline_source, compiler, work / "base_archived_driver",
                                                            args.compile_timeout, flags)
                        row["baseline_build"] = builds[base_key]
                        row["baseline_comparison"] = baseline_build_comparison(builds[base_key], profile)
                        if builds[base_key]["status"] == "PASS":
                            binaries["base"] = Path(builds[base_key]["binary"])
                    stage_case = nullcontext
                    if row["case_source"] == "official_library_checker" and args.quick:
                        cases = inline_cases(entry, work)
                    elif row["case_source"] == "official_library_checker":
                        problem = row["problem_id"]
                        if problem not in prepared:
                            if checkout is None:
                                checkout = official.checkout_official(cache, pin)
                            prepared[problem] = official.official_problem(checkout, problem, cache, pin, args.generation_timeout)
                        directory, details = prepared[problem]
                        sizes = official_input_sizes(checkout, directory, details, cache, args.generation_timeout)
                        cases = select_official_cases(directory, details, args.quick, sizes=sizes)
                        row["official"] = details
                        row["official_case_input_bytes"] = sizes
                        if details.get("generation_strategy") == "one_case_at_a_time":
                            stage_case = partial(staged_official_case, checkout=checkout, directory=directory,
                                                 details=details, cache=cache, timeout=args.generation_timeout)
                    else:
                        cases = []
                        for case_plan in nonlc.plans_for_library(row["library"], quick=args.quick):
                            generated = nonlc.generate_case(case_plan, work / "inputs" / case_plan["name"])
                            case = {**case_plan, **generated}
                            case.setdefault("metadata", case_plan)
                            cases.append(case)
                    row["expected_case_names"] = [case["name"] for case in cases]
                    if not cases or len(row["expected_case_names"]) != len(set(row["expected_case_names"])):
                        raise PerformanceError("empty or duplicate performance cases")
                    budgets = ({case["name"]: limits.whole(row["library"], row["verify_id"], case["name"],
                                case["input_bytes"] if "input_bytes" in case else Path(case["input_path"]).stat().st_size)
                                for case in cases} if limits is not None else {})
                    options = {"repetitions": args.repetitions, "warmups": args.warmups,
                               "timeout": args.case_timeout, "memory_mib": args.memory_mib}
                    for case in cases:
                        with stage_case(case):
                            result = measure_with_comparison(binaries, case, entry, work, row["baseline_comparison"],
                                                             regression_profile=regression_profile, **options)
                        row["cases"].append(result)
                        write_report(output, report)
                    if revision is not None:
                        confirm_worst_case(row, binaries, cases, entry, work,
                                           stage_case=stage_case, regression_profile=regression_profile, **options)
                    row["current_measurements_passed"], row["status"] = True, "PASS"
                    if limits is not None:
                        for result in row["cases"]:
                            result["performance_limit"] = limits.evaluate(budgets[result["name"]],
                                                                           result["cpu_samples_ns"]["current"])
                        row["performance_limits_passed"] = all(c["performance_limit"]["status"] == "PASS" for c in row["cases"])
                except (OSError, ValueError, official.RunnerError, subprocess.SubprocessError) as error:
                    record_failure(row, error)
                finally:
                    # 入力・正解はhashと規模を記録済み。Max入力をshard全体で溜め込まない。
                    problem = row.get("problem_id")
                    if not args.quick and row["case_source"] == "official_library_checker" and checkout is not None:
                        try:
                            directory = prepared.pop(problem)[0] if problem in prepared else official.find_problem(checkout, problem)
                            row["test_data_cleanup"] = official.discard_test_data(checkout, directory)
                        except (OSError, official.RunnerError) as error:
                            record_failure(row, error, key="cleanup_error")
                            if "error" not in row:
                                record_failure(row, error)
                    shutil.rmtree(work / "inputs", ignore_errors=True)
                    for actual in work.glob("*.actual"):
                        actual.unlink(missing_ok=True)
                write_report(output, report)
            check_frozen(frozen, root)
            if limits is not None:
                limits.check_unchanged()
            report["source_freeze_passed"] = True
            report["current_measurements_passed"] = complete(report, entries=by_id, nonlc=nonlc)
            if revision is not None:
                counts = Counter(library_regression_status(row) for row in report["results"])
                has_existing_libraries = any(row.get("baseline_comparison") != "BASELINE_NEW_LIBRARY"
                                             for row in report["results"])
                report["performance_regression_counts"] = dict(counts)
                report["performance_regression_passed"] = (
                    report["current_measurements_passed"] and not counts["FAIL"]
                    and (counts["PASS"] > 0 or not has_existing_libraries))
                if report["current_measurements_passed"] and not report["performance_regression_passed"]:
                    report["error"] = ("性能退行または基準版の比較失敗を検出しました。" if counts["FAIL"]
                                       else "比較対象が既存ですが、比較できたケースがありません。")
            if limits is not None:
                report["performance_limits_passed"] = bool(report["results"]) and all(
                    row.get("performance_limits_passed") is True for row in report["results"])
                if report["current_measurements_passed"] and not report["performance_limits_passed"] and "error" not in report:
                    report["error"] = "固定性能予算を超過しました。正しさの照合結果と測定値はresultsへ保存しています。"
            report["status"] = "PASS" if (report["current_measurements_passed"]
                                         and report.get("performance_regression_passed", True)
                                         and report.get("performance_limits_passed", True)) else "FAIL"
    except (OSError, ValueError, official.RunnerError, subprocess.SubprocessError) as error:
        record_failure(report, error)
    finally:
        if frozen is not None:
            try:
                check_frozen(frozen, root)
            except (OSError, PerformanceError) as error:
                report["source_freeze_passed"], report["current_measurements_passed"] = False, False
                if "performance_regression_passed" in report:
                    report["performance_regression_passed"] = False
                record_failure(report, error)
        write_report(output, report)
    return 0 if report["status"] == "PASS" else 1


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--base", help="同じ入力で退行を検査する基準commit。省略時は現在版だけを計測する")
    parser.add_argument("--cache", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--limits", type=Path, help="旧方式の固定性能予算JSONを任意で照合する")
    parser.add_argument("--compiler", default="g++")
    parser.add_argument("--compile-profile", choices=ci_compile_profiles.COMPILE_PROFILES, default="standard",
                        help="基準と現在版の両方に使うコンパイル設定")
    parser.add_argument("--regression-profile", choices=performance_regression.REGRESSION_PROFILES,
                        default=performance_regression.LOCAL_PROFILE, help="基準版との性能比較で使う許容幅の設定")
    parser.add_argument("--repetitions", type=int, default=3)
    parser.add_argument("--warmups", type=int, default=1)
    parser.add_argument("--quick", action="store_true")
    parser.add_argument("--plan", action="store_true")
    parser.add_argument("--library", action="append")
    parser.add_argument("--shard-index", type=int, default=0)
    parser.add_argument("--shard-count", type=int, default=1)
    parser.add_argument("--case-timeout", type=float, default=120)
    parser.add_argument("--compile-timeout", type=float, default=300)
    parser.add_argument("--generation-timeout", type=float, default=1200)
    parser.add_argument("--memory-mib", type=int, default=2048)
    args = parser.parse_args(argv)
    if not 1 <= args.repetitions <= 20 or not 0 <= args.warmups <= 5:
        parser.error("invalid repetitions/warmups")
    if not all(math.isfinite(v) and v > 0 for v in (args.case_timeout, args.compile_timeout, args.generation_timeout)):
        parser.error("invalid timeout")
    if not 64 <= args.memory_mib <= 4096:
        parser.error("invalid memory limit")
    try:
        return run(args)
    except (OSError, ValueError, official.RunnerError, subprocess.SubprocessError) as error:
        print(str(error), file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
