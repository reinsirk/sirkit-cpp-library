# SPDX-License-Identifier: CC0-1.0
from copy import deepcopy
import json
from pathlib import Path
import sys
import tempfile
import unittest
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import ci_quality_report as quality


class QualityTests(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory()
        self.root = Path(self.temporary.name)
        self.library = "DataStructure/A.cpp"
        self.write(self.library, "namespace sirkit {}\n// fixture\n")
        self.write("Graph/B.cpp", "namespace sirkit {}\n")
        self.driver = "Verify/current/drivers/alpha_main.cpp"
        self.write(self.driver, "int main(){return 0;}\n")
        self.entry = {"id": "alpha", "libraries": [self.library], "driver": self.driver,
                      "submission": "Verify/current/submissions/alpha.cpp", "samples": [{"input": "", "output": ""}],
                      "source_hashes": {self.library: self.hash(self.library)}, "driver_sha256": self.hash(self.driver),
                      "support_source_hashes": {}}
        with quality.verify_root(self.root):
            source, _ = quality.prepare_verify.render(self.entry)
        self.write(self.entry["submission"], source)
        self.entry["submission_sha256"] = quality.digest(source)
        self.write("Verify/current/manifest.json", {"format": 1, "entries": [self.entry]})
        self.inputs = self.root / "artifacts"
        self.inputs.mkdir()

    def tearDown(self):
        self.temporary.cleanup()

    def write(self, name, value):
        path = self.root / name
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_bytes(value if isinstance(value, bytes) else value.encode() if isinstance(value, str) else json.dumps(value).encode())
        return path

    def hash(self, name):
        return quality.digest((self.root / name).read_bytes())

    def report_file(self, value, name="one/result.json"):
        path = self.inputs / name
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(json.dumps(value))
        return path

    def aggregate(self, value, name="one/result.json"):
        return quality.aggregate(self.root, [self.report_file(value, name)])

    def category(self, report, category, library=None):
        row = next(row for row in report["libraries"] if row["path"] == (library or self.library))
        return row["categories"][category]

    def normal(self):
        rows = []
        for skip in (False, True):
            with quality.verify_root(self.root):
                source, _ = quality.prepare_verify.render(self.entry, skip_runtime_checks=skip)
            rows.append({"id": "alpha", "compiler": "g++", "standard_fallback": False,
                         "skip_runtime_checks": skip, "submission_sha256": self.entry["submission_sha256"],
                         "tested_source_sha256": quality.digest(source), "compile_ok": True,
                         "command": ["g++", "-std=c++23", "-O2", "fixture.cpp"],
                         "passed": True, "samples": [{"passed": True, "returncode": 0}]})
        return {"format": 1, "status": "PASS", "online_status": "not_evaluated", "compiler": "g++",
                "standard_fallback": False, "plan": {"selected_ids": ["alpha"]}, "results": rows}

    def contract(self):
        driver = "Verify/ci/contracts.cpp"
        self.write(driver, "// checked contract fixture\n")
        source = {self.library: self.hash(self.library), driver: self.hash(driver)}
        return {"format": 1, "type": "sirkit.contracts", "status": "PASS", "checked": True,
                "online_status": "not_evaluated", "compile_returncode": 0, "returncode": 0,
                "source_hashes": source, "source_hashes_after": deepcopy(source),
                "command": ["g++", "-std=c++23", "-O2", "-UNDEBUG", "-USKIP_RUNTIME_CHECKS", driver],
                "compiler": {"command": ["g++"], "version": "test compiler"},
                "results": [{"library": self.library, "status": "PASS", "normal_checks": 3,
                             "expected_failures": 1, "cases": [{"name": "out_of_range",
                              "expected_exception": "std::out_of_range", "status": "PASS",
                              "post_failure_state_checked": True}]}]}

    def performance(self):
        fixture = "Verify/ci/benchmarks/alpha.cpp"
        common = "Verify/ci/benchmarks/benchmark_common.hpp"
        self.write(fixture, "// benchmark\n"); self.write(common, "// harness\n")
        return {"format": 1, "scope": "representative_normal_library_kernels", "status": "PASS",
                "online_status": "not_evaluated", "flags": ["-std=c++23", "-O2", "-DNDEBUG", "-DSKIP_RUNTIME_CHECKS"],
                "repetitions": 3, "warmups": 1, "base_revision": "a" * 40,
                "compiler": {"command": ["g++"], "version": "test GCC"}, "cpu_model": "unit CPU",
                "current_measurements_passed": True, "compared_cases": 1, "uncompared_cases": 0,
                "sources": {"alpha": {"path": self.library, "current_sha256": self.hash(self.library),
                             "base_sha256": "a" * 64, "harness_sha256": self.hash(fixture),
                             "common_harness_sha256": self.hash(common)}},
                "results": [{"id": "alpha", "status": "MEASURED", "size": 1024, "pattern": "random",
                             "rounds": 1, "samples_ns": {"base": [100, 101, 102], "current": [200, 201, 202]},
                             "median_ns": {"base": 101, "current": 201}, "current_over_base": 201/101,
                             "checksum": "42", "independent_reference": "passed"}]}

    def instrumented(self, mode="sanitizers"):
        hashes = {self.library: self.hash(self.library), self.driver: self.hash(self.driver)}
        flags = ["-fsanitize=address,undefined"] if mode == "sanitizers" else ["-fprofile-instr-generate", "-fcoverage-mapping"]
        return {"format": 1, "mode": mode, "checked": True, "status": "PASS", "online_status": "not_evaluated",
                "all_source_hashes": hashes, "source_hashes_after": deepcopy(hashes),
                "library_source_hashes": {self.library: hashes[self.library]}, "vendor_source_hashes": {},
                "selected_ids": ["alpha"], "results": [{"id": "alpha", "status": "PASS", "compile_returncode": 0,
                 "driver_sha256": hashes[self.driver], "command": ["clang++", "-UNDEBUG", "-USKIP_RUNTIME_CHECKS", *flags],
                 "samples": [{"passed": True, "returncode": 0}]}]}

    def test_missing_reports_and_unmeasured_libraries_never_pass(self):
        report = quality.aggregate(self.root, [])
        self.assertEqual(report["status"], "UNAVAILABLE")
        self.assertEqual(self.category(report, "normal")["status"], "UNAVAILABLE")
        report = self.aggregate(self.contract())
        self.assertEqual(report["status"], "PARTIAL")
        self.assertEqual(report["input_report_status"], "PASS")
        self.assertEqual(self.category(report, "failure")["status"], "PASS")
        self.assertEqual(self.category(report, "normal", "Graph/B.cpp")["status"], "UNAVAILABLE")

    def test_normal_requires_both_variants_actual_generated_sha_and_driver(self):
        report = self.aggregate(self.normal())
        self.assertEqual(self.category(report, "normal")["status"], "PASS")
        bad = self.normal(); bad["results"].pop()
        self.assertEqual(self.aggregate(bad)["status"], "FAIL")
        bad = self.normal(); bad["results"][0]["tested_source_sha256"] = "f" * 64
        self.assertEqual(self.category(self.aggregate(bad), "normal")["status"], "STALE")
        value = self.normal(); self.write(self.driver, "int main(){return 1;}\n")
        self.assertEqual(self.category(self.aggregate(value), "normal")["status"], "STALE")

    def test_normal_duplicate_compile_failure_and_bad_sample(self):
        for mutation in (lambda v: v["results"].append(deepcopy(v["results"][0])),
                         lambda v: v["results"][0].update(compile_ok="true"),
                         lambda v: v["results"][0]["samples"][0].update(returncode=1),
                         lambda v: v.update(online_status="AC"),
                         lambda v: v["results"][0]["command"].append("-DNDEBUG")):
            value = self.normal(); mutation(value)
            self.assertEqual(self.aggregate(value)["status"], "FAIL")

    def test_contract_failure_requires_exception_type_and_post_failure_check(self):
        for mutation in (lambda v: v["results"][0].update(expected_failures=0),
                         lambda v: v["results"][0].update(cases=[]),
                         lambda v: v["results"][0]["cases"][0].update(expected_exception="segfault"),
                         lambda v: v["results"][0]["cases"][0].update(post_failure_state_checked=False),
                         lambda v: v["command"].append("-DNDEBUG"),
                         lambda v: v["command"].extend(["-D", "SKIP_RUNTIME_CHECKS"])):
            value = self.contract(); mutation(value)
            self.assertEqual(self.aggregate(value)["status"], "FAIL")

    def test_contract_crash_or_compile_failure_not_expected_failure_success(self):
        for rc in (-11, 1):
            value = self.contract(); value.update(status="FAIL", returncode=rc, results=[])
            report = self.aggregate(value)
            self.assertEqual(report["status"], "FAIL")
            self.assertEqual(self.category(report, "failure")["status"], "FAIL")
            self.assertEqual(self.category(report, "failure")["evidence"][0]["checks"], 0)
        value = self.contract(); value["returncode"] = -11
        self.assertEqual(self.aggregate(value)["status"], "FAIL")

    def test_stale_driver_library_or_changed_midrun_never_pass(self):
        value = self.contract(); self.write(self.library, "// edited\n")
        self.assertEqual(self.category(self.aggregate(value), "failure")["status"], "STALE")
        value = self.contract(); value["source_hashes_after"][self.library] = "0" * 64
        self.assertEqual(self.aggregate(value)["status"], "FAIL")

    def test_performance_is_advisory_and_missing_base_not_speedup(self):
        report = self.aggregate(self.performance())
        data = self.category(report, "performance")
        self.assertEqual(data["status"], "PASS")
        self.assertGreater(data["evidence"][0]["current_over_base"], 1)
        self.assertTrue(data["evidence"][0]["advisory"])
        value = self.performance(); value.update(status="NO_BASELINE", compared_cases=0, uncompared_cases=1)
        row = value["results"][0]; row.update(status="NO_BASELINE", current_over_base=None)
        del row["samples_ns"]["base"]; del row["median_ns"]["base"]
        report = self.aggregate(value)
        self.assertEqual(self.category(report, "performance")["evidence"][0]["comparison"], "NO_BASELINE")
        self.assertIn("比較元なし", quality.markdown(report))

    def test_performance_recomputes_medians_ratios_and_counts(self):
        for mutation in (lambda v: v["results"][0]["median_ns"].update(current=1),
                         lambda v: v["results"][0].update(current_over_base=0.1),
                         lambda v: v.update(compared_cases=0),
                         lambda v: v["results"][0]["samples_ns"]["current"].append(5),
                         lambda v: v["results"][0].update(checksum="-1"),
                         lambda v: v["results"][0].update(independent_reference="unverified")):
            value = self.performance(); mutation(value)
            self.assertEqual(self.aggregate(value)["status"], "FAIL")
        value = self.performance(); self.write("Verify/ci/benchmarks/alpha.cpp", "// changed harness\n")
        self.assertEqual(self.category(self.aggregate(value), "performance")["status"], "STALE")

    def test_instrumented_requires_checked_flags_full_hashes_and_nonempty_samples(self):
        self.assertEqual(self.category(self.aggregate(self.instrumented()), "sanitizers")["status"], "PASS")
        for mutation in (lambda v: v.update(checked=False),
                         lambda v: v["results"][0].update(samples=[]),
                         lambda v: v["results"][0]["command"].remove("-fsanitize=address,undefined"),
                         lambda v: v["all_source_hashes"].pop(self.driver)):
            value = self.instrumented(); mutation(value)
            self.assertEqual(self.aggregate(value)["status"], "FAIL")

    def test_coverage_uses_per_source_metrics_and_missing_data_unavailable(self):
        value = self.instrumented("coverage")
        report = self.aggregate(value)
        self.assertEqual(self.category(report, "coverage")["status"], "UNAVAILABLE")
        path = self.inputs / "one/coverage.json"
        metric = {"count": 2, "covered": 1, "percent": 50}
        path.write_text(json.dumps({"format": 1, "type": "sirkit.source-coverage-union", "files": [
            {"filename": "/old/runner/checkout/" + self.library, "lines": [[1, True], [2, False]],
             "branches": [[1, 1, 1, 5, True, False]], "summary": {"lines": metric, "branches": metric}}]}))
        data = self.category(self.aggregate(value), "coverage")
        self.assertEqual(data["status"], "PASS")
        self.assertEqual(data["evidence"][0]["metrics"]["lines"]["percent"], 50)

    def official(self):
        self.entry["problem_url"] = "https://judge.yosupo.jp/problem/aplusb"
        self.write("Verify/current/manifest.json", {"format": 1, "entries": [self.entry]})
        official = {"repository": "https://github.com/yosupo06/library-checker-problems", "revision": "a"*40}
        detail = dict(official, problem_id="aplusb", info_sha256="b"*64,
                      hash_json_sha256="c"*64, checker_sha256="d"*64,
                      expected_cases=["example_00", "random_00"])
        rows = [{"name": name, "status": "PASS", "elapsed_ns": 100,
                 "returncode": 0, "checker_returncode": 0}for name in detail["expected_cases"]]
        row = {"id": "alpha", "status": "PASS", "libraries": [self.library],
               "source_hashes": {self.library: self.hash(self.library)}, "driver": self.driver,
               "driver_sha256": self.hash(self.driver), "submission": self.entry["submission"],
               "submission_sha256": self.entry["submission_sha256"], "support_source_hashes": {},
               "io_source_hashes": {}, "official": detail, "cases": rows,
               "case_counts": {"expected": 2, "completed": 2, "passed": 2, "failed": 0},
               "elapsed_ns": 200, "compile_command": ["g++", "-std=c++23", "fixture.cpp"],
               "performance_scope": "whole-program wall time, checker excluded"}
        return {"format": 1, "type": "sirkit.library-checker", "status": "PASS",
                "online_status": "not_evaluated", "official": official,
                "selection": {"selected_ids": ["alpha"]}, "results": [row],
                "compiler": {"command": ["g++"], "version": "test GCC", "flags": ["-std=c++23", "-O2"]}}

    def test_official_cases_are_local_not_online_or_kernel_performance(self):
        report = self.aggregate(self.official())
        category = self.category(report, "library_checker")
        self.assertEqual(category["status"], "PASS")
        self.assertEqual(category["evidence"][0]["case_counts"]["passed"], 2)
        self.assertEqual(category["evidence"][0]["elapsed_ns"], 200)
        self.assertEqual(self.category(report, "performance")["status"], "UNAVAILABLE")
        self.assertEqual(report["online_status"], "not_evaluated")
        self.assertIn("本体だけの性能測定とは分けます", quality.markdown(report))

    def test_official_missing_duplicate_or_bad_checker_case_cannot_pass(self):
        for mutation in (lambda v: v["results"][0]["cases"].pop(),
                         lambda v: v["results"][0]["cases"].append(deepcopy(v["results"][0]["cases"][0])),
                         lambda v: v["results"][0]["cases"][0].update(checker_returncode=1),
                         lambda v: v["results"][0]["cases"][0].pop("elapsed_ns"),
                         lambda v: v["results"][0]["official"].update(checker_sha256=None),
                         lambda v: v["official"].update(revision="main"),
                         lambda v: v["official"].update(repository="https://example.invalid/problems")):
            value = self.official(); mutation(value)
            self.assertEqual(self.aggregate(value)["status"], "FAIL")

    def test_official_source_changed_and_empty_shard_are_explicit(self):
        value = self.official(); self.write(self.library, "// changed\n")
        self.assertEqual(self.category(self.aggregate(value), "library_checker")["status"], "STALE")
        value = self.official(); value.update(status="SKIPPED", results=[])
        value["selection"]["selected_ids"] = []
        report = self.aggregate(value)
        self.assertEqual(report["status"], "PARTIAL")
        self.assertEqual(self.category(report, "library_checker")["status"], "UNAVAILABLE")
        value["status"] = "PASS"
        self.assertEqual(self.aggregate(value)["status"], "FAIL")


    def test_official_runner_change_is_stale_and_submission_sha_is_preserved(self):
        value = self.official()
        runner = "Scripts/ci_library_checker.py"
        self.write(runner, "# original runner\n")
        value["runner"] = {"source": runner, "sha256": self.hash(runner)}
        report = self.aggregate(value)
        evidence = self.category(report, "library_checker")["evidence"][0]
        self.assertEqual(evidence["submission_sha256"], self.entry["submission_sha256"])
        self.assertEqual(evidence["runner"], value["runner"])
        self.write(runner, "# edited runner\n")
        self.assertEqual(self.category(self.aggregate(value), "library_checker")["status"], "STALE")


    def test_official_generation_unavailable_is_failure_not_empty_pass(self):
        value = self.official(); value["status"] = "FAIL"
        row = value["results"][0]
        row.update(status="FAIL", unavailable=True, cases=[], official={}, case_counts={"expected":0,"completed":0,"passed":0,"failed":0})
        row.pop("elapsed_ns")
        report = self.aggregate(value)
        self.assertEqual(report["status"], "FAIL")
        self.assertEqual(self.category(report, "library_checker")["status"], "FAIL")
        self.assertIsNone(self.category(report, "library_checker")["evidence"][0]["elapsed_ns"])
        self.assertIn("未実行", quality.markdown(report))


    def test_directory_discovery_does_not_parse_auxiliary_json_and_bad_paths_rejected(self):
        self.report_file(self.contract())
        (self.inputs / "one/coverage_entries").mkdir()
        (self.inputs / "one/coverage_entries/entry_0001.json").write_text('{"not":"report"}')
        self.assertEqual(quality.aggregate(self.root, [self.inputs])["input_report_status"], "PASS")
        value = self.contract(); value["source_hashes"]["../outside"] = "a" * 64
        self.assertEqual(self.aggregate(value)["status"], "FAIL")
        with self.assertRaises(quality.ReportError):
            quality.relative("Fast/GCC/DataStructure/A.cpp", library=True)

    def test_ambiguous_json_keys_and_nonfinite_values_are_not_accepted(self):
        path = self.report_file(self.contract())
        path.write_text('{"format":1,"format":2}')
        self.assertEqual(quality.aggregate(self.root, [path])["status"], "FAIL")
        path.write_text('{"format":1,"time":NaN}')
        self.assertEqual(quality.aggregate(self.root, [path])["status"], "FAIL")


    def test_report_digest_and_payload_use_one_snapshot_even_if_replaced(self):
        path = self.report_file(self.contract())
        original = path.read_bytes()
        replacement = b'{"format":1,"status":"running"}'
        read_bytes = Path.read_bytes
        reads = []
        def replace_after_read(current):
            value = read_bytes(current)
            if current == path:
                reads.append(current)
                if len(reads) == 1:
                    path.write_bytes(replacement)
            return value
        with patch.object(Path, "read_bytes", autospec=True, side_effect=replace_after_read):
            report = quality.aggregate(self.root, [path])
        self.assertEqual(len(reads), 1)
        self.assertEqual(report["input_report_status"], "PASS")
        self.assertEqual(report["reports"][0]["sha256"], quality.digest(original))
        self.assertEqual(path.read_bytes(), replacement)


    def test_markdown_escapes_report_control_text(self):
        report = quality.aggregate(self.root, [])
        report["libraries"][0]["path"] = 'bad|`<img src=x>\n\u202e{{site.title}}{% include secret %}'
        rendered = quality.markdown(report)
        self.assertNotIn("<img", rendered)
        self.assertNotIn("\u202e", rendered)
        self.assertIn("&lt;img", rendered)
        self.assertIn("&#124;", rendered)
        self.assertNotIn("{{", rendered)
        self.assertNotIn("{%", rendered)
        self.assertIn("&#123;", rendered)


if __name__ == "__main__":
    unittest.main()
