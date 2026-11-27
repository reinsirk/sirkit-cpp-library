#!/usr/bin/env python3
# SPDX-License-Identifier: CC0-1.0
"""公式Library Checker全ケースを固定版の外部キャッシュで照合する。"""
from __future__ import annotations

import argparse
from collections import Counter
from contextlib import contextmanager
import datetime
import fcntl
import hashlib
import json
import math
import os
from pathlib import Path
import re
import resource
import shutil
import signal
import subprocess
import sys
import time
import tomllib
from urllib.parse import urlparse

try:
    from . import ci_verify, prepare_verify
except ImportError:
    import ci_verify
    import prepare_verify

ROOT = Path(__file__).resolve().parents[1]
PIN = ROOT / "Verify/ci/library_checker.json"
GENERATION_STACK_BYTES = 512 * 1024 * 1024
CHECKER_STACK_BYTES = 64 * 1024 * 1024
OWN_INPUTS = {"Scripts/ci_library_checker.py", "Verify/ci/library_checker.json"}
PERFORMANCE_SCOPE = (
    "whole_program_wall_time_including_process_start_io_and_runner_wait; checker excluded; "
    "not a kernel benchmark, baseline comparison, or online judge result"
)


class RunnerError(RuntimeError):
    def __init__(self, kind: str, details: dict | None = None):
        super().__init__(kind)
        self.kind = kind
        self.details = details or {}


def digest(path: Path) -> str:
    with path.open("rb") as stream:
        return hashlib.file_digest(stream, "sha256").hexdigest()


def external_cache(cache: Path, output: Path, root: Path = ROOT) -> tuple[Path, Path]:
    """ネットワーク・mkdirより先にsymlinkを含む境界を確認する。"""
    resolved, destination, repository = cache.expanduser().resolve(), output.expanduser().resolve(), root.resolve()
    if resolved == repository or repository in resolved.parents:
        raise ValueError("--cacheはリポジトリ外へ指定してください")
    if resolved == destination or resolved in destination.parents or destination in resolved.parents:
        raise ValueError("--cacheと--outputは互いに含まれない別ディレクトリにしてください")
    return resolved, destination


def load_pin(path: Path = PIN) -> dict:
    pin = json.loads(path.read_text())
    if set(pin) != {"repository", "revision", "license_url"}:
        raise ValueError("公式設定にはrepository/revision/license_urlだけを指定します")
    if pin["repository"] != "https://github.com/yosupo06/library-checker-problems.git":
        raise ValueError("公式リポジトリURLが一致しません")
    if not re.fullmatch(r"[0-9a-f]{40}", pin["revision"]):
        raise ValueError("公式revisionは40桁のcommit SHAを指定してください")
    expected = "https://github.com/yosupo06/library-checker-problems/blob/" + pin["revision"] + "/LICENSE"
    if pin["license_url"] != expected:
        raise ValueError("公式ライセンスURLが固定revisionと一致しません")
    return pin


def cache_path(cache: Path, *parts: str) -> Path:
    path = cache.joinpath(*parts)
    resolved = path.resolve()
    if resolved != cache and cache not in resolved.parents:
        raise ValueError("外部cache内のパスが別の場所を指しています")
    return path


def problem_id(entry: dict) -> str | None:
    parsed = urlparse(entry.get("problem_url") or "")
    match = re.fullmatch(r"/problem/([A-Za-z0-9_]+)/?", parsed.path)
    return match[1] if parsed.scheme == "https" and parsed.netloc == "judge.yosupo.jp" and match else None


