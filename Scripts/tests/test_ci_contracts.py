# SPDX-License-Identifier: CC0-1.0
from copy import deepcopy
import json
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import ci_contracts as contracts


def result():
    return {"format": 1, "results": [
        {"library": path, "status": "PASS", "normal_checks": 3, "expected_failures": 1,
         "cases": [{"name": "invalid_point", "expected_exception": "std::out_of_range",
                    "status": "PASS", "post_failure_state_checked": True}]}
        for path in contracts.LIBRARIES]}


class ParseTests(unittest.TestCase):
    def test_all_libraries_require_positive_success_and_expected_failure_counts(self):
        self.assertEqual(len(contracts.parse_results(json.dumps(result()))), 6)
        for key in ("normal_checks", "expected_failures"):
            for value in (0, -1, True, "3", None):
                data = result()
                data["results"][0][key] = value
                with self.subTest(key=key, value=value), self.assertRaises(ValueError):
                    contracts.parse_results(json.dumps(data))

    def test_missing_duplicate_unknown_or_empty_rows_are_not_a_pass(self):
        variants = [[], result()["results"][:-1]]
        data = result()["results"]
        data[-1] = deepcopy(data[0])
        variants.append(data)
        data = result()["results"]
        data[0]["library"] = "../../outside.cpp"
        variants.append(data)
        for rows in variants:
            with self.subTest(rows=rows), self.assertRaises(ValueError):
                contracts.parse_results(json.dumps({"format": 1, "results": rows}))

    def test_expected_failure_needs_exact_exception_label_and_postcondition(self):
        for key, value in (("expected_exception", "SIGSEGV"), ("expected_exception", []),
                           ("post_failure_state_checked", False), ("post_failure_state_checked", 1),
                           ("status", "FAIL"), ("name", "x|<script>")):
            data = result()
            data["results"][0]["cases"][0][key] = value
            with self.subTest(key=key, value=value), self.assertRaises(ValueError):
                contracts.parse_results(json.dumps(data))
        data = result()
        data["results"][0]["cases"] = []
        with self.assertRaises(ValueError):
            contracts.parse_results(json.dumps(data))

    def test_duplicate_json_keys_and_non_objects_are_rejected(self):
        for text in ['{"format":1,"format":1,"results":[]}', '[]', 'null', '{"format":true}', 'NaN']:
            with self.subTest(text=text), self.assertRaises(ValueError):
                contracts.parse_results(text)


