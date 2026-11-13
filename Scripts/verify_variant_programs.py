#!/usr/bin/env python3
# SPDX-License-Identifier: CC0-1.0
"""凍結manifestの提出コード・使用例をローカル照合する。生成器やcollect()は呼ばない。"""
from __future__ import annotations

import argparse
import ast
from collections import Counter
from datetime import datetime, timezone
import fnmatch
import fcntl
import hashlib
import importlib.util
import inspect
import json
import os
from pathlib import Path
import re
import signal
import subprocess
import sys
import time

sys.dont_write_bytecode = True
ROOT = Path(__file__).resolve().parents[1]
PROFILES = ("Variants/GCC", "Variants/Clang", "Fast/GCC", "Fast/Clang")
COMPILERS = {"GCC": ["/usr/bin/g++"], "Clang": ["/usr/bin/clang++", "--gcc-install-dir=/usr/lib/gcc/x86_64-linux-gnu/15"]}


def sha(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def encode(value: object) -> bytes:
    return (json.dumps(value, ensure_ascii=False, sort_keys=True, indent=2) + "\n").encode()


def stamp() -> str:
    return datetime.now(timezone.utc).isoformat()


def write_json(path: Path, value: object) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary = path.with_name(path.name + ".tmp")
    temporary.write_bytes(encode(value))
    temporary.replace(path)


def matches_example(mode: str, expected: str, output: str) -> bool:
    actual, wanted = output.split(), expected.split()
    if len(actual) != len(wanted):
        return False
    for index, (left, right) in enumerate(zip(actual, wanted)):
        if left == right:
            continue
        # 第4行のmin_ld(0.5L,-0.25L)だけ。APIはゼロの符号を規定しない。
        if mode == "line_container_2d_signed_zero_at_6" and index == 6 and {left, right} == {"-0", "0"}:
            continue
        return False
    return True


class Runner:
    def __init__(self, options: argparse.Namespace):
        self.options = options
        self.directory = options.output_dir.resolve()
        self.directory.mkdir(parents=True, exist_ok=True)
        self.lock = (self.directory / "runner.lock").open("a+")
        try:
            fcntl.flock(self.lock.fileno(), fcntl.LOCK_EX | fcntl.LOCK_NB)
        except BlockingIOError:
            raise ValueError("同じoutput-dirのrunnerが実行中です: " + str(self.directory)) from None
        self.run_id = datetime.now(timezone.utc).strftime("%Y%m%dT%H%M%S%fZ")
        self.run_path = self.directory / "runs" / (self.run_id + ".json")
        self.report = {"format": 1, "run_id": self.run_id, "started_at_utc": stamp(),
                       "status": "snapshotting", "cpu": options.cpu,
                       "performance_measurement": False, "online_status": "not_evaluated",
                       "results": [], "active": None}
        self.checker = None
        self.custom_checker_source = inspect.getsource(matches_example).encode()
        self.report["custom_example_checker_sha256"] = sha(self.custom_checker_source)
        os.sched_setaffinity(0, {options.cpu})
        self.compilations = {}
        if options.resume:
            # 入力例やcheckerの更新は実行をやり直すが、同じソース/コンパイラの再ビルドは不要。
            for path in (self.directory / "results").glob("*.json"):
                try:
                    item = json.loads(path.read_text())
                    if item.get("compile", {}).get("returncode") != 0 or not item.get("binary_sha256"):
                        continue
                    recipe = self.compilation_recipe(item["cache_recipe"], item["source"])
                    binary = Path(item.get("binary_path", self.directory / "artifacts" / item["cache_key"] / "program"))
                    if binary.is_file():
                        self.compilations[sha(encode(recipe))] = (binary, item["binary_sha256"], item["compile"])
                except (OSError, ValueError, KeyError, TypeError):
                    continue
        self.save()

    @staticmethod
    def compilation_recipe(recipe: dict, source: str) -> dict:
        return {"format": 1, "source": source,
                **{key: recipe[key] for key in ("source_sha256", "compiler_command", "compiler_version", "flags", "compile_timeout")}}

    def save(self) -> None:
        self.report["updated_at_utc"] = stamp()
        self.report["counts"] = dict(Counter(item["status"] for item in self.report["results"]))
        write_json(self.run_path, self.report)
        write_json(self.directory / "checkpoint.json", self.report)

    def pause(self) -> None:
        pause = self.directory / "pause"
        acknowledgement = self.directory / "pause_ack"
        acknowledged = False
        while pause.exists():
            if not acknowledged:
                write_json(acknowledgement, {"pid": os.getpid(), "run_id": self.run_id,
                           "at_utc": stamp(), "state": "no_active_subprocess", "active": self.report["active"]})
                acknowledged = True
                print("PAUSED: " + str(acknowledgement), flush=True)
            time.sleep(0.25)
        acknowledgement.unlink(missing_ok=True)

    def execute(self, command: list[str], timeout: float, folder: Path,
                name: str, input_text: str | None = None) -> dict:
        self.pause()
        folder.mkdir(parents=True, exist_ok=True)
        stdout_path, stderr_path = folder / (name + ".stdout"), folder / (name + ".stderr")
        record = {"command": command, "stdout_path": str(stdout_path), "stderr_path": str(stderr_path)}
        environment = dict(os.environ, ASAN_OPTIONS="detect_leaks=0", LC_ALL="C", LANG="C")
        with stdout_path.open("wb") as stdout, stderr_path.open("wb") as stderr:
            try:
                process = subprocess.Popen(command, cwd=ROOT, stdin=subprocess.PIPE if input_text is not None else subprocess.DEVNULL,
                                           stdout=stdout, stderr=stderr, start_new_session=True, env=environment)
                try:
                    process.communicate(input_text.encode() if input_text is not None else None, timeout=timeout)
                    record.update(status="EXITED", returncode=process.returncode)
                except subprocess.TimeoutExpired:
                    os.killpg(process.pid, signal.SIGKILL)
                    process.communicate()
                    record.update(status="TIMEOUT", returncode=process.returncode, timeout_seconds=timeout)
                except BaseException:
                    if process.poll() is None:
                        os.killpg(process.pid, signal.SIGKILL)
                        process.communicate()
                    raise
            except OSError as error:
                record.update(status="SPAWN_ERROR", error=str(error), returncode=None)
        record["stdout"] = stdout_path.read_text(errors="replace")
        record["stderr"] = stderr_path.read_text(errors="replace")
        return record

    def source(self, data: bytes) -> Path:
        path = self.directory / "sources" / (sha(data) + ".cpp")
        if not path.exists():
            path.parent.mkdir(exist_ok=True)
            path.write_bytes(data)
        elif path.read_bytes() != data:
            raise ValueError("凍結ソースの不一致: " + str(path))
        return path

    def snapshot(self) -> tuple[list[dict], list[dict]]:
        paths = ("Variants/verify_manifest.json", "Variants/manifest.json", "Verify/current/manifest.json",
                 "Scripts/prepare_verify.py", "Scripts/verify_graph_samples.py", "Scripts/package_revised_libraries.py",
                 "Scripts/example_dependencies.py")
        frozen = {name: (ROOT / name).read_bytes() for name in paths}
        hashes = {name: sha(data) for name, data in frozen.items()}
        snapshot_id = sha(encode({"files": hashes, "custom_example_checker": sha(self.custom_checker_source)}))
        snapshot = self.directory / "snapshots" / snapshot_id
        for name, data in frozen.items():
            path = snapshot / name
            path.parent.mkdir(parents=True, exist_ok=True)
            if path.exists() and path.read_bytes() != data:
                raise ValueError("snapshotの不一致: " + str(path))
            path.write_bytes(data)
        checker_path = snapshot / "Scripts/prepare_verify.py"
        (snapshot / "Scripts/custom_example_checker.py").write_bytes(self.custom_checker_source)
        spec = importlib.util.spec_from_file_location("frozen_verify_" + snapshot_id, checker_path)
        self.checker = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(self.checker)
        # expand_aclだけがvendorを参照する。matchesは凍結した隣のcheckerを使う。
        self.checker.BASE = ROOT / "Verify/current"
        dependency_spec = importlib.util.spec_from_file_location(
            "frozen_example_dependencies_" + snapshot_id, snapshot / "Scripts/example_dependencies.py")
        dependency_module = importlib.util.module_from_spec(dependency_spec)
        dependency_spec.loader.exec_module(dependency_module)
        self.expand_example_dependencies = dependency_module.expand_example_dependencies
        self.report["example_expander_sha256"] = hashes["Scripts/example_dependencies.py"]
        self.checker_sha = sha(encode({name: hashes[name] for name in paths if name.endswith(("prepare_verify.py", "verify_graph_samples.py"))}))
        variants = json.loads(frozen[paths[0]])
        libraries = json.loads(frozen[paths[1]])
        normal = json.loads(frozen[paths[2]])
        entries = {entry["id"]: entry for entry in normal["entries"]}
        if len(entries) != len(normal["entries"]):
            raise ValueError("normal manifestのIDが重複しています")
        self.report.update(snapshot_id=snapshot_id, snapshot=str(snapshot), manifest_hashes=hashes,
                           snapshot_program_count=len(variants["entries"]), normal_entry_count=len(entries),
                           omitted_local=variants.get("omitted_local", []))
        programs = []
        seen = set()
        for item in variants["entries"]:
            profile = ("Fast/" if item["fast"] else "Variants/") + item["compiler"]
            identity = (profile, item["id"])
            if identity in seen:
                raise ValueError("variant manifestのIDが重複しています: " + str(identity))
            seen.add(identity)
            if item["id"] not in entries:
                raise ValueError("normal manifestにないID: " + item["id"])
            data = (ROOT / item["path"]).read_bytes()
            if sha(data) != item["sha256"]:
                raise ValueError("manifest読取中に提出ソースが変化しました: " + item["path"])
            path = self.source(data)
            if profile not in self.options.profiles or not self.selected(item["id"]):
                continue
            entry = entries[item["id"]]
            for setting in self.options.settings:
                programs.append(dict(kind="program", id=item["id"], profile=profile, compiler=item["compiler"],
                                     setting=setting, source=str(path), source_sha256=sha(data),
                                     original_path=item["path"], entry=entry, samples=entry.get("samples", [])))
        mapping = {}
        for node in ast.parse(frozen[paths[5]].decode()).body:
            if isinstance(node, ast.Assign) and any(isinstance(target, ast.Name) and target.id == "LIBRARIES" for target in node.targets):
                mapping = ast.literal_eval(node.value)
        if not mapping:
            raise ValueError("packageのExamples対応を読み取れません")
        source_map = {(("Fast/" if x["fast"] else "Variants/") + x["compiler"], x["source"]): x for x in libraries["entries"]}
        known_libraries = {library for _, library in source_map}
        normal_dependencies = {}
        profile_dependencies = {}
        dependency_hashes = {}

        def load_normal_dependency(library):
            if library not in normal_dependencies:
                data = (ROOT / library).read_bytes()
                expected = {entry["source_hashes"][library] for entry in entries.values()
                            if library in entry.get("source_hashes", {})}
                if len(expected) != 1 or sha(data) not in expected:
                    raise ValueError("normal manifestとExample依存が一致しません: " + library)
                normal_dependencies[library] = data.decode()
                dependency_hashes["Normal/" + library] = sha(data)
            return normal_dependencies[library]

        def load_profile_dependency(profile, library):
            key = (profile, library)
            if key not in profile_dependencies:
                item = source_map.get(key)
                if item is None:
                    raise ValueError("配布manifestにExample依存がありません: " + profile + "/" + library)
                data = (ROOT / item["path"]).read_bytes()
                if sha(data) != item["sha256"]:
                    raise ValueError("manifest読取中に依存が変化しました: " + item["path"])
                profile_dependencies[key] = data.decode()
                dependency_hashes[profile + "/" + library] = sha(data)
            return profile_dependencies[key]

        examples = []
        if self.options.kind != "programs":
            for example, library in mapping.items():
                identity = "example:" + Path(example).stem
                if not self.selected(identity):
                    continue
                raw = (ROOT / "Examples" / example).read_text()
                example_body = re.sub(r"//[^\n]*", "", raw)
                reads_input = bool(re.search(r"\b(?:cin|wcin|getline|scanf|fscanf|fread|getchar|getc|fgets|stdin)\b|\bread\s*\(", example_body))
                sample_id = Path(example).stem.removesuffix("_example")
                input_samples = [{"input": ""}]
                input_origin = "no_stdin_reader_detected"
                if reads_input:
                    if sample_id not in entries or not entries[sample_id].get("samples"):
                        raise ValueError("stdinを読むExampleの有効入力を明示してください: " + example)
                    input_samples = [{"input": sample["input"]} for sample in entries[sample_id]["samples"]]
                    if not any(sample["input"].strip() for sample in input_samples):
                        raise ValueError("stdinを読むExampleに空でない入力がありません: " + example)
                    input_origin = "Verify/current/manifest.json:" + sample_id
                include = '#include "../' + library + '"'
                if raw.count(include) != 1:
                    raise ValueError("Exampleのincludeが一意でありません: " + example)
                base = self.checker.expand_acl(self.expand_example_dependencies(
                    raw, "Examples/" + example, known_libraries, load_normal_dependency)).encode()
                base_path = self.source(base)
                for profile in self.options.profiles:
                    compiler = profile.rsplit("/", 1)[1]

                    adapted = raw
                    node_adapter = profile.startswith("Fast/") and library == "DataStructure/Node.cpp"
                    if node_adapter:
                        adapted = adapted.replace("entry.get()", "entry").replace("所有は残る", "参照は残る").replace(
                            "共有所有の循環を切ってから解放する", "循環を切る。メモリは解放しない")
                    code = self.checker.expand_acl(self.expand_example_dependencies(
                        adapted, "Examples/" + example, known_libraries,
                        lambda dependency: load_profile_dependency(profile, dependency))).encode()
                    code_path = self.source(code)
                    body = re.sub(r"//[^\n]*", "", raw)
                    excluded = profile.startswith("Fast/") and bool(re.search(r"\bcatch\s*\(|\bthrow\b", body))
                    for setting in self.options.settings:
                        baseline = dict(kind="example_baseline", id=identity, profile="Normal/" + compiler, compiler=compiler,
                                        setting=setting, source=str(base_path), source_sha256=sha(base), samples=input_samples)
                        examples.append(dict(kind="example", id=identity, profile=profile, compiler=compiler, setting=setting,
                                             source=str(code_path), source_sha256=sha(code), original_path="Examples/" + example,
                                             example_sha256=sha(raw.encode()), library=library, baseline=baseline,
                                             node_adapter=node_adapter, excluded_contract=excluded,
                                             input_samples=input_samples, input_origin=input_origin,
                                             example_checker="line_container_2d_signed_zero_at_6" if example == "line_container_2d_example.cpp" else "tokens"))
        self.report["snapshot_example_dependencies"] = dependency_hashes
        self.report["snapshot_example_mapping"] = mapping
        self.report["planned_program_conditions"] = len(programs) if self.options.kind != "examples" else 0
        self.report["planned_example_conditions"] = len(examples)
        # pause中にrootが再生成しても、ここまでに全てのコードを不変のSHAファイルへ保存する。
        write_json(snapshot / "selected_jobs.json", {"programs": programs, "examples": examples})
        self.versions = {}
        for compiler, prefix in COMPILERS.items():
            result = self.execute([prefix[0], "--version"], 30, snapshot / "compiler_versions", compiler)
            if result["status"] != "EXITED" or result["returncode"] != 0:
                raise ValueError("コンパイラのバージョン取得に失敗: " + compiler)
            self.versions[compiler] = result["stdout"]
        self.report["compiler_versions"] = self.versions
        self.report["status"] = "running"
        self.save()
        return ([] if self.options.kind == "examples" else programs), examples

    def selected(self, identity: str) -> bool:
        return not self.options.ids or any(fnmatch.fnmatchcase(identity, pattern) for pattern in self.options.ids)

    def flags(self, job: dict) -> list[str]:
        return ["-std=c++23", "-O2", "-Wall", "-Wextra"] + (["-march=native"] if job["setting"] == "native" else [])

    def validate(self, job: dict, expected: str | list[dict] | None = None) -> dict:
        self.pause()
        entry = job.get("entry", {})
        # 既知のonline記録は照合にも結果の判定にも使わない。
        semantic_entry = {key: value for key, value in entry.items() if key not in {"judge_record", "judge_status", "submission_urls"}}
        samples = job.get("samples", expected if isinstance(expected, list) else [{"input": "", "output": expected or ""}])
        matcher = "prepare_verify.matches"
        if job["kind"] == "example_baseline":
            matcher = "exit_only"
        elif job["kind"] == "example":
            mode = job.get("example_checker", "tokens")
            matcher = "normal_example_tokens" if mode == "tokens" else "normal_example_" + mode
        checker_digest = self.checker_sha
        if job["kind"] == "example" and job.get("example_checker", "tokens") != "tokens":
            checker_digest = sha(encode({"prepare_verify": self.checker_sha,
                                        "custom_example": sha(self.custom_checker_source)}))
        key_data = dict(contract_version=1, kind=job["kind"], id=job["id"], profile=job["profile"],
                        source_sha256=job["source_sha256"], compiler_command=COMPILERS[job["compiler"]],
                        compiler_version=self.versions[job["compiler"]], flags=self.flags(job),
                        samples_sha256=sha(encode(samples)), entry_sha256=sha(encode(semantic_entry)),
                        compile_timeout=self.options.compile_timeout, run_timeout=self.options.run_timeout,
                        checker_sha256=checker_digest, matcher=matcher)
        key = sha(encode(key_data))
        result_path = self.directory / "results" / (key + ".json")
        prior = json.loads(result_path.read_text()) if self.options.resume and result_path.is_file() else None
        if prior and prior.get("status") == "LOCAL_SAMPLE_PASS":
            return dict(prior, cached=True)
        artifact = self.directory / "artifacts" / key
        binary = Path(prior.get("binary_path", artifact / "program")) if prior else artifact / "program"
        compilation_key = sha(encode(self.compilation_recipe(key_data, job["source"])))
        result = {**{name: job[name] for name in ("kind", "id", "profile", "setting", "source_sha256")},
                  "cache_key": key, "cache_recipe": key_data, "run_id": self.run_id, "started_at_utc": stamp(),
                  "status": "RUNNING", "online_status": "not_evaluated", "cached": False,
                  "original_path": job.get("original_path"), "source": job["source"], "samples": []}
        result["compilation_cache_key"] = compilation_key
        result["compile_reused"] = False
        if job["kind"] == "example":
            result["input_origin"] = job.get("input_origin")
            result["example_checker"] = job.get("example_checker", "tokens")
        if job.get("excluded_contract"):
            result.update(status="EXCLUDED_FAST_CONTRACT", reason="Exampleにthrow/catchを含むため、Fastの有効入力契約を人が確認するまで実行しない")
            self.persist(result_path, result)
            return result
        self.report["active"] = {"cache_key": key, "id": job["id"], "profile": job["profile"], "setting": job["setting"]}
        self.save()
        compile_cached = prior and prior.get("compile", {}).get("returncode") == 0 and binary.is_file() and sha(binary.read_bytes()) == prior.get("binary_sha256")
        if compile_cached:
            result["compile"] = prior["compile"]
            result["binary_sha256"] = prior["binary_sha256"]
            result["compile_reused"] = True
        else:
            cached_build = self.compilations.get(compilation_key) if self.options.resume else None
            if cached_build and cached_build[0].is_file() and sha(cached_build[0].read_bytes()) == cached_build[1]:
                binary, result["binary_sha256"], result["compile"] = cached_build
                result["compile_reused"] = True
            else:
                binary = artifact / "program"
                command = COMPILERS[job["compiler"]] + self.flags(job) + [job["source"], "-o", str(binary)]
                result["compile"] = self.execute(command, self.options.compile_timeout, artifact / self.run_id, "compile")
                if result["compile"]["status"] != "EXITED" or result["compile"]["returncode"] != 0:
                    result["status"] = "COMPILE_" + ("TIMEOUT" if result["compile"]["status"] == "TIMEOUT" else "FAIL")
                    self.persist(result_path, result)
                    return result
                result["binary_sha256"] = sha(binary.read_bytes())
        result["binary_path"] = str(binary)
        self.compilations[compilation_key] = (binary, result["binary_sha256"], result["compile"])
        self.persist(result_path, result)
        for index, sample in enumerate(samples):
            if compile_cached and index < len(prior.get("samples", [])) and prior["samples"][index].get("passed"):
                result["samples"].append(prior["samples"][index])
                continue
            execution = self.execute([str(binary)], self.options.run_timeout, artifact / self.run_id, "sample_" + str(index), sample["input"])
            passed = execution["status"] == "EXITED" and execution["returncode"] == 0
            if passed:
                try:
                    if job["kind"] == "program":
                        passed = self.checker.matches(entry, sample, execution["stdout"])
                    elif job["kind"] == "example":
                        mode = job.get("example_checker", "tokens")
                        if mode == "tokens":
                            passed = self.checker.matches({"sample_checker": "tokens"}, sample, execution["stdout"])
                        else:
                            passed = matches_example(mode, sample["output"], execution["stdout"])
                except Exception as error:
                    execution["checker_error"] = type(error).__name__ + ": " + str(error)
                    passed = False
            execution.update(passed=passed, sample_sha256=sha(encode(sample)), input=sample["input"],
                             expected_output=sample.get("output"), sample_checker=job.get("example_checker", entry.get("sample_checker", "tokens")))
            result["samples"].append(execution)
            self.persist(result_path, result)
        result["status"] = "LOCAL_SAMPLE_PASS" if samples and all(item["passed"] for item in result["samples"]) else "SAMPLE_FAIL"
        self.persist(result_path, result)
        return result

    def persist(self, path: Path, result: dict) -> None:
        result["updated_at_utc"] = stamp()
        write_json(path, result)
        write_json(self.directory / "attempts" / result["cache_key"] / (self.run_id + ".json"), result)

    def record(self, result: dict) -> None:
        row = {name: result[name] for name in ("kind", "id", "profile", "setting", "cache_key", "status", "cached")}
        self.report["results"].append(row)
        self.report["active"] = None
        self.save()
        if result["status"] not in {"LOCAL_SAMPLE_PASS", "EXCLUDED_FAST_CONTRACT"}:
            print("FAIL: " + json.dumps(row, ensure_ascii=False) + " " + str(self.directory / "results" / (result["cache_key"] + ".json")), flush=True)
            if self.options.fail_fast:
                raise RuntimeError("失敗した検査があります")
        elif len(self.report["results"]) % 25 == 0:
            print(str(len(self.report["results"])) + " conditions: " + json.dumps(self.report["counts"], ensure_ascii=False), flush=True)

    def run(self) -> int:
        try:
            programs, examples = self.snapshot()
            if not programs and not examples:
                raise ValueError("指定に一致する対象がありません")
            for job in programs:
                self.record(self.validate(job))
            baselines = {}
            for job in examples:
                baseline = job["baseline"]
                identity = (baseline["id"], baseline["compiler"], baseline["setting"], baseline["source_sha256"])
                if identity not in baselines:
                    verified = self.validate(baseline)
                    self.record(verified)
                    baselines[identity] = verified
                verified = baselines[identity]
                if verified["status"] != "LOCAL_SAMPLE_PASS":
                    key = sha(encode({"dependency": verified["cache_key"], "source": job["source_sha256"],
                                      "profile": job["profile"], "setting": job["setting"]}))
                    blocked = {**{name: job[name] for name in ("kind", "id", "profile", "setting")},
                               "cache_key": key, "status": "EXAMPLE_BASELINE_FAILED", "cached": False,
                               "run_id": self.run_id, "baseline_cache_key": verified["cache_key"]}
                    self.persist(self.directory / "results" / (key + ".json"), blocked)
                    self.record(blocked)
                    continue
                expected = [{"input": sample["input"], "output": checked["stdout"]}
                            for sample, checked in zip(job["input_samples"], verified["samples"])]
                self.record(self.validate(job, expected))
            drift = {name: sha((ROOT / name).read_bytes()) for name, digest in self.report["manifest_hashes"].items()
                     if sha((ROOT / name).read_bytes()) != digest}
            self.report["workspace_snapshot_drift"] = drift
            failures = [x for x in self.report["results"] if x["status"] not in {"LOCAL_SAMPLE_PASS", "EXCLUDED_FAST_CONTRACT"}]
            excluded = any(x["status"] == "EXCLUDED_FAST_CONTRACT" for x in self.report["results"])
            self.report["status"] = "failed_for_snapshot" if failures else (
                "local_samples_passed_with_exclusions_for_snapshot" if excluded else "local_samples_passed_for_snapshot")
            self.report["completed_at_utc"] = stamp()
            self.save()
            print(self.report["status"] + ": " + str(self.run_path), flush=True)
            return 1 if failures else 0
        except (Exception, KeyboardInterrupt) as error:
            self.report.update(status="interrupted" if isinstance(error, KeyboardInterrupt) else "runner_error", error=type(error).__name__ + ": " + str(error))
            self.save()
            print(self.report["error"], file=sys.stderr, flush=True)
            return 2
        finally:
            (self.directory / "pause_ack").unlink(missing_ok=True)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output-dir", type=Path, required=True)
    parser.add_argument("--resume", action="store_true", help="同じcode/flags/compiler/sample/checkerの成功結果と途中のコンパイルを再利用")
    parser.add_argument("--id", dest="ids", action="append", help="IDのglob。使用例はexample:node_example等")
    parser.add_argument("--profile", action="append", choices=PROFILES)
    parser.add_argument("--setting", action="append", choices=("default", "native"))
    parser.add_argument("--kind", choices=("all", "programs", "examples"), default="all")
    parser.add_argument("--cpu", type=int, default=0)
    parser.add_argument("--compile-timeout", type=float, default=180)
    parser.add_argument("--run-timeout", type=float, default=60)
    parser.add_argument("--fail-fast", action="store_true")
    options = parser.parse_args()
    options.profiles = options.profile or list(PROFILES)
    options.settings = options.setting or ["default", "native"]
    return Runner(options).run()


if __name__ == "__main__":
    raise SystemExit(main())