def selection(entries: list[dict], changed: list[str], requested: list[str] | None,
              all_entries: bool, shard_index: int, shard_count: int, root: Path = ROOT) -> dict:
    if shard_count < 1 or not 0 <= shard_index < shard_count:
        raise ValueError("shardは0 <= index < countの範囲にしてください")
    if any(not re.fullmatch(r"[A-Za-z0-9_][A-Za-z0-9_.-]*", e["id"]) for e in entries):
        raise ValueError("Verify IDにパスや特殊文字は指定できません")
    universal = bool(set(changed) & OWN_INPUTS)
    plan = ci_verify.select_entries(entries, changed, requested=requested,
                                   all_entries=all_entries or (universal and not requested), root=root)
    by_id = {entry["id"]: entry for entry in entries}
    candidates = [identifier for identifier in plan["selected_ids"] if problem_id(by_id[identifier])]
    excluded = [identifier for identifier in plan["selected_ids"] if identifier not in candidates]
    return {**plan, "coverage": "library_checker_official_full_cases", "all_selected_ids": candidates,
            "selected_ids": candidates[shard_index::shard_count],
            "excluded_non_library_checker_ids": excluded,
            "shard_index": shard_index, "shard_count": shard_count}


def write_report(directory: Path, report: dict) -> None:
    ci_verify.write_report(directory / "result.json", report)


def execute(command: list[str], *, cwd: Path, log: Path, timeout: float,
            stdin=None, stdout=None, env: dict | None = None,
            stack_bytes: int | None = None) -> tuple[int | None, int]:
    """生ログは外部cacheだけへ保存し、タイムアウト時は子プロセス群も止める。"""
    started = time.perf_counter_ns()
    child_setup = None
    if stack_bytes is not None:
        soft, hard = generation_stack_limit(stack_bytes)
        def child_setup():
            resource.setrlimit(resource.RLIMIT_STACK, (soft, hard))
            resource.setrlimit(resource.RLIMIT_CORE, (0, 0))
    with log.open("ab") as errors:
        process = subprocess.Popen(command, cwd=cwd, stdin=stdin,
                                   stdout=stdout if stdout is not None else errors,
                                   stderr=errors, env=env, start_new_session=True, preexec_fn=child_setup)
        try:
            code = process.wait(timeout=timeout)
        except subprocess.TimeoutExpired:
            os.killpg(process.pid, signal.SIGKILL)
            process.wait()
            code = None
    return code, time.perf_counter_ns() - started


def git_text(checkout: Path, *arguments: str) -> str:
    return subprocess.run(["git", "-C", str(checkout), *arguments], check=True,
                          capture_output=True, text=True, timeout=120).stdout.strip()


def checkout_official(cache: Path, pin: dict) -> Path:
    checkout = cache_path(cache, "official", pin["revision"])
    log = cache_path(cache, "checkout.log")
    if not checkout.exists():
        checkout.mkdir(parents=True)
        commands = [["git", "init", str(checkout)],
                    ["git", "-C", str(checkout), "remote", "add", "origin", pin["repository"]],
                    ["git", "-C", str(checkout), "fetch", "--depth", "1", "origin", pin["revision"]],
                    ["git", "-C", str(checkout), "checkout", "--detach", "FETCH_HEAD"]]
        for command in commands:
            if execute(command, cwd=cache, log=log, timeout=180)[0] != 0:
                raise RunnerError("official_checkout_unavailable")
    try:
        if git_text(checkout, "rev-parse", "HEAD") != pin["revision"]:
            raise RunnerError("official_revision_mismatch")
        if git_text(checkout, "remote", "get-url", "origin") != pin["repository"]:
            raise RunnerError("official_remote_mismatch")
        if subprocess.run(["git", "-C", str(checkout), "diff", "--quiet", "HEAD", "--"],
                          capture_output=True, timeout=120).returncode != 0:
            raise RunnerError("official_tracked_sources_modified")
        for path in checkout.rglob("*"):
            if path.is_symlink() and checkout.resolve() not in path.resolve().parents:
                raise RunnerError("official_symlink_outside_checkout")
    except subprocess.SubprocessError as error:
        raise RunnerError("official_checkout_unavailable") from error
    return checkout


def find_problem(checkout: Path, identifier: str) -> Path:
    matches = list(checkout.glob("**/" + identifier + "/info.toml"))
    if len(matches) != 1:
        raise RunnerError("official_problem_unavailable")
    result = matches[0].parent.resolve()
    if checkout.resolve() not in result.parents:
        raise RunnerError("official_problem_outside_cache")
    return result