class RunnerTests(unittest.TestCase):
    def prepare_root(self, root):
        for path in (*contracts.LIBRARIES, contracts.FIXTURE):
            file = root / path
            file.parent.mkdir(parents=True, exist_ok=True)
            file.write_text("// owned fixture\n")

    def run_case(self, root, output, *, crash=False, compile_fail=False, mutate=False, fallback=False):
        calls = []
        def invoke(command, **kwargs):
            calls.append(command)
            if command[:3] == ["git", "rev-parse", "HEAD"]:
                return subprocess.CompletedProcess(command, 0, "a" * 40 + "\n", "")
            if command[-1] == "--version":
                return subprocess.CompletedProcess(command, 0, "g++ test version\n", "")
            if "-o" in command:
                return subprocess.CompletedProcess(command, 1 if compile_fail else 0, "", "compile failed" if compile_fail else "")
            if mutate:
                (root / contracts.LIBRARIES[0]).write_text("changed while running\n")
            return subprocess.CompletedProcess(command, -11 if crash else 0, json.dumps(result()), "segmentation fault" if crash else "")
        with patch.object(contracts, "compiler_command", return_value=["/usr/bin/g++"]), patch.object(contracts.subprocess, "run", side_effect=invoke):
            report = contracts.run(root=root, output=output, compiler="g++", fallback=fallback)
        return report, calls

    def test_checked_flags_source_hashes_and_ci_provenance_are_preserved(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory) / "root"
            self.prepare_root(root)
            with patch.dict(contracts.os.environ, {"GITHUB_RUN_ID": "123", "GITHUB_SHA": "b" * 40}):
                report, calls = self.run_case(root, Path(directory) / "out", fallback=True)
            self.assertEqual(report["status"], "PASS")
            self.assertEqual(report["source_hashes"], report["source_hashes_after"])
            self.assertEqual(set(report["source_hashes"]), {*contracts.LIBRARIES, contracts.FIXTURE})
            self.assertEqual(report["provenance"]["run_id"], "123")
            self.assertEqual(report["provenance"]["github_sha"], "b" * 40)
            command = next(command for command in calls if "-o" in command)
            for flag in ["-UNDEBUG", "-USKIP_RUNTIME_CHECKS", "-std=c++23", *contracts.FALLBACK_FLAGS]:
                self.assertIn(flag, command)
            self.assertNotIn("-DNDEBUG", command)
            self.assertNotIn("-DSKIP_RUNTIME_CHECKS", command)
            self.assertEqual(report["online_status"], "not_evaluated")

    def test_crash_is_failure_even_when_stdout_claims_pass(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory) / "root"
            self.prepare_root(root)
            report, _ = self.run_case(root, Path(directory) / "out", crash=True)
            self.assertEqual(report["status"], "FAIL")
            self.assertEqual(report["returncode"], -11)
            self.assertEqual(report["results"], [])
            self.assertIn("異常終了", report["error"])

    def test_source_change_during_execution_invalidates_all_results(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory) / "root"
            self.prepare_root(root)
            report, _ = self.run_case(root, Path(directory) / "out", mutate=True)
            self.assertEqual(report["status"], "FAIL")
            self.assertNotEqual(report["source_hashes"], report["source_hashes_after"])

    def test_compile_failure_overwrites_preexisting_pass(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory) / "root"
            self.prepare_root(root)
            output = Path(directory) / "out"
            output.mkdir()
            (output / "result.json").write_text('{"status":"PASS"}')
            report, calls = self.run_case(root, output, compile_fail=True)
            self.assertEqual(report["status"], "FAIL")
            self.assertEqual(json.loads((output / "result.json").read_text())["status"], "FAIL")
            self.assertEqual(report["results"], [])
            self.assertFalse(any(len(command) == 1 for command in calls))

    def test_repository_escape_via_symlink_is_rejected(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory) / "root"
            self.prepare_root(root)
            victim = root / contracts.LIBRARIES[0]
            victim.unlink()
            outside = Path(directory) / "outside.cpp"
            outside.write_text("outside")
            victim.symlink_to(outside)
            with self.assertRaises(ValueError):
                contracts.snapshot(root)

    def test_invalid_timeouts_fail_before_compilation(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory) / "root"
            self.prepare_root(root)
            for timeout in (0, -1, float("nan"), float("inf")):
                with self.subTest(timeout=timeout), patch.object(contracts, "provenance", return_value={}), patch.object(contracts, "compiler_command") as compiler:
                    report = contracts.run(root=root, output=Path(directory) / "out", compiler="g++", timeout=timeout)
                    self.assertEqual(report["status"], "FAIL")
                    compiler.assert_not_called()


@unittest.skipUnless(shutil.which("g++"), "GCC integration probes require g++")
class FixtureDetectionTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.work = tempfile.TemporaryDirectory(prefix="sirkit-contracts-self-test-")
        cls.addClassCleanup(cls.work.cleanup)
        cls.binary = Path(cls.work.name) / "contracts"
        cls.root = Path(__file__).resolve().parents[2]
        command = [shutil.which("g++"), "-std=c++23", "-O0", "-UNDEBUG", "-USKIP_RUNTIME_CHECKS",
                   "-I", str(cls.root), str(cls.root / contracts.FIXTURE), "-o", str(cls.binary)]
        # 既存の本体性能計測へコンパイル負荷を重ねない。
        cls.lock = open("/tmp/sirkit_performance_cpu12.lock", "a")
        cls.addClassCleanup(cls.lock.close)
        try:
            import fcntl
            fcntl.flock(cls.lock, fcntl.LOCK_EX)
        except ImportError:
            pass
        subprocess.run(command, check=True, capture_output=True, text=True, timeout=120)

    def test_wrong_exception_type_and_subtype_are_not_expected_failure_successes(self):
        for option in ("--probe-wrong-exception", "--probe-exception-subtype"):
            with self.subTest(option=option):
                result = subprocess.run([str(self.binary), option], capture_output=True, text=True, timeout=30)
                self.assertEqual(result.returncode, 1)
                self.assertEqual(result.stdout, "")
                self.assertIn("wrong exception", result.stderr)

    def test_disabled_checks_or_assertions_are_rejected_by_fixture(self):
        for macro in ("NDEBUG", "SKIP_RUNTIME_CHECKS"):
            with self.subTest(macro=macro):
                result = subprocess.run([shutil.which("g++"), "-std=c++23", "-E", "-D" + macro,
                                         "-I", str(self.root), str(self.root / contracts.FIXTURE)],
                                        stdout=subprocess.DEVNULL, stderr=subprocess.PIPE, text=True, timeout=60)
                self.assertNotEqual(result.returncode, 0)
                self.assertIn("contracts require runtime checks and assertions", result.stderr)


if __name__ == "__main__":
    unittest.main()
