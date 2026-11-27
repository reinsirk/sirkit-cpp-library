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

    def all_performance(self, quick=False):
        runner = "Scripts/ci_performance_all.py"
        self.write(runner, "// runner fixture\n")
        names = ["registered_sample_00"] if quick else ["example_00", "max_random_00"]
        row = {"library": self.library, "verify_id": "alpha", "libraries": [self.library],
               "case_source": "official_library_checker", "status": "PASS", "current_measurements_passed": True,
               "current_build": {"status": "PASS"}, "source_hashes": self.entry["source_hashes"],
               "driver": self.driver, "driver_sha256": self.hash(self.driver), "support_source_hashes": {},
               "submission_sha256": self.entry["submission_sha256"], "expected_case_names": names,
               "official": {"expected_cases": ["example_00", "max_random_00"],
                            "info_sha256": "1" * 64, "hash_json_sha256": "2" * 64, "checker_sha256": "3" * 64},
               "official_case_input_bytes": {"example_00": 8, "max_random_00": 20},
               "cases": [{"name": name, "status": "PASS", "correctness_passed": True,
                          "input_sha256": "4" * 64, "expected_sha256": "5" * 64,
                          "input_bytes": 8, "metadata": {"dimensions": {"n": 5}},
                          "samples_ns": {"current": [100, 200, 300]}, "median_ns": {"current": 200},
                          "cpu_samples_ns": {"current": [10, 20, 30]}, "cpu_median_ns": {"current": 20},
                          "current_over_base": None, "comparison": "NOT_REQUESTED"} for name in names]}
        return {"format": 1, "type": "sirkit.performance-all", "status": "PASS", "online_status": "not_evaluated",
                "scope": "whole_verify_program_fixed_main_io", "size_profile": "quick_smoke" if quick else "full_representative",
                "all_library_paths": [self.library, "Graph/B.cpp"], "selected_library_paths": [self.library],
                "selection_kind": "all_libraries", "shard_count": 2, "shard_index": 0,
                "repetitions": 3, "warmups": 1, "base_revision": None,
                "source_freeze_passed": True, "source_hashes": {runner: self.hash(runner)},
                "runner": {"source": runner, "sha256": self.hash(runner)},
                "flags": ["-std=c++23", "-O2"], "compiler": {"command": ["g++"], "version": "test GCC"},
                "cpu_model": "unit CPU", "platform": "test Linux", "results": [row]}

    def test_absolute_kernel_without_comparison(self):
        value = self.performance()
        value.update(status="CURRENT_ONLY", comparison_requested=False, base_revision=None, compared_cases=0, uncompared_cases=1)
        row = value["results"][0]
        row.update(status="CURRENT_ONLY", current_over_base=None)
        del row["samples_ns"]["base"]; del row["median_ns"]["base"]
        with patch.dict(quality.ci_benchmark.LIBRARIES, {"alpha": self.library}):
            report = self.aggregate(value)
        self.assertEqual(report["input_report_status"], "PASS", report["errors"])
        self.assertIn("wall ms", quality.markdown(report))

    def test_all_performance_absolute_times_and_coverage(self):
        report = self.aggregate(self.all_performance())
        self.assertEqual(report["input_report_status"], "PASS", report["errors"])
        self.assertEqual(report["performance_coverage"], {"all_libraries": 2, "whole_program_full_measured": 1})
        rows = self.category(report, "performance")["evidence"]
        self.assertEqual(rows[0]["cpu_median_ns"]["current"], 20)
        self.assertIsNone(rows[0]["current_over_base"])
        self.assertIn("1/2", quality.markdown(report))
        self.assertIn("Verify全体", quality.markdown(report))

    def test_all_performance_omissions_are_not_success(self):
        for mutate in (lambda v: v["results"].clear(),
                       lambda v: v["selected_library_paths"].clear(),
                       lambda v: v["all_library_paths"].pop(),
                       lambda v: v["results"][0]["cases"].pop(),
                       lambda v: (v["results"][0]["cases"].pop(), v["results"][0]["expected_case_names"].pop()),
                       lambda v: v["results"][0]["current_build"].update(status="FAIL"),
                       lambda v: v["results"][0]["cases"][0]["median_ns"].update(current=999),
                       lambda v: v["results"][0]["cases"][0]["cpu_samples_ns"]["current"].pop()):
            value = self.all_performance(); mutate(value)
            with self.subTest(value=value):
                self.assertEqual(self.aggregate(value)["input_report_status"], "FAIL")

    def test_all_performance_quick_is_only_partial_and_source_changes_stale(self):
        report = self.aggregate(self.all_performance(quick=True))
        self.assertEqual(self.category(report, "performance")["status"], "PARTIAL")
        self.assertEqual(report["performance_coverage"]["whole_program_full_measured"], 0)
        self.assertIn("縮小確認のみ", quality.markdown(report))
        value = self.all_performance(); self.write(self.library, "// changed\n")
        self.assertEqual(self.category(self.aggregate(value), "performance")["status"], "STALE")

    def test_nonlc_plan_deletion_and_scale_change_are_not_success(self):
        from types import SimpleNamespace
        plans = [{"name": "example_00", "dimensions": {"n": 5}},
                 {"name": "max_random_00", "dimensions": {"n": 5}}]
        with patch.object(quality.ci_performance_all, "nonlc_module", return_value=SimpleNamespace(plans_for_library=lambda *args, **kw: plans)):
            value = self.all_performance(); value["results"][0]["case_source"] = "deterministic_non_lc"
            self.assertEqual(self.aggregate(value)["input_report_status"], "PASS")
            value["results"][0]["cases"].pop(); value["results"][0]["expected_case_names"].pop()
            self.assertEqual(self.aggregate(value)["input_report_status"], "FAIL")
            value = self.all_performance(); value["results"][0]["case_source"] = "deterministic_non_lc"
            value["results"][0]["cases"][0]["metadata"]["dimensions"]["n"] = 1
            self.assertEqual(self.aggregate(value)["input_report_status"], "FAIL")

    def test_local_performance_binds_the_runtime_checks_disabled_program(self):
        from types import SimpleNamespace

        self.entry["verification_kind"] = "local_regression"
        with quality.verify_root(self.root):
            normal, _ = quality.prepare_verify.render(self.entry)
            measured, _ = quality.prepare_verify.render(self.entry, skip_runtime_checks=True)
        self.write(self.entry["submission"], normal)
        self.entry["submission_sha256"] = quality.digest(normal)
        self.write("Verify/current/manifest.json", {"format": 1, "entries": [self.entry]})
        report = self.all_performance()
        row = report["results"][0]
        row.update(case_source="deterministic_non_lc", submission_sha256=quality.digest(measured))
        plans = [{"name": name, "dimensions": {"n": 5}} for name in row["expected_case_names"]]
        nonlc = SimpleNamespace(plans_for_library=lambda *args, **kw: plans)
        with patch.object(quality.ci_performance_all, "nonlc_module", return_value=nonlc):
            self.assertEqual(self.category(self.aggregate(report), "performance")["status"], "PASS")
            row["submission_sha256"] = quality.digest(normal)
            self.assertEqual(self.aggregate(report)["input_report_status"], "FAIL")
            row["submission_sha256"] = quality.digest(measured)
            self.write(self.library, "// updated library\n")
            self.assertEqual(self.category(self.aggregate(report), "performance")["status"], "STALE")

    def test_all_performance_failed_rows_render_without_fake_time(self):
        value = self.all_performance(); value["results"][0].update(status="FAIL", error="compile failed")
        report = self.aggregate(value)
        self.assertEqual(self.category(report, "performance")["status"], "FAIL")
        self.assertIn("未実施", quality.markdown(report))
        self.assertEqual(report["performance_coverage"]["whole_program_full_measured"], 0)

    def failed_performance_shard(self, *, quick=False, peer_status="FAIL"):
        value = self.all_performance(quick=quick)
        value.update(status="FAIL", current_measurements_passed=False, shard_count=1,
                     selected_library_paths=list(value["all_library_paths"]))
        value["results"].append({"library": "Graph/B.cpp", "verify_id": "beta",
                                 "libraries": ["Graph/B.cpp"], "status": peer_status,
                                 "current_measurements_passed": False, "cases": [],
                                 "error": "compile failed" if peer_status == "FAIL" else "not run"})
        entries = {"alpha": self.entry, "beta": {"id": "beta", "libraries": ["Graph/B.cpp"]}}
        return value, entries

    def test_failed_performance_shard_preserves_successful_library_measurements(self):
        for peer_status in ("FAIL", "NOT_RUN"):
            with self.subTest(peer_status=peer_status):
                value, entries = self.failed_performance_shard(peer_status=peer_status)
                report = quality.aggregate(self.root, [self.report_file(value)], source_manifest=entries)
                self.assertEqual(report["errors"], [])
                self.assertEqual(report["status"], "FAIL")
                self.assertEqual(report["input_report_status"], "FAIL")
                self.assertEqual(report["reports"][0]["status"], "FAIL")
                passed = self.category(report, "performance")
                self.assertEqual(passed["status"], "PASS")
                self.assertEqual(len(passed["evidence"]), 2)
                self.assertEqual(passed["evidence"][0]["median_ns"]["current"], 200)
                self.assertEqual(passed["evidence"][0]["cpu_median_ns"]["current"], 20)
                peer = self.category(report, "performance", "Graph/B.cpp")
                self.assertEqual(peer["status"], "FAIL" if peer_status == "FAIL" else "UNAVAILABLE")
                self.assertNotIn("median_ns", peer["evidence"][0])
                self.assertEqual(report["performance_coverage"]["whole_program_full_measured"], 1)

    def test_failed_performance_shard_keeps_quick_and_source_integrity_limits(self):
        for profile, freeze, stale, expected in (
                (True, True, False, "PARTIAL"),
                (False, False, False, "FAIL"),
                (False, True, True, "STALE")):
            with self.subTest(quick=profile, freeze=freeze, stale=stale):
                value, entries = self.failed_performance_shard(quick=profile)
                value["source_freeze_passed"] = freeze
                original = (self.root / self.library).read_bytes()
                if stale:
                    self.write(self.library, original + b"// changed after measurement\n")
                try:
                    report = quality.aggregate(self.root, [self.report_file(value)], source_manifest=entries)
                finally:
                    self.write(self.library, original)
                self.assertEqual(report["errors"], [])
                self.assertEqual(report["status"], "FAIL")
                self.assertEqual(self.category(report, "performance")["status"], expected)
                self.assertEqual(report["performance_coverage"]["whole_program_full_measured"], 0)

    def instrumented(self, mode="sanitizers"):
        hashes = {self.library: self.hash(self.library), self.driver: self.hash(self.driver)}
        flags = ["-fsanitize=address,undefined"] if mode == "sanitizers" else ["-fprofile-instr-generate", "-fcoverage-mapping"]
        return {"format": 1, "mode": mode, "checked": True, "status": "PASS", "online_status": "not_evaluated",
                "all_source_hashes": hashes, "source_hashes_after": deepcopy(hashes),
                "library_source_hashes": {self.library: hashes[self.library]}, "vendor_source_hashes": {},
                "selected_ids": ["alpha"], "results": [{"id": "alpha", "status": "PASS", "compile_returncode": 0,
                 "driver_sha256": hashes[self.driver], "command": ["clang++", "-UNDEBUG", "-USKIP_RUNTIME_CHECKS", *flags],
                 "samples": [{"passed": True, "returncode": 0}]}]}

    def with_limits(self, value, *, kernel=False):
        module = quality.performance_limits
        configured = []
        if kernel:
            for row in value["results"]:
                row["pattern_id"] = 0
                configured.append({"scope": "kernel", "library": self.library,
                    "kernel_id": row["id"], "case": row["pattern"], "size": row["size"],
                    "pattern": 0, "rounds": row["rounds"], "metric": "kernel_elapsed_ns",
                    "baseline_median_ns": 100, "limit_ns": 200})
        else:
            for row in value["results"]:
                for case in row["cases"]:
                    configured.append({"scope": "whole_program", "library": self.library,
                        "verify_id": row["verify_id"], "case": case["name"],
                        "input_bytes": case["input_bytes"], "metric": "child_cpu_ns",
                        "baseline_median_ns": 100, "limit_ns": 10000000})
        path = self.write("Verify/ci/performance_limits.json", {"format": 1,
            "baseline": {"fixture": "fixed"},
            "policy": {"multiplier": 2, "whole_program_minimum_ns": 10000000}, "limits": configured})
        limits = module.load(path)
        value["performance_limits"] = limits.metadata
        gates = []
        for row in value["results"]:
            if kernel:
                entry = limits.kernel(self.library, row["id"], row["pattern"], row["size"], 0, row["rounds"])
                row["performance_limit"] = limits.evaluate(entry, row["samples_ns"]["current"])
                gates.append(row["performance_limit"])
            else:
                for case in row["cases"]:
                    entry = limits.whole(self.library, row["verify_id"], case["name"], case["input_bytes"])
                    case["performance_limit"] = limits.evaluate(entry, case["cpu_samples_ns"]["current"])
                    gates.append(case["performance_limit"])
        value["performance_limits_passed"] = all(g["status"] == "PASS" for g in gates)
        if not value["performance_limits_passed"]:
            value["status"] = "FAIL"
        return value

    def test_whole_limits_use_cpu_not_noisy_wall_time(self):
        value = self.all_performance()
        for case in value["results"][0]["cases"]:
            case["samples_ns"]["current"] = [1000000000] * 3
            case["median_ns"]["current"] = 1000000000
        report = self.aggregate(self.with_limits(value))
        data = self.category(report, "performance")
        self.assertEqual(data["status"], "PASS", report["errors"])
        self.assertTrue(all(e["performance_limit"]["metric"] == "child_cpu_ns" for e in data["evidence"]))
        self.assertIn("上限 ms", quality.markdown(report))
        case = value["results"][0]["cases"][0]
        case["cpu_samples_ns"]["current"] = [10000001] * 3
        case["cpu_median_ns"]["current"] = 10000001
        report = self.aggregate(self.with_limits(value))
        self.assertEqual(self.category(report, "performance")["status"], "FAIL")
        self.assertEqual(value["results"][0]["cases"][0]["status"], "PASS")
        self.assertTrue(value["results"][0]["cases"][0]["correctness_passed"])

    def test_kernel_fixed_limit_boundary_and_tampering(self):
        self.enterContext(patch.dict(quality.ci_benchmark.SCENARIOS, {"alpha": ("random",)}))
        value = self.performance()
        value["results"][0]["samples_ns"]["current"] = [199, 200, 201]
        value["results"][0]["median_ns"]["current"] = 200
        value["results"][0]["current_over_base"] = 200 / 101
        report = self.aggregate(self.with_limits(value, kernel=True))
        self.assertEqual(self.category(report, "performance")["status"], "PASS", report["errors"])
        row = value["results"][0]
        row["samples_ns"]["current"] = [200, 201, 202]
        row["median_ns"]["current"] = 201
        row["current_over_base"] = 201 / 101
        self.with_limits(value, kernel=True)
        report = self.aggregate(value)
        self.assertEqual(self.category(report, "performance")["status"], "FAIL", report["errors"])
        row["performance_limit"]["status"] = "PASS"
        self.assertTrue(self.aggregate(value)["errors"])

    def test_changed_budget_or_missing_gate_cannot_reuse_pass(self):
        value = self.with_limits(self.all_performance())
        del value["results"][0]["cases"][0]["performance_limit"]
        self.assertTrue(self.aggregate(value)["errors"])
        value = self.with_limits(self.all_performance())
        path = self.root / "Verify/ci/performance_limits.json"
        current = json.loads(path.read_text());current["baseline"]["fixture"] = "changed"
        self.write("Verify/ci/performance_limits.json", current)
        self.assertTrue(self.aggregate(value)["errors"])

    def test_coverage_display_does_not_round_missing_path_to_hundred(self):
        self.assertEqual(quality.coverage_percent({"count": 1000000, "covered": 999999}), "100未満")

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

    def test_structural_benchmark_names_and_scale_are_preserved(self):
        value = self.performance()
        source = value["sources"].pop("alpha")
        self.write("Verify/ci/benchmarks/fenwick.cpp", "// benchmark\n")
        value["sources"]["fenwick"] = source
        value["scale_scope"] = "small_large_ci_stress_and_power_of_two_boundaries"
        row = value["results"][0]
        row.update(id="fenwick", pattern="empty_full_lowbit", pattern_id=2,
                   size=2_000_000, scale="ci_stress_limit")
        data = self.category(self.aggregate(value), "performance")
        self.assertEqual(data["status"], "PASS")
        self.assertEqual(data["evidence"][0]["scale"], "ci_stress_limit")
        for changes in ({"size": 1024}, {"pattern_id": 3}, {"pattern": "invented_case"},
                        {"scale": "smoke"}):
            wrong = deepcopy(value)
            wrong["results"][0].update(changes)
            self.assertEqual(self.aggregate(wrong)["status"], "FAIL")

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

    def kernel_comparison(self, *, regressed=False):
        value = self.performance()
        value.update(status="FAIL" if regressed else "PASS", performance_regression_passed=not regressed)
        row = value["results"][0]
        current = 1_050_001 if regressed else 1_050_000
        row.update(samples_ns={"base": [1_000_000] * 3, "current": [current] * 3},
                   median_ns={"base": 1_000_000, "current": current},
                   current_over_base=current / 1_000_000,
                   regression_confirmation={"base": [1_000_000] * 3, "current": [1_060_000] * 3})
        row["performance_regression"] = quality.performance_regression.evaluate("kernel_elapsed_ns", row["samples_ns"])
        return value

    def test_kernel_comparison_uses_final_samples_after_confirmation(self):
        for regressed in (False, True):
            with self.subTest(regressed=regressed):
                value = self.kernel_comparison(regressed=regressed)
                report = self.aggregate(value)
                self.assertFalse(report["errors"])
                expected = "FAIL" if regressed else "PASS"
                self.assertEqual(report["input_report_status"], expected)
                evidence, = self.category(report, "performance")["evidence"]
                self.assertEqual(evidence["status"], expected)
                self.assertEqual(evidence["performance_regression"]["limit_ns"], 1_050_000)
                self.assertEqual(evidence["regression_confirmation"], value["results"][0]["regression_confirmation"])
                self.assertIn("比較元コミット", quality.markdown(report))
                self.assertIn("（wall・再測定）", quality.markdown(report))

    def test_regression_confirmation_and_summary_cannot_be_forged(self):
        for mutate in (
                lambda report: report.update(performance_regression_passed=True),
                lambda report: report.update(performance_regression_passed=0),
                lambda report: report.update(status="PASS"),
                lambda report: report.update(base_revision=None),
                lambda report: report["results"][0].pop("regression_confirmation"),
                lambda report: report["results"][0]["regression_confirmation"].update(current=[1_020_000] * 3),
                lambda report: report["results"][0]["regression_confirmation"].update(base=[1_000_000] * 2),
                lambda report: report["results"][0]["performance_regression"].update(limit_ns=1_050_001)):
            with self.subTest(mutate=mutate):
                value = self.kernel_comparison(regressed=True)
                mutate(value)
                report = self.aggregate(value)
                self.assertTrue(report["errors"])
                self.assertEqual(report["input_report_status"], "FAIL")

    def test_kernel_worst_case_gate_recomputes_across_all_cases(self):
        value = self.performance()
        source = value["sources"].pop("alpha")
        fixture = "Verify/ci/benchmarks/fenwick.cpp"
        self.write(fixture, "// benchmark\n")
        source["harness_sha256"] = self.hash(fixture)
        value["sources"]["fenwick"] = source
        value.update(regression_policy="library_worst_case", performance_regression_passed=True,
                     compared_cases=2, expected_cases=2)
        template = value["results"][0]
        value["results"] = []
        groups = {"64:0": {"base": [100_000] * 3, "current": [800_000] * 3},
                  "128:0": {"base": [1_000_000] * 3, "current": [900_000] * 3}}
        for key, samples in groups.items():
            row = dict(template, id="fenwick", size=int(key.split(":")[0]),
                       pattern=quality.ci_benchmark.SCENARIOS["fenwick"][0], pattern_id=0,
                       samples_ns=samples, median_ns={role: times[0] for role, times in samples.items()},
                       current_over_base=samples["current"][0] / samples["base"][0],
                       performance_regression=quality.performance_regression.evaluate("kernel_elapsed_ns", samples))
            value["results"].append(row)
        value["worst_case_regressions"] = {
            "fenwick": quality.performance_regression.evaluate_worst("kernel_elapsed_ns", groups)}
        report = self.aggregate(value)
        self.assertFalse(report["errors"])
        self.assertEqual(self.category(report, "performance")["status"], "PASS")
        self.assertIn("参考値", quality.markdown(report))
        self.assertIn("基準の最悪ケース", quality.markdown(report))
        value["worst_case_regressions"]["fenwick"]["baseline_case"] = "64:0"
        self.assertTrue(self.aggregate(value)["errors"])

    def test_whole_worst_case_gate_accepts_slowest_case_switch(self):
        value = self.all_performance()
        profile = "github-hosted"
        value.update(base_revision="a" * 40, performance_regression_passed=True,
                     performance_regression_counts={"PASS": 1}, regression_policy="library_worst_case",
                     regression_profile=profile)
        row, = value["results"]
        row.update(baseline_sources={"source_hashes": {self.library: "a" * 64},
                                     "resolved_paths": {self.library: self.library}},
                   baseline_comparison="MEASURED", baseline_build={"status": "PASS"},
                   baseline_driver={"kind": "current", "path": self.driver,
                                    "sha256": row["driver_sha256"], "same_as_current": True})
        groups = {}
        for case, baseline, current in zip(row["cases"], (10_000_000, 1_000_000), (8_000_000, 11_000_000)):
            samples = {"base": [baseline] * 3, "current": [current] * 3}
            medians = {"base": baseline, "current": current}
            groups[case["name"]] = samples
            case.update(samples_ns=samples, median_ns=medians, cpu_samples_ns=samples, cpu_median_ns=medians,
                        current_over_base=current / baseline, comparison="MEASURED",
                        performance_regression=quality.performance_regression.evaluate("child_cpu_ns", samples, profile=profile))
        row["worst_case_regression"] = quality.performance_regression.evaluate_worst("child_cpu_ns", groups, profile=profile)
        report = self.aggregate(value)
        self.assertFalse(report["errors"])
        data = self.category(report, "performance")
        self.assertEqual(data["status"], "PASS")
        self.assertEqual(data["evidence"][1]["performance_regression"]["status"], "FAIL")
        self.assertEqual(row["worst_case_regression"]["baseline_case"], "example_00")
        self.assertEqual(row["worst_case_regression"]["observed_case"], "max_random_00")
        for wrong_profile in ("local", "unknown"):
            with self.subTest(profile=wrong_profile):
                self.assertTrue(self.aggregate({**value, "regression_profile": wrong_profile})["errors"])
        row["worst_case_confirmation"] = {row["cases"][0]["name"]: deepcopy(groups[row["cases"][0]["name"]])}
        self.assertTrue(self.aggregate(value)["errors"])

    def test_whole_uses_cpu_baseline_and_preserves_comparison_unavailable(self):
        value = self.all_performance()
        value.update(status="FAIL", base_revision="a" * 40, performance_regression_passed=False,
                     performance_regression_counts={"FAIL": 1, "NOT_COMPARABLE": 1})
        row = value["results"][0]
        row.update(baseline_sources={"source_hashes": {self.library: "a" * 64},
                                     "resolved_paths": {self.library: self.library}},
                   baseline_comparison="MEASURED", baseline_build={"status": "PASS"},
                   baseline_driver={"kind": "current", "path": self.driver,
                                    "sha256": row["driver_sha256"], "same_as_current": True})
        compared, unavailable = row["cases"]
        compared.update(samples_ns={"base": [50_000_000] * 3, "current": [2_000_000] * 3},
                        median_ns={"base": 50_000_000, "current": 2_000_000},
                        cpu_samples_ns={"base": [1_000_000] * 3, "current": [1_101_000] * 3},
                        cpu_median_ns={"base": 1_000_000, "current": 1_101_000},
                        current_over_base=0.04, comparison="MEASURED",
                        regression_confirmation={"base": [1_000_000] * 3, "current": [1_102_000] * 3})
        compared["performance_regression"] = quality.performance_regression.evaluate(
            "child_cpu_ns", compared["cpu_samples_ns"])
        unavailable.update(comparison="BASELINE_INVALID",
                           baseline_failure={"trial": 0, "returncode": 0, "timed_out": False,
                                             "oracle_passed": False, "output_sha256": "a" * 64},
                           performance_regression={"status": "NOT_COMPARABLE", "metric": "child_cpu_ns",
                                                   "reason": "BASELINE_INVALID"})
        report = self.aggregate(value)
        self.assertFalse(report["errors"])
        self.assertEqual(report["input_report_status"], "FAIL")
        bad, missing = self.category(report, "performance")["evidence"]
        self.assertEqual(bad["performance_regression"]["baseline_median_ns"], 1_000_000)
        self.assertEqual(bad["status"], "FAIL")
        self.assertEqual(missing["status"], "PASS")
        self.assertNotIn("baseline_median_ns", missing["performance_regression"])
        markdown = quality.markdown(report)
        self.assertIn("2.000 | 1.101 | 1.000 | 1.101× | 1.100 | 退行（CPU・再測定）", markdown)
        self.assertIn("比較対象外（旧版照合失敗）", markdown)
        unavailable["performance_regression"] = compared["performance_regression"]
        self.assertTrue(self.aggregate(value)["errors"])
        unavailable["regression_confirmation"] = compared["regression_confirmation"]
        unavailable["performance_regression"] = {"status": "FAIL", "metric": "child_cpu_ns",
                                                "reason": "BASELINE_CONFIRMATION_UNAVAILABLE"}
        value["performance_regression_counts"] = {"FAIL": 2}
        report = self.aggregate(value)
        self.assertFalse(report["errors"])
        self.assertIn("比較失敗（基準版の再測定不可）", quality.markdown(report))

    def test_new_whole_library_has_no_comparable_cases(self):
        value = self.all_performance()
        value.update(base_revision="a" * 40, performance_regression_passed=True,
                     performance_regression_counts={"NOT_COMPARABLE": 2})
        item = value["results"][0]
        item.update(baseline_sources={"source_hashes": {}, "resolved_paths": {},
                                      "missing_path": self.library, "missing_paths": [self.library]},
                    baseline_comparison="BASELINE_NEW_LIBRARY")
        for case in item["cases"]:
            case.update(comparison="BASELINE_NEW_LIBRARY",
                        performance_regression={"status": "NOT_COMPARABLE", "metric": "child_cpu_ns",
                                                "reason": "BASELINE_NEW_LIBRARY"})
        report = self.aggregate(value)
        self.assertFalse(report["errors"])
        self.assertEqual(report["input_report_status"], "PASS")
        self.assertIn("比較対象外（比較元なし）", quality.markdown(report))
        item["baseline_sources"]["missing_paths"] = []
        self.assertTrue(self.aggregate(value)["errors"])

    def test_unpaired_kernel_does_not_claim_regression_pass(self):
        value = self.performance()
        value.update(status="NO_BASELINE", compared_cases=0, uncompared_cases=1, expected_cases=1,
                     performance_regression_passed=False)
        case = value["results"][0]
        case.update(status="NO_BASELINE", current_over_base=None)
        del case["samples_ns"]["base"]
        del case["median_ns"]["base"]
        report = self.aggregate(value)
        self.assertFalse(report["errors"])
        self.assertEqual(report["input_report_status"], "PARTIAL")
        self.assertIn("比較対象外（比較元なし）", quality.markdown(report))

    def test_instrumented_requires_checked_flags_full_hashes_and_nonempty_samples(self):
        self.assertEqual(self.category(self.aggregate(self.instrumented()), "sanitizers")["status"], "PASS")
        for mutation in (lambda v: v.update(checked=False),
                         lambda v: v["results"][0].update(samples=[]),
                         lambda v: v["results"][0]["command"].remove("-fsanitize=address,undefined"),
                         lambda v: v["all_source_hashes"].pop(self.driver)):
            value = self.instrumented(); mutation(value)
            self.assertEqual(self.aggregate(value)["status"], "FAIL")

    def test_coverage_requires_exact_full_metrics_and_missing_data_fails(self):
        value = self.instrumented("coverage")
        report = self.aggregate(value)
        self.assertEqual(self.category(report, "coverage")["status"], "FAIL")
        self.assertEqual(report["input_report_status"], "FAIL")
        path = self.inputs / "one/coverage.json"
        metric = {"count": 2, "covered": 1, "percent": 50}
        path.write_text(json.dumps({"format": 1, "type": "sirkit.source-coverage-union", "files": [
            {"filename": "/old/runner/checkout/" + self.library, "lines": [[1, True], [2, False]],
             "branches": [[1, 1, 1, 5, True, False]], "summary": {"lines": metric, "branches": metric}}]}))
        data = self.category(self.aggregate(value), "coverage")
        self.assertEqual(data["status"], "FAIL")
        self.assertEqual(data["evidence"][0]["metrics"]["lines"]["percent"], 50)

    def test_full_coverage_and_no_conditions_can_pass_without_rounding(self):
        value = self.instrumented("coverage")
        self.report_file(value)
        path = self.inputs / "one/coverage.json"
        def measured(line_hit=True, branch_hit=True, branches=True):
            metric = {"count": 2, "covered": 1 + line_hit, "percent": 100}
            branch_metric = {"count": 2 if branches else 0,
                             "covered": 1 + branch_hit if branches else 0, "percent": 100}
            path.write_text(json.dumps({"format": 1, "type": "sirkit.source-coverage-union", "files": [
                {"filename": self.library, "lines": [[1, True], [2, line_hit]],
                 "branches": [[1, 1, 1, 5, True, branch_hit]] if branches else [],
                 "summary": {"lines": metric, "branches": branch_metric}}]}))
            return self.aggregate(value)
        for line_hit, branch_hit in ((False, True), (True, False), (False, False)):
            report = measured(line_hit, branch_hit)
            self.assertEqual(self.category(report, "coverage")["status"], "FAIL")
            self.assertEqual(report["input_report_status"], "FAIL")
        report = measured()
        self.assertEqual(self.category(report, "coverage")["status"], "PASS")
        self.assertEqual(report["input_report_status"], "PASS")
        self.assertEqual(report["coverage_policy"]["required_percent"], 100)
        self.assertIn("どちらも100%", quality.markdown(report))
        self.assertIn("2/2", quality.markdown(report))
        self.assertEqual(self.category(measured(branches=False), "coverage")["status"], "PASS")
        for field, setting in (("coverage_complete", False), ("unmapped_sources", [self.library])):
            value[field] = setting
            self.assertEqual(self.aggregate(value)["input_report_status"], "FAIL")
            del value[field]

    def test_coverage_accepts_registered_standalone_test_with_matching_sources(self):
        value = self.instrumented("coverage")
        name = "standalone_test.cpp"
        driver = "Verify/current/tests/" + name
        registry = quality.ci_instrumented.COVERAGE_TESTS
        self.write(driver, "int main() {}\n")
        self.write(registry, {"format": 1, "tests": {name: [self.library]}})
        value["all_source_hashes"].update({registry: self.hash(registry), driver: self.hash(driver)})
        value["source_hashes_after"] = deepcopy(value["all_source_hashes"])
        identifier = "coverage_test_standalone_test"
        value["selected_ids"] = [identifier]
        value["results"][0].update(id=identifier, driver=driver, driver_sha256=self.hash(driver),
                                   coverage_test="assertions")
        self.write("artifacts/one/coverage.json",
                   {"format": 1, "type": "sirkit.source-coverage-union", "files": [{
                       "filename": self.library, "lines": [[1, True]], "branches": [],
                       "summary": {"lines": {"count": 1, "covered": 1},
                                   "branches": {"count": 0, "covered": 0}}
                   }]})
        report = self.aggregate(value)
        self.assertEqual(self.category(report, "coverage")["status"], "PASS", report["errors"])
        self.write(registry, {"format": 1, "tests": {name: ["Graph/B.cpp"]}})
        self.assertEqual(self.aggregate(value)["input_report_status"], "FAIL")

    def test_only_registered_failure_fixtures_count_and_require_a_successful_checked_run(self):
        value = self.instrumented("coverage")
        name = "negative_test.cpp"
        driver = "Verify/current/tests/" + name
        registry = quality.ci_instrumented.COVERAGE_TESTS
        other = "Graph/B.cpp"
        self.write(driver, "int main() {}\n")
        self.write(registry, {"format": 1, "tests": {name: [self.library, other]},
                              "failure_tests": {self.library: name}})
        value["all_source_hashes"].update({registry: self.hash(registry), driver: self.hash(driver),
                                           other: self.hash(other)})
        value["source_hashes_after"] = deepcopy(value["all_source_hashes"])
        identifier = "coverage_test_negative_test"
        value["selected_ids"] = [identifier]
        row = value["results"][0]
        row.update(id=identifier, driver=driver, driver_sha256=self.hash(driver), coverage_test="assertions")
        self.assertEqual(self.category(quality.aggregate(self.root, []), "failure")["status"], "UNAVAILABLE")

        report = self.aggregate(value)
        negative = self.category(report, "failure")
        self.assertEqual(negative["status"], "PASS", report["errors"])
        self.assertEqual(negative["evidence"][0]["driver"], driver)
        self.assertEqual(self.category(report, "failure", other)["status"], "UNAVAILABLE")
        self.assertEqual(self.category(report, "coverage")["status"], "FAIL")

        self.write(driver, "int main() { return 1; }\n")
        self.assertEqual(self.category(self.aggregate(value), "failure")["status"], "STALE")
        self.write(driver, "int main() {}\n")
        row["status"] = "FAIL"
        row["samples"][0].update(passed=False, returncode=1)
        value["status"] = "FAIL"
        self.assertEqual(self.category(self.aggregate(value), "failure")["status"], "FAIL")

    def test_library_checker_applicability_uses_registered_judge_problem(self):
        self.entry["problem_url"] = "https://judge.yosupo.jp/problem/unionfind"
        self.write("Verify/current/manifest.json", {"format": 1, "entries": [self.entry]})
        report = quality.aggregate(self.root, [])
        self.assertEqual(self.category(report, "library_checker")["status"], "UNAVAILABLE")
        self.assertEqual(self.category(report, "library_checker", "Graph/B.cpp")["status"], "NOT_APPLICABLE")
        self.assertIn("| <code>Graph/B.cpp</code> | 未測定 | 未測定 | 無し |", quality.markdown(report))

    def test_coverage_accepts_gcov_lines_only_for_registered_checked_run(self):
        value = self.instrumented("coverage")
        name = "standalone_test.cpp"
        driver = "Verify/current/tests/" + name
        registry = quality.ci_instrumented.COVERAGE_TESTS
        self.write(driver, "int main() {}\n")
        self.write(registry, {"format": 1, "tests": {name: [self.library]}, "gcc_line_tests": [name]})
        value["all_source_hashes"].update({registry: self.hash(registry), driver: self.hash(driver)})
        value["source_hashes_after"] = deepcopy(value["all_source_hashes"])
        identifier = "coverage_test_standalone_test"
        value["selected_ids"] = [identifier]
        entry = value["results"][0]
        entry.update(id=identifier, driver=driver, driver_sha256=self.hash(driver),
                     coverage_test="assertions", gcc_line_coverage={"version": "13.3.0"})
        path = "artifacts/one/coverage.json"
        row = {"filename": self.library, "lines": [[1, True]], "branches": [], "gcov_line_counts": [[1, 12]],
               "summary": {"lines": {"count": 1, "covered": 1}, "branches": {"count": 0, "covered": 0}}}
        report = {"format": 1, "type": "sirkit.source-coverage-union", "files": [row]}
        self.write(path, report)
        data = self.aggregate(value)
        self.assertEqual(self.category(data, "coverage")["status"], "PASS", data["errors"])
        row["gcov_line_counts"] = [[2, 50]]
        self.write(path, report)
        self.assertEqual(self.aggregate(value)["input_report_status"], "FAIL")
        row["gcov_line_counts"] = [[1, 12]]
        self.write(path, report)
        entry.pop("gcc_line_coverage")
        self.assertEqual(self.aggregate(value)["input_report_status"], "FAIL")

    def test_i386_coverage_requires_a_profile_and_the_registered_environment(self):
        value = self.instrumented("coverage")
        name = "limits_test.cpp"
        driver = "Verify/current/tests/" + name
        registry = quality.ci_instrumented.COVERAGE_TESTS
        self.write(driver, "int main() {}\n")
        self.write(registry, {"format": 1, "tests": {name: [self.library]}, "i386_tests": [name]})
        value["all_source_hashes"].update({registry: self.hash(registry), driver: self.hash(driver)})
        value["source_hashes_after"] = deepcopy(value["all_source_hashes"])
        value["tools"] = {"qemu_i386": {"command": "/usr/bin/qemu-i386", "version": "QEMU 8.2"}}
        identifier = "coverage_test_limits_test"
        value["selected_ids"] = [identifier]
        row = value["results"][0]
        row.update(id=identifier, driver=driver, driver_sha256=self.hash(driver), coverage_test="assertions",
                   coverage_target="i386", execution_command=["/usr/bin/qemu-i386", "/tmp/entry"])
        row["command"].extend(["-m32", "-o", "/tmp/entry"])
        row["samples"][0]["profile_written"] = True
        self.write("artifacts/one/coverage.json", {
            "format": 1, "type": "sirkit.source-coverage-union", "files": [{
                "filename": self.library, "lines": [[1, True]], "branches": [],
                "summary": {"lines": {"count": 1, "covered": 1}, "branches": {"count": 0, "covered": 0}},
            }],
        })
        report = self.aggregate(value)
        category = self.category(report, "coverage")
        self.assertEqual(category["status"], "PASS", report["errors"])
        self.assertIn("i386", category["evidence"][0]["scope"])
        for mutation in (
            lambda v: v["results"][0]["samples"][0].pop("profile_written"),
            lambda v: v["results"][0]["command"].remove("-m32"),
            lambda v: v["results"][0].update(execution_command=["/tmp/entry"]),
            lambda v: v["tools"].pop("qemu_i386"),
        ):
            candidate = deepcopy(value)
            mutation(candidate)
            self.assertEqual(self.aggregate(candidate)["input_report_status"], "FAIL")

    def test_gcov_unchecked_lines_require_the_matching_registration(self):
        value = self.instrumented("coverage")
        name = "unchecked_test.cpp"
        driver = "Verify/current/tests/" + name
        registry = quality.ci_instrumented.COVERAGE_TESTS
        self.write(driver, "int main() {}\n")
        self.write(registry, {"format": 1, "tests": {name: [self.library]}, "gcc_unchecked_line_tests": [name]})
        value["all_source_hashes"].update({registry: self.hash(registry), driver: self.hash(driver)})
        value["source_hashes_after"] = deepcopy(value["all_source_hashes"])
        identifier = "coverage_test_unchecked_test"
        value["selected_ids"] = [identifier]
        row = value["results"][0]
        row.update(id=identifier, driver=driver, driver_sha256=self.hash(driver), coverage_test="assertions",
                   gcc_unchecked_line_coverage={"version": "13.3.0"})
        self.write("artifacts/one/coverage.json", {
            "format": 1, "type": "sirkit.source-coverage-union", "files": [{
                "filename": self.library, "lines": [[1, True]], "branches": [], "gcov_unchecked_line_counts": [[1, 7]],
                "summary": {"lines": {"count": 1, "covered": 1}, "branches": {"count": 0, "covered": 0}},
            }],
        })
        report = self.aggregate(value)
        self.assertEqual(self.category(report, "coverage")["status"], "PASS", report["errors"])
        row["gcc_line_coverage"] = row.pop("gcc_unchecked_line_coverage")
        self.assertEqual(self.aggregate(value)["input_report_status"], "FAIL")

    def test_clang_unchecked_coverage_requires_two_successful_builds_and_profiles(self):
        value = self.instrumented("coverage")
        name = "unchecked_test.cpp"
        driver = "Verify/current/tests/" + name
        registry = quality.ci_instrumented.COVERAGE_TESTS
        self.write(driver, "int main() {}\n")
        self.write(registry, {"format": 1, "tests": {name: [self.library]}, "clang_unchecked_tests": [name]})
        value["all_source_hashes"].update({registry: self.hash(registry), driver: self.hash(driver)})
        value["source_hashes_after"] = deepcopy(value["all_source_hashes"])
        identifier = "coverage_test_unchecked_test"
        value["selected_ids"] = [identifier]
        row = value["results"][0]
        row.update(id=identifier, driver=driver, driver_sha256=self.hash(driver), coverage_test="assertions")
        row["clang_unchecked_coverage"] = {
            "command": [*row["command"], "-DSKIP_RUNTIME_CHECKS"],
            "samples": [{"index": 0, "returncode": 0, "passed": True, "profile_written": True}],
        }
        self.write("artifacts/one/coverage.json", {
            "format": 1, "type": "sirkit.source-coverage-union", "files": [{
                "filename": self.library, "lines": [[1, True]], "branches": [],
                "summary": {"lines": {"count": 1, "covered": 1}, "branches": {"count": 0, "covered": 0}},
            }],
        })
        report = self.aggregate(value)
        self.assertEqual(self.category(report, "coverage")["status"], "PASS", report["errors"])
        row["clang_unchecked_coverage"]["samples"][0]["profile_written"] = False
        self.assertEqual(self.aggregate(value)["input_report_status"], "FAIL")

    def test_coverage_requires_registered_proof_and_raw_counters_for_each_exclusion(self):
        value = self.instrumented("coverage")
        policy = quality.ci_instrumented.COVERAGE_EXCLUSIONS
        proved = {"region": [1, 1, 1, 5], "side": "false", "reason": "常に正の範囲"}
        self.write(policy, {"format": 1, "files": {
            self.library: {"sha256": self.hash(self.library), "regions": [proved]}}})
        value["all_source_hashes"][policy] = self.hash(policy)
        value["source_hashes_after"] = deepcopy(value["all_source_hashes"])
        metric = {"count": 1, "covered": 1}
        row = {"filename": self.library, "lines": [[1, True]],
               "branches": [[1, 1, 1, 5, True, False]], "exclusions": [proved],
               "summary": {"lines": metric, "branches": metric}}

        def aggregate(measurement):
            self.write("artifacts/one/coverage.json",
                       {"format": 1, "type": "sirkit.source-coverage-union", "files": [measurement]})
            return self.aggregate(value)

        report = aggregate(row)
        self.assertEqual(self.category(report, "coverage")["status"], "PASS", report["errors"])
        for mutation in (lambda m: m.update(exclusions=[]),
                         lambda m: m["exclusions"][0].update(reason="別の理由"),
                         lambda m: m["branches"][0].__setitem__(-1, True)):
            changed = deepcopy(row)
            mutation(changed)
            self.assertEqual(aggregate(changed)["input_report_status"], "FAIL")
        self.write(policy, {"format": 1, "files": {}})
        self.assertEqual(aggregate(row)["input_report_status"], "FAIL")

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
        self.assertIn("プログラムの起動と入出力を含み", quality.markdown(report))

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