def expected_cases(info: dict) -> list[str]:
    names = []
    for test in info["tests"]:
        count = test["number"]
        if not isinstance(count, int) or isinstance(count, bool) or count < 1:
            raise RunnerError("invalid_official_case_count")
        names.extend(Path(test["name"]).stem + "_" + str(i).zfill(2) for i in range(count))
    if not names or len(names) != len(set(names)):
        raise RunnerError("invalid_official_case_names")
    return sorted(names)


def generation_stack_limit(requested: int = GENERATION_STACK_BYTES) -> tuple[int, int]:
    """公式プログラムの深い再帰用。親と提出の実行制限は変更しない。"""
    soft, hard = resource.getrlimit(resource.RLIMIT_STACK)
    if soft == resource.RLIM_INFINITY:
        return soft, hard
    target = max(soft, requested)
    return (target if hard == resource.RLIM_INFINITY else min(target, hard)), hard


def generation_failure(log: Path, returncode: int | None, stack_bytes: int) -> dict:
    # 生ログや公式の入出力は成果物に含めず、原因の分類と照合用SHAだけ残す。
    with log.open("rb") as stream:
        stream.seek(0, os.SEEK_END)
        size = stream.tell()
        stream.seek(max(0, size - 16384))
        tail = stream.read().decode("utf-8", errors="replace")
    return {"returncode": returncode, "timed_out": returncode is None,
            "signals": sorted(set(re.findall(r"\bSIG[A-Z0-9]+\b", tail))),
            "compiler_error_detected": bool(re.search(r"(?:^|\n).*\berror:", tail)),
            "generation_stack_bytes": stack_bytes,
            "log_sha256": digest(log), "log_bytes": size}


def official_problem(checkout: Path, identifier: str, cache: Path, pin: dict,
                     generation_timeout: float) -> tuple[Path, dict]:
    directory = find_problem(checkout, identifier)
    info = tomllib.loads((directory / "info.toml").read_text())
    names = expected_cases(info)
    command = [sys.executable, str(checkout / "generate.py"), str(directory / "info.toml")]
    env = os.environ.copy()
    env["CXX"] = "g++"
    log = cache_path(cache, identifier + ".generation.log")
    log.write_bytes(b"")
    stack_bytes, _ = generation_stack_limit()
    code, _ = execute(command, cwd=checkout, log=log, timeout=generation_timeout,
                      env=env, stack_bytes=GENERATION_STACK_BYTES)
    if code != 0:
        raise RunnerError("official_generation_unavailable", generation_failure(log, code, stack_bytes))
    hashes = json.loads((directory / "hash.json").read_text())
    if set(hashes) != {name + extension for name in names for extension in (".in", ".out")}:
        raise RunnerError("official_case_manifest_mismatch")
    for subdir, extension in [("in", ".in"), ("out", ".out")]:
        found = {p.name for p in (directory / subdir).glob("*" + extension)}
        if found != {name + extension for name in names}:
            raise RunnerError("official_case_set_mismatch")
        for name in names:
            path = directory / subdir / (name + extension)
            if directory not in path.resolve().parents or digest(path) != hashes[name + extension]:
                raise RunnerError("official_case_hash_mismatch")
    checker = directory / "checker"
    if not checker.is_file() or not os.access(checker, os.X_OK):
        raise RunnerError("official_checker_unavailable")
    return directory, {**pin, "problem_id": identifier, "info_sha256": digest(directory / "info.toml"),
                       "hash_json_sha256": digest(directory / "hash.json"),
                       "checker_source_sha256": digest(directory / "checker.cpp"),
                       "checker_sha256": digest(checker), "expected_cases": names,
                       "official_timelimit_seconds": info.get("timelimit"),
                       "generation_command": command, "generation_compiler_env": "g++",
                       "generation_stack_bytes": stack_bytes}


