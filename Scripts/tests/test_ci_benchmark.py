# SPDX-License-Identifier: CC0-1.0
from contextlib import redirect_stderr, redirect_stdout
from pathlib import Path
import argparse
import io
import json
import shutil
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import ci_benchmark as bench


def output(ns=100, checksum="42", expected="42"):
    return json.dumps({"elapsed_ns": ns, "checksum": checksum, "expected": expected}).encode()


class ProtocolTests(unittest.TestCase):
    def test_results_require_actual_time_and_independent_reference_equality(self):
        self.assertEqual(bench.parse_result(output())["elapsed_ns"], 100)
        for bad in [b"not-json", b"[]", output(ns=0), output(ns=-1), output(ns=True),
                    output(checksum="43"), output(checksum="-1", expected="-1"),
                    output(checksum=str(1 << 64), expected=str(1 << 64)),
                    b'{"elapsed_ns":1,"checksum":42,"expected":42}']:
            with self.subTest(bad=bad), self.assertRaises(bench.BenchmarkError):
                bench.parse_result(bad)

    def test_warmups_are_excluded_and_stage_order_alternates(self):
        calls = []
        def fake(args, **kwargs):
            calls.append(args)
            return subprocess.CompletedProcess(args, 0, output(ns=10 if args[0] == "base" else 20))
        with patch.object(bench, "command", side_effect=fake):
            row = bench.measure({"base": Path("base"), "current": Path("current")}, 1024, 0, 2, 3, 2)
        self.assertEqual(row["samples_ns"], {"base": [10, 10, 10], "current": [20, 20, 20]})
        self.assertEqual(row["current_over_base"], 2)
        self.assertEqual(row["orders"], [["base", "current"], ["current", "base"],
                                       ["base", "current"], ["current", "base"], ["base", "current"]])
        self.assertEqual(len(calls), 10)
        self.assertTrue(all(args[1:] == ["1024", "0", "2"] for args in calls))

    def test_reference_consistent_but_cross_version_different_output_is_failure(self):
        def fake(args, **kwargs):
            value = "1" if args[0] == "base" else "2"
            return subprocess.CompletedProcess(args, 0, output(checksum=value, expected=value))
        with patch.object(bench, "command", side_effect=fake), self.assertRaisesRegex(bench.BenchmarkError, "一致"):
            bench.measure({"base": Path("base"), "current": Path("current")}, 2, 0, 1, 1, 0)

    def test_missing_baseline_is_na_not_speedup(self):
        with patch.object(bench, "command", return_value=subprocess.CompletedProcess([], 0, output())):
            row = bench.measure({"current": Path("current")}, 2, 1, 1, 1, 0)
        self.assertEqual(row["status"], "NO_BASELINE")
        self.assertIsNone(row["current_over_base"])
        self.assertNotIn("base", row["median_ns"])
        row["id"] = "fenwick"
        md = bench.markdown({"status": "NO_BASELINE", "results": [row]})
        self.assertIn("N/A", md)
        self.assertNotIn("PASS", md)
        self.assertIn("比較未実施", md)


class GitAndFailureTests(unittest.TestCase):
    def test_ref_is_resolved_as_one_argument_without_shell(self):
        calls = []
        def fake(args, **kwargs):
            calls.append((args, kwargs))
            return subprocess.CompletedProcess(args, 0, b"a" * 40 + b"\n", b"")
        ref = "--help;$(touch SHOULD_NOT_EXIST)"
        with patch.object(bench.subprocess, "run", side_effect=fake):
            self.assertEqual(bench.resolve_base(ref), "a" * 40)
        self.assertEqual(calls[0][0][-2:], ["--end-of-options", ref + "^{commit}"])
        self.assertFalse(calls[0][1].get("shell", False))

    def test_missing_blob_and_git_failure_are_distinguished(self):
        with patch.object(bench, "command", return_value=subprocess.CompletedProcess([], 0, b"")) as run:
            self.assertIsNone(bench.baseline_source("a" * 40, "Math/missing.cpp"))
            self.assertEqual(run.call_count, 1)
        with patch.object(bench, "command", side_effect=bench.BenchmarkError("git failed")):
            with self.assertRaises(bench.BenchmarkError):
                bench.baseline_source("a" * 40, "Math/missing.cpp")

    def test_only_blob_content_is_requested_from_resolved_commit(self):
        calls = []
        def fake(args, **kwargs):
            calls.append(args)
            data = b"100644 blob " + b"b" * 40 + b"\tMath/A.cpp\0" if "ls-tree" in args else b"body"
            return subprocess.CompletedProcess(args, 0, data)
        with patch.object(bench, "command", side_effect=fake):
            self.assertEqual(bench.baseline_source("a" * 40, "Math/A.cpp"), b"body")
        self.assertEqual(calls[-1], ["git", "show", "a" * 40 + ":Math/A.cpp"])
        self.assertFalse(any("checkout" in args or "worktree" in args for args in calls))

    def test_invalid_base_overwrites_stale_pass_with_failure_report(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory)
            (path / "result.json").write_text('{"status":"PASS"}')
            args = argparse.Namespace(output=path, base="bad", compiler="g++", quick=True,
                                      repetitions=1, warmups=0, id=None)
            with patch.object(bench, "resolve_base", side_effect=bench.BenchmarkError("missing base")), redirect_stderr(io.StringIO()):
                self.assertEqual(bench.run(args), 1)
            data = json.loads((path / "result.json").read_text())
            self.assertEqual(data["status"], "FAIL")
            self.assertEqual(data["results"], [])
            self.assertNotIn("PASS", (path / "summary.md").read_text())

    def test_invalid_repetitions_are_rejected_before_execution(self):
        with redirect_stderr(io.StringIO()), self.assertRaises(SystemExit) as caught:
            bench.main(["--base", "HEAD", "--output", "/tmp/unused-benchmark-test", "--repetitions", "0"])
        self.assertEqual(caught.exception.code, 2)


class RuntimeTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.compiler = shutil.which("g++")
        if cls.compiler is None:
            raise unittest.SkipTest("g++ is needed for benchmark protocol smoke tests")

    def test_real_three_library_smoke_and_snapshots(self):
        with tempfile.TemporaryDirectory() as directory:
            args = argparse.Namespace(output=Path(directory), base="HEAD", compiler=self.compiler,
                                      quick=True, repetitions=1, warmups=0, id=None)
            with redirect_stdout(io.StringIO()):self.assertEqual(bench.run(args), 0)
            report = json.loads((Path(directory) / "result.json").read_text())
            self.assertEqual(report["status"], "PASS")
            self.assertEqual(len(report["results"]), 6)
            for source in report["sources"].values():
                folder = Path(directory) / source["snapshots"]
                self.assertEqual(bench.digest((folder / "current-library.hpp").read_bytes()), source["current_sha256"])
                self.assertEqual(bench.digest((folder / "base-library.hpp").read_bytes()), source["base_sha256"])
            self.assertTrue(all(row["independent_reference"] == "passed" for row in report["results"]))

    def test_all_base_blobs_missing_measures_current_but_does_not_pass(self):
        with tempfile.TemporaryDirectory() as directory:
            args = argparse.Namespace(output=Path(directory), base="HEAD", compiler=self.compiler,
                                      quick=True, repetitions=1, warmups=0, id=["fenwick"])
            with patch.object(bench, "baseline_source", return_value=None), redirect_stdout(io.StringIO()):
                self.assertEqual(bench.run(args), 2)
            report = json.loads((Path(directory) / "result.json").read_text())
            self.assertEqual(report["status"], "NO_BASELINE")
            self.assertEqual(report["compared_cases"], 0)
            self.assertTrue(report["current_measurements_passed"])
            self.assertEqual(len(report["results"]), 2)

    def test_real_wrong_kernel_is_detected_before_any_measurement(self):
        fixture = b'''#include "benchmark_common.hpp"
int main(int argc,char**argv) {
 return sirkit::detail::benchmark_main(argc,argv,
 [](std::size_t n,std::size_t){return n;},
 [](std::size_t){return std::uint64_t{1};},
 [](std::size_t){return std::uint64_t{2};});
}'''
        with tempfile.TemporaryDirectory() as directory:
            binary = bench.build(bench.compiler_command(self.compiler), b"", fixture,
                                 (bench.FIXTURES / "benchmark_common.hpp").read_bytes(), Path(directory), "bad")
            result = subprocess.run([str(binary), "2", "0", "1"], capture_output=True)
            self.assertNotEqual(result.returncode, 0)
            self.assertEqual(result.stdout, b"")
            self.assertIn(b"independent reference mismatch before timing", result.stderr)

    def test_standard_arithmetic_reference_and_library_path(self):
        with tempfile.TemporaryDirectory() as directory:
            compiler = [*bench.compiler_command(self.compiler), "-U__SIZEOF_INT128__",
                        "-DSIRKIT_FORCE_STANDARD_ARITHMETIC", "-DSIRKIT_DISABLE_SIMD"]
            binary = bench.build(compiler, (bench.ROOT / bench.LIBRARIES["rolling_hash"]).read_bytes(),
                                 (bench.FIXTURES / "rolling_hash.cpp").read_bytes(),
                                 (bench.FIXTURES / "benchmark_common.hpp").read_bytes(), Path(directory), "standard")
            for pattern in ("0", "1"):
                result = subprocess.run([str(binary), "256", pattern, "1"], capture_output=True, check=True)
                self.assertGreater(bench.parse_result(result.stdout)["elapsed_ns"], 0)

    def test_real_build_failure_is_not_a_measurement(self):
        with tempfile.TemporaryDirectory() as directory, self.assertRaises(bench.BenchmarkError):
            bench.build(bench.compiler_command(self.compiler), b"", b"#error deliberate-build-failure\n",
                        b"", Path(directory), "bad")


if __name__ == "__main__":
    unittest.main()