def provenance(entry: dict, root: Path = ROOT) -> dict:
    data, hashes = prepare_verify.render(entry)
    submission = "Verify/current/submissions/" + entry["id"] + ".cpp"
    if (root / submission).read_bytes() != data:
        raise RunnerError("submission_not_synchronized")
    support = ci_verify.dependencies(entry, root) - set(entry["libraries"]) - {entry["driver"], submission}
    return {"id": entry["id"], "libraries": list(entry["libraries"]), "source_hashes": hashes,
            "submission": submission, "submission_sha256": hashlib.sha256(data).hexdigest(),
            "driver": entry["driver"], "driver_sha256": digest(root / entry["driver"]),
            "support_source_hashes": {name: digest(root / name) for name in sorted(support)},
            "io_source_hashes": {name: digest(root / name) for name in sorted(support)
                                 if name == prepare_verify.FAST_IO}, "problem_url": entry["problem_url"],
            "embedded_compile_settings": prepare_verify.compile_settings(entry, data.decode(), True)}


def discard_test_data(checkout: Path, directory: Path) -> list[str]:
    """固定checkout配下の未追跡in/outだけを消す。原典・checkerは保持する。"""
    checkout, directory = checkout.resolve(), directory.resolve()
    if (checkout == ROOT.resolve() or ROOT.resolve() in checkout.parents
            or checkout not in directory.parents):
        raise RunnerError("test_data_cleanup_outside_checkout")
    removals = []
    for name in ("in", "out"):
        path = directory / name
        if path.is_symlink():
            raise RunnerError("test_data_cleanup_symlink")
        if not path.exists():
            continue
        if not path.is_dir() or directory not in path.resolve().parents:
            raise RunnerError("test_data_cleanup_outside_problem")
        relative = path.relative_to(checkout).as_posix()
        if git_text(checkout, "ls-files", "-z", "--", relative):
            raise RunnerError("test_data_cleanup_tracked_files")
        if any(child.is_symlink() for child in path.rglob("*")):
            raise RunnerError("test_data_cleanup_symlink")
        removals.append(path)
    for path in removals:
        shutil.rmtree(path)
    return [path.name for path in removals]


def run_case(binary: Path, directory: Path, name: str, work: Path,
             timeout: float) -> dict:
    actual = cache_path(work.resolve(), "actual.out")
    with (directory / "in" / (name + ".in")).open("rb") as source, actual.open("wb") as answer:
        code, elapsed = execute([str(binary)], cwd=work, log=cache_path(work.resolve(), "solution.log"),
                                timeout=timeout, stdin=source, stdout=answer)
    row = {"name": name, "status": "FAIL", "elapsed_ns": elapsed,
           "returncode": code, "checker_returncode": None}
    if code != 0:
        row["error_kind"] = "time_limit" if code is None else "runtime_error"
        return row
    command = [str(directory / "checker"), str(directory / "in" / (name + ".in")),
               str(actual), str(directory / "out" / (name + ".out"))]
    # 幅2木分解のように、公式の参照出力を深く再帰して検査するチェッカーもある。
    checker_code, _ = execute(command, cwd=work, log=cache_path(work.resolve(), "checker.log"), timeout=60,
                              stack_bytes=CHECKER_STACK_BYTES)
    row["checker_returncode"] = checker_code
    if checker_code == 0:
        row["status"] = "PASS"
    else:
        row["error_kind"] = "checker_rejected" if checker_code in (1, 2) else "checker_error"
    return row


def counts(results: list[dict], selected: int) -> dict:
    values = {"selected": selected, "completed": len(results),
              "passed": sum(row["status"] == "PASS" for row in results),
              "failed": sum(row["status"] == "FAIL" for row in results),
              "unavailable": sum(bool(row.get("unavailable")) for row in results)}
    for field in ("expected", "completed", "passed", "failed"):
        values["case_" + field] = sum(row.get("case_counts", {}).get(field, 0) for row in results)
    return values


def run_selected(entries: list[dict], plan: dict, cache: Path, output: Path, pin: dict,
                 compiler: str = "g++", *, root: Path = ROOT, case_timeout: float = 30,
                 generation_timeout: float = 1200, discard_data: bool = False) -> dict:
    if len(plan["selected_ids"]) != len(set(plan["selected_ids"])):
        raise ValueError("選択したVerify IDが重複しています")
    selected = {entry["id"]: entry for entry in entries if entry["id"] in plan["selected_ids"]}
    if set(selected) != set(plan["selected_ids"]):
        raise ValueError("選択したVerify IDが変化しました")
    order = sorted(plan["selected_ids"], key=lambda name: (problem_id(selected[name]) or "", name)) if discard_data else plan["selected_ids"]
    remaining = Counter(problem_id(selected[name]) for name in order)
    flags = ["-std=c++23", "-O2", "-DSKIP_RUNTIME_CHECKS=", "-DNDEBUG="]
    report = {"type": "sirkit.library-checker", "format": 1, "status": "running",
              "created_at_utc": datetime.datetime.now(datetime.timezone.utc).isoformat(),
              "online_status": "not_evaluated", "selection": plan, "official": pin,
              "compiler": {"command": [compiler], "version": None, "flags": flags},
              "runner": {"source": "Scripts/ci_library_checker.py", "sha256": digest(Path(__file__)),
                         "python": sys.version, "platform": sys.platform,
                         "cpu_affinity": sorted(os.sched_getaffinity(0)) if hasattr(os, "sched_getaffinity") else None},
              "performance_scope": PERFORMANCE_SCOPE, "case_timeout_seconds": case_timeout,
              "discard_test_data": discard_data, "execution_order": order,
              "results": [], "counts": counts([], len(selected))}
    write_report(output, report)
    if not selected:
        report["status"] = "SKIPPED"
        write_report(output, report)
        return report
    try:
        version = subprocess.run([compiler, "--version"], capture_output=True, text=True,
                                 check=True, timeout=30).stdout.splitlines()[0]
        report["compiler"]["version"] = version
        checkout = checkout_official(cache, pin)
        prepared = {}
        for identifier in order:
            entry = selected[identifier]
            problem = problem_id(entry)
            row = {"id": identifier, "status": "FAIL", "unavailable": False,
                   "cases": [], "case_counts": {"expected": 0, "completed": 0, "passed": 0, "failed": 0}}
            report["results"].append(row)
            try:
                row.update(provenance(entry, root))
                if problem is None:
                    raise RunnerError("not_a_library_checker_problem")
                if problem not in prepared:
                    prepared[problem] = official_problem(checkout, problem, cache, pin, generation_timeout)
                directory, details = prepared[problem]
                row["official"] = details
                row["case_counts"]["expected"] = len(details["expected_cases"])
                work = cache_path(cache, "runs", identifier + "-" + row["submission_sha256"])
                work.mkdir(parents=True, exist_ok=True)
                binary = cache_path(cache, "runs", work.name, "solution")
                snapshot = cache_path(cache, "runs", work.name, "submission.cpp")
                source_data = (root / row["submission"]).read_bytes()
                if hashlib.sha256(source_data).hexdigest() != row["submission_sha256"]:
                    raise RunnerError("submission_changed_before_compile")
                snapshot.write_bytes(source_data)
                command = [compiler, *flags, str(snapshot), "-o", str(binary)]
                row["compile_command"] = command
                if execute(command, cwd=root, log=cache_path(cache, "runs", work.name, "compile.log"), timeout=300)[0] != 0:
                    raise RunnerError("submission_compile_failed")
                row["executable_sha256"] = digest(binary)
                for name in details["expected_cases"]:
                    case = run_case(binary, directory, name, work, case_timeout)
                    row["cases"].append(case)
                    if case["status"] != "PASS":
                        break
                actual = [case["name"] for case in row["cases"]]
                passed = sum(case["status"] == "PASS" for case in row["cases"])
                if actual == details["expected_cases"] and passed == len(actual) and actual:
                    row["status"] = "PASS"
            except (OSError, ValueError, subprocess.SubprocessError, RunnerError) as error:
                row["unavailable"] = True
                row["error_kind"] = error.kind if isinstance(error, RunnerError) else type(error).__name__
                if isinstance(error, RunnerError) and error.details:
                    row["error_details"] = error.details
            finally:
                passed = sum(case["status"] == "PASS" for case in row["cases"])
                row["case_counts"].update(completed=len(row["cases"]), passed=passed,
                                          failed=len(row["cases"]) - passed)
                row["elapsed_ns"] = sum(case["elapsed_ns"] for case in row["cases"])
                row["performance_scope"] = PERFORMANCE_SCOPE
                remaining[problem] -= 1
                if discard_data and problem is not None and remaining[problem] == 0:
                    group = [value for value in report["results"] if problem_id(selected[value["id"]]) == problem]
                    try:
                        removed = discard_test_data(checkout, find_problem(checkout, problem))
                        for value in group:
                            value["test_data_cleanup"] = {"status": "PASS", "removed_directories": removed}
                        prepared.pop(problem, None)
                    except (OSError, ValueError, subprocess.SubprocessError, RunnerError) as error:
                        for value in group:
                            value.update(status="FAIL", unavailable=True,
                                         cleanup_error_kind=error.kind if isinstance(error, RunnerError) else type(error).__name__)
            report["counts"] = counts(report["results"], len(selected))
            write_report(output, report)
        report["status"] = "PASS" if all(row["status"] == "PASS" for row in report["results"]) else "FAIL"
    except (OSError, ValueError, subprocess.SubprocessError, RunnerError) as error:
        report["status"] = "FAIL"
        report["error_kind"] = error.kind if isinstance(error, RunnerError) else type(error).__name__
        if isinstance(error, RunnerError) and error.details:
            report["error_details"] = error.details
        present = {row["id"] for row in report["results"]}
        for identifier in plan["selected_ids"]:
            if identifier in present:
                continue
            row = {"id": identifier, "status": "FAIL", "unavailable": True,
                   "error_kind": report["error_kind"], "cases": [],
                   "case_counts": {"expected": 0, "completed": 0, "passed": 0, "failed": 0}}
            try:
                row.update(provenance(selected[identifier], root))
            except (OSError, ValueError, RunnerError):
                pass
            report["results"].append(row)
        report["counts"] = counts(report["results"], len(selected))
    finally:
        report["completed_at_utc"] = datetime.datetime.now(datetime.timezone.utc).isoformat()
        write_report(output, report)
    return report


@contextmanager
def cache_lock(cache: Path):
    with cache_path(cache, ".sirkit.lock").open("a") as lock:
        fcntl.flock(lock, fcntl.LOCK_EX)
        yield


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--base", default="origin/main")
    parser.add_argument("--id", action="append")
    parser.add_argument("--all", action="store_true")
    parser.add_argument("--shard-index", type=int, default=0)
    parser.add_argument("--shard-count", type=int, default=1)
    parser.add_argument("--compiler", default="g++")
    parser.add_argument("--cache", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True, help="metadataのresult.jsonだけを保存するディレクトリ")
    parser.add_argument("--case-timeout", type=float, default=30)
    parser.add_argument("--generation-timeout", type=float, default=1200)
    parser.add_argument("--discard-test-data", action="store_true",
                        help="同じ問題の全IDを終えたら未追跡in/outを削除する。CIの一時領域を抑える")
    args = parser.parse_args(argv)
    if args.all and args.id:
        parser.error("--allと--idは同時に指定できません")
    try:
        cache, output = external_cache(args.cache, args.output)
        if not all(math.isfinite(value) and value > 0 for value in (args.case_timeout, args.generation_timeout)):
            raise ValueError("timeoutは正の秒数を指定してください")
        pin = load_pin()
        entries = prepare_verify.collect()
        changed = [] if args.all or args.id else ci_verify.git_changed_paths(args.base)
        plan = selection(entries, changed, args.id, args.all, args.shard_index, args.shard_count)
        cache.mkdir(parents=True, exist_ok=True)
        cache, output = external_cache(cache, output)
        with cache_lock(cache):
            report = run_selected(entries, plan, cache, output, pin, args.compiler,
                                  case_timeout=args.case_timeout, generation_timeout=args.generation_timeout,
                                  discard_data=args.discard_test_data)
        print(f"Library Checker {report['status']}: {len(plan['selected_ids'])}件、結果: {output / 'result.json'}")
        return 0 if report["status"] in {"PASS", "SKIPPED"} else 1
    except (OSError, ValueError, subprocess.SubprocessError, RunnerError) as error:
        print(error.kind if isinstance(error, RunnerError) else str(error), file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
