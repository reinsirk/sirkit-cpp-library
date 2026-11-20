# SPDX-License-Identifier: CC0-1.0
"""全ライブラリ性能runnerの軽い検査。実コンパイル・公式生成・通信は起動しない。"""
from contextlib import redirect_stdout
import io
import json
from pathlib import Path
import signal
import subprocess
import tempfile
from types import SimpleNamespace
import unittest
from unittest.mock import Mock, patch

from Scripts import ci_performance_all as perf


def case(folder: Path, name="case"):
    folder.mkdir(parents=True, exist_ok=True)
    (folder / "input").write_bytes(b"1\n")
    (folder / "expected").write_bytes(b"42\n")
    return {"name": name, "input_path": str(folder / "input"), "expected_path": str(folder / "expected"),
            "input_sha256": perf.file_digest(folder / "input"),
            "expected_sha256": perf.file_digest(folder / "expected"), "checker": "tokens"}


def fixed_limits(folder: Path, *, library="DataStructure/A.cpp", name="registered_sample_00"):
    path = folder / "fixed-limits.json"
    path.write_text(json.dumps({"format": 1, "baseline": {"run_id": "unit-fixture"},
        "policy": {"multiplier": 2, "whole_program_minimum_ns": 10_000_000},
        "limits": [{"scope": "whole_program", "library": library, "verify_id": "a", "case": name,
                    "metric": "child_cpu_ns", "baseline_median_ns": 1_000_000,
                    "limit_ns": 10_000_000, "input_bytes": 2}]}))
    return path


class CatalogTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory(); self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        for name in ["DataStructure/A.cpp", "Graph/B.cpp", "String/C.cpp"]:
            p = self.root / name; p.parent.mkdir(parents=True, exist_ok=True); p.write_text("// library\n")
        self.entries = [
            {"id": "a", "libraries": ["DataStructure/A.cpp"], "driver": "driver", "problem_url": "https://judge.yosupo.jp/problem/unionfind"},
            {"id": "combined", "libraries": ["DataStructure/A.cpp", "Graph/B.cpp"], "driver": "driver", "problem_url": "https://judge.yosupo.jp/problem/aplusb"},
            {"id": "c", "libraries": ["String/C.cpp"], "driver": "driver", "problem_url": "https://yukicoder.me/problems/no/1"},
        ]
        self.nonlc = SimpleNamespace(LIBRARY_VERIFY_IDS={"String/C.cpp": "c"},
            plans_for_library=lambda library, quick=False: [{"name": "fixed", "verify_id": "c"}])

    def test_every_library_has_a_driver_and_combined_scope_is_explicit(self):
        rows = perf.catalog(self.entries, self.nonlc, self.root)
        self.assertEqual([r["library"] for r in rows], ["DataStructure/A.cpp", "Graph/B.cpp", "String/C.cpp"])
        self.assertEqual(rows[0]["verify_id"], "a")
        self.assertFalse(rows[0]["combined_library_scope"])
        self.assertTrue(rows[1]["combined_library_scope"])
        self.assertEqual(rows[2]["case_source"], "deterministic_non_lc")

    def test_missing_nonlc_mapping_or_case_catalog_is_failure(self):
        self.nonlc.LIBRARY_VERIFY_IDS = {}
        with self.assertRaises(perf.PerformanceError):perf.catalog(self.entries, self.nonlc, self.root)
        self.nonlc.LIBRARY_VERIFY_IDS = {"String/C.cpp": "c"}
        self.nonlc.plans_for_library = lambda library, quick=False: []
        with self.assertRaises(perf.PerformanceError):perf.catalog(self.entries, self.nonlc, self.root)

    def test_shards_cover_every_library_exactly_once(self):
        rows = perf.catalog(self.entries, self.nonlc, self.root)
        shards = [perf.select_catalog(rows, None, i, 4) for i in range(4)]
        flattened = [r["library"] for shard in shards for r in shard]
        self.assertEqual(set(flattened), {r["library"] for r in rows})
        self.assertEqual(len(flattened), len(set(flattened)))
        for index, count, requested in [(0,0,None), (-1,4,None), (4,4,None), (0,1,["missing"]), (0,1,[rows[0]["library"]]*2)]:
            with self.assertRaises(perf.PerformanceError):perf.select_catalog(rows, requested, index, count)

    def test_official_selection_keeps_every_generator_family(self):
        with tempfile.TemporaryDirectory() as temporary:
            directory = Path(temporary); (directory / "in").mkdir(); (directory / "out").mkdir()
            names = ["example_00", "random_00", "max_random_00", "conchon_filliatre_tle_00", "conchon_filliatre_tle_01", "fibonacci_00", "short_period_00", "almost_single_00"]
            for i, name in enumerate(names):
                (directory / "in" / (name+".in")).write_text("x"*(i+1))
                (directory / "out" / (name+".out")).write_text("1\n")
            (directory / "checker").write_bytes(b"fake")
            details = {"expected_cases": names, "revision": "a"*40, "problem_id": "problem", "checker_sha256": perf.file_digest(directory / "checker")}
            selected = perf.select_official_cases(directory, details, False)
            self.assertEqual({r["name"] for r in selected}, set(names))
            self.assertEqual(len(selected), len({r["name"] for r in selected}))
            tiny = perf.select_official_cases(directory, details, True)
            self.assertEqual([r["name"] for r in tiny], ["example_00"])

    def test_quick_inline_cases_do_not_call_official_generation(self):
        with tempfile.TemporaryDirectory() as temporary, \
             patch.object(subprocess, "Popen", side_effect=AssertionError("no children")), \
             patch.object(perf.official, "official_problem", side_effect=AssertionError("no generator")):
            rows = perf.inline_cases({"samples": [{"input": "1\n", "output": "2\n"}]}, Path(temporary))
            self.assertEqual(len(rows),1)
            self.assertEqual(rows[0]["metadata"]["size_profile"], "quick_smoke")

    def test_namespace_adapter_does_not_rewrite_algorithm_or_qualified_current_namespace(self):
        entry = {"id": "test", "libraries": ["Geometry/Point.cpp"], "problem_url": "https://judge.yosupo.jp/problem/aplusb", "allow_custom_io": True, "driver": "unused"}
        source, adapter = perf.render_program(entry, {"Geometry/Point.cpp": b"namespace geo { struct Point {}; }\n"}, "int main(){sirkit::geo::Point p;}\n", self.root, legacy_adapter=True)
        self.assertIn(b"namespace sirkit { namespace geo = ::geo; }",source)
        self.assertEqual(adapter["sha256"],perf.digest(adapter["source"].encode()))
        _, adapter = perf.render_program(entry, {"Geometry/Point.cpp": b"namespace sirkit { namespace geo { struct Point {}; } }\n"}, "int main(){}", self.root, legacy_adapter=True)
        self.assertEqual(adapter["kind"],"none")

    def test_archived_driver_requires_matching_problem_samples_and_confirmed_source(self):
        path = "Verify/current/drivers/example.cpp"
        old_driver = b"int main() { return 0; }\n"
        entry = {"id": "a", "driver": path, "libraries": ["DataStructure/a.cpp"],
                 "problem_url": "https://judge.yosupo.jp/problem/unionfind",
                 "samples": [{"input": "a", "output": "b"}]}
        archive = dict(entry, driver_sha256=perf.digest(old_driver))
        def baseline(_revision, name, _root):
            if name == path:
                return old_driver
            return json.dumps({"entries": [archive]}).encode()
        with patch.object(perf.ci_benchmark, "baseline_source", side_effect=baseline):
            text, proof = perf.archived_driver("base", entry, perf.digest(b"new"), {}, self.root)
            self.assertEqual(text.encode(), old_driver)
            self.assertFalse(proof["same_as_current"])
            self.assertEqual(proof["sha256"], perf.digest(old_driver))
            for fields in ({"samples": []}, {"driver_sha256": perf.digest(b"modified")},
                           {"problem_url": "https://judge.yosupo.jp/problem/associative_array"},
                           {"support_source_hashes": {"old-header": "changed"}}):
                with self.subTest(fields=fields):
                    archive = {**entry, "driver_sha256": perf.digest(old_driver), **fields}
                    self.assertIsNone(perf.archived_driver("base", entry, perf.digest(b"new"), {}, self.root))


class MeasurementTests(unittest.TestCase):
    def test_atcoder_baseline_constexpr_limit_is_distinct_from_other_build_errors(self):
        diagnostic = ("submission.cpp:167:45: error: 'constexpr' evaluation operation count exceeds limit "
                      "of 2097152 (use '-fconstexpr-ops-limit=' to increase the limit)\n")
        build = {"status": "FAIL", "timed_out": False, "diagnostic_excerpt": diagnostic}
        comparison = perf.baseline_build_comparison(build, "atcoder")
        self.assertEqual(comparison, "BASELINE_CONSTEXPR_LIMIT")
        self.assertEqual(perf.regression_result({"comparison": comparison})["status"], "NOT_COMPARABLE")
        self.assertEqual(perf.baseline_build_comparison(build, "standard"), "BASELINE_API_INCOMPATIBLE")
        build["diagnostic_excerpt"] += "submission.cpp:168:3: error: unknown name\n"
        self.assertEqual(perf.baseline_build_comparison(build, "atcoder"), "BASELINE_API_INCOMPATIBLE")

    @staticmethod
    def paired_result(name, baseline, current):
        result = {"name": name, "comparison": "MEASURED",
                  "cpu_samples_ns": {"base": [baseline] * 3, "current": [current] * 3}}
        return {**result, "performance_regression": perf.regression_result(result)}

    def test_losing_a_case_is_advisory_when_library_worst_passes(self):
        binaries = {"base": Path("base"), "current": Path("current")}
        fast = self.paired_result("fast", 5_000_000, 9_500_000)
        slow = self.paired_result("slow", 10_000_000, 9_000_000)
        row = {"cases": [fast, slow]}
        with patch.object(perf, "measure_case") as measure:
            perf.confirm_worst_case(row, binaries, row["cases"], {}, Path("work"), repetitions=3)
        measure.assert_not_called()
        self.assertEqual(fast["performance_regression"]["status"], "FAIL")
        self.assertNotIn("worst_case_confirmation", row)
        self.assertEqual(row["worst_case_regression"]["baseline_case"], "slow")
        self.assertEqual(row["worst_case_regression"]["observed_case"], "fast")
        self.assertEqual(perf.library_regression_status(row), "PASS")

    def test_worst_regression_repeats_all_paired_cases_in_reverse_with_same_options(self):
        binaries = {"base": Path("base"), "current": Path("current")}
        cases = [{"name": name} for name in ("slow", "fast", "unpaired")]
        options = {"repetitions": 3, "warmups": 1, "timeout": 10, "memory_mib": 64}
        for current, expected in ((10_500_000, "PASS"), (10_500_001, "FAIL")):
            first = [self.paired_result("slow", 10_000_000, 9_000_000),
                     self.paired_result("fast", 5_000_000, 11_000_000)]
            unpaired = {"name": "unpaired", "comparison": "BASELINE_CONSTEXPR_LIMIT"}
            unpaired["performance_regression"] = perf.regression_result(unpaired)
            row = {"cases": [*first, unpaired]}
            confirmed = [self.paired_result("slow", 10_000_000, current),
                         self.paired_result("fast", 5_000_000, 8_000_000)]
            with self.subTest(current=current), patch.object(perf, "measure_case", side_effect=confirmed) as measure:
                perf.confirm_worst_case(row, binaries, cases, {}, Path("work"), **options)
            self.assertEqual(measure.call_count, 2)
            for call, expected_case in zip(measure.call_args_list, cases[:2], strict=True):
                self.assertEqual(list(call.args[0]), ["current", "base"])
                self.assertEqual(call.args[1:], (expected_case, {}, Path("work")))
                self.assertEqual(call.kwargs, options)
            self.assertEqual(row["worst_case_confirmation"], perf.paired_cpu_samples(first))
            self.assertEqual(row["worst_case_regression"]["observed_case"], "slow")
            self.assertEqual(row["worst_case_regression"]["compared_cases"], 2)
            self.assertEqual(row["cases"][2], unpaired)
            self.assertEqual(perf.library_regression_status(row), expected)

    def test_unavailable_baseline_is_classified_by_recorded_cause(self):
        for comparison, failure, expected in (
                ("BASELINE_NEW_LIBRARY", {}, "NOT_COMPARABLE"),
                ("BASELINE_DEPENDENCY_MISSING", {}, "FAIL"),
                ("BASELINE_API_INCOMPATIBLE", {}, "FAIL"),
                ("BASELINE_INVALID", {"returncode": 0, "timed_out": False, "oracle_passed": False}, "NOT_COMPARABLE"),
                ("BASELINE_INVALID", {"returncode": -9, "timed_out": True, "oracle_passed": None}, "FAIL")):
            with self.subTest(comparison=comparison, failure=failure):
                gate = perf.regression_result({"comparison": comparison, "baseline_failure": failure})
                self.assertEqual(gate, {"status": expected, "metric": "child_cpu_ns", "reason": comparison})

    def test_unavailable_confirmation_keeps_initial_regression_failure(self):
        first = self.paired_result("case", 200_000, 900_000)
        second = {"name": "case", "comparison": "BASELINE_INVALID", "baseline_failure": {
            "returncode": 0, "timed_out": False, "oracle_passed": False}}
        row = {"cases": [first]}
        binaries = {"base": Path("base"), "current": Path("current")}
        with patch.object(perf, "measure_case", return_value=second):
            perf.confirm_worst_case(row, binaries, [{"name": "case"}], {}, Path("work"))
        self.assertEqual(row["cases"][0]["performance_regression"], {"status": "FAIL", "metric": "child_cpu_ns",
                         "reason": "BASELINE_CONFIRMATION_UNAVAILABLE"})
        self.assertEqual(row["worst_case_confirmation"], {"case": first["cpu_samples_ns"]})
        self.assertNotIn("worst_case_regression", row)
        self.assertEqual(perf.library_regression_status(row), "FAIL")

    def test_library_status_preserves_comparison_errors_and_current_failures(self):
        paired = self.paired_result("pair", 500_000, 500_000)
        for comparison, expected in (("BASELINE_CONSTEXPR_LIMIT", "PASS"), ("BASELINE_BUILD_TIMEOUT", "FAIL")):
            case_result = {"name": "missing", "comparison": comparison}
            case_result["performance_regression"] = perf.regression_result(case_result)
            row = {"cases": [paired, case_result], "worst_case_regression": {"status": "PASS"}}
            self.assertEqual(perf.library_regression_status(row), expected)
        case_result = {"name": "missing", "comparison": "BASELINE_NEW_LIBRARY"}
        case_result["performance_regression"] = perf.regression_result(case_result)
        row = {"cases": [case_result]}
        self.assertEqual(perf.library_regression_status(row), "NOT_COMPARABLE")
        self.assertEqual(perf.library_regression_status({"status": "FAIL", **row}), "FAIL")

    def test_absolute_current_time_and_cpu_samples_are_separate(self):
        with tempfile.TemporaryDirectory() as temporary:
            work = Path(temporary); profile = case(work / "input")
            def execute(command, **kwargs):
                kwargs["output"].write_bytes(b"42\n")
                return {"returncode":0,"timed_out":False,"elapsed_ns":100,"cpu_ns":50}
            with patch.object(perf,"execute",side_effect=execute):
                row=perf.measure_case({"current":Path("current")},profile,{},work,repetitions=3,warmups=1,timeout=1,memory_mib=64)
            self.assertEqual(row["samples_ns"],{"current":[100]*3})
            self.assertEqual(row["cpu_samples_ns"],{"current":[50]*3})
            self.assertIsNone(row["current_over_base"])

    def test_invalid_baseline_does_not_create_ratio_or_fail_current(self):
        with tempfile.TemporaryDirectory() as temporary:
            work = Path(temporary); profile = case(work / "input")
            def execute(command, **kwargs):
                kwargs["output"].write_bytes(b"99\n" if command[0]=="base" else b"42\n")
                return {"returncode":0,"timed_out":False,"elapsed_ns":100,"cpu_ns":50}
            with patch.object(perf,"execute",side_effect=execute):
                row=perf.measure_case({"base":Path("base"),"current":Path("current")},profile,{},work,repetitions=2,warmups=0,timeout=1,memory_mib=64)
            self.assertEqual(row["comparison"],"BASELINE_INVALID")
            self.assertEqual(set(row["samples_ns"]),{"current"})
            self.assertIsNone(row["current_over_base"])
            self.assertTrue(row["correctness_passed"])

    def test_wrong_or_missing_current_result_is_failure(self):
        with tempfile.TemporaryDirectory() as temporary:
            work=Path(temporary);profile=case(work/"input")
            def bad(command,**kwargs):
                kwargs["output"].write_bytes(b"99\n")
                return {"returncode":0,"timed_out":False,"elapsed_ns":100}
            with patch.object(perf,"execute",side_effect=bad),self.assertRaises(perf.PerformanceError):
                perf.measure_case({"current":Path("current")},profile,{},work,repetitions=1,warmups=0,timeout=1,memory_mib=64)
            actual=work/"empty";actual.write_bytes(b"");Path(profile["expected_path"]).write_bytes(b"");profile["expected_sha256"]=perf.file_digest(Path(profile["expected_path"]))
            self.assertFalse(perf.output_matches(actual,profile,{},work))
            profile["allow_empty_output"]=True
            self.assertTrue(perf.output_matches(actual,profile,{},work))
            Path(profile["expected_path"]).write_bytes(b" \n\t");profile["expected_sha256"]=perf.file_digest(Path(profile["expected_path"]))
            profile.pop("allow_empty_output")
            self.assertFalse(perf.output_matches(actual,profile,{},work))

    def test_official_checker_error_is_distinct_from_rejected_answer(self):
        with tempfile.TemporaryDirectory() as temporary:
            work = Path(temporary)
            profile = case(work / "input")
            checker = work / "checker"
            checker.write_bytes(b"checker")
            profile.update(checker="official", checker_path=str(checker), checker_sha256=perf.file_digest(checker))
            for returncode, timed_out in ((-11, False), (0, True), (3, False)):
                with self.subTest(returncode=returncode, timed_out=timed_out):
                    with patch.object(perf, "execute", return_value={"returncode": returncode, "timed_out": timed_out}):
                        with self.assertRaisesRegex(perf.PerformanceError, "official checker failed"):
                            perf.output_matches(work / "actual", profile, {}, work)
            with patch.object(perf, "execute", return_value={"returncode": 1, "timed_out": False}):
                self.assertFalse(perf.output_matches(work / "actual", profile, {}, work))

    def test_wait4_captures_actual_child_cpu_without_polling_wait_timeout(self):
        with tempfile.TemporaryDirectory() as temporary:
            work=Path(temporary);process=Mock(pid=123,returncode=None);timer=Mock()
            with patch.object(perf.subprocess,"Popen",return_value=process), \
                 patch.object(perf.threading,"Timer",return_value=timer), \
                 patch.object(perf.os,"wait4",return_value=(123,0,SimpleNamespace(ru_utime=.005,ru_stime=.003))):
                row=perf.execute(["fake"],work=work,output=work/"out",log=work/"log",timeout=1,memory_mib=64)
            self.assertEqual(row["cpu_ns"],8_000_000)
            self.assertEqual(row["returncode"],0)
            self.assertFalse(row["timed_out"])
            process.wait.assert_not_called()
            timer.cancel.assert_called_once();timer.join.assert_called_once()

    def test_timeout_only_kills_dedicated_child_group(self):
        with tempfile.TemporaryDirectory() as temporary:
            work=Path(temporary);process=Mock(pid=123,returncode=None)
            def timer(timeout,callback):
                obj=Mock();obj.start.side_effect=callback;return obj
            with patch.object(perf.subprocess,"Popen",return_value=process), \
                 patch.object(perf.threading,"Timer",side_effect=timer), \
                 patch.object(perf.os,"wait4",return_value=(123,signal.SIGKILL,SimpleNamespace(ru_utime=0,ru_stime=0))), \
                 patch.object(perf.os,"killpg")as kill:
                row=perf.execute(["fake"],work=work,output=work/"out",log=work/"log",timeout=1,memory_mib=64)
            kill.assert_called_once_with(123,signal.SIGKILL)
            self.assertTrue(row["timed_out"])
            self.assertEqual(row["returncode"],-signal.SIGKILL)


class CompletionTests(unittest.TestCase):
    def good(self):
        return {"selected_library_paths":["A.cpp"],"repetitions":2,"source_freeze_passed":True,
                "results":[{"library":"A.cpp","status":"PASS","current_measurements_passed":True,
                 "expected_case_names":["one"],"cases":[{"name":"one","status":"PASS","correctness_passed":True,"samples_ns":{"current":[1,2]}}]}]}

    def test_missing_library_case_repetition_or_freeze_never_pass(self):
        self.assertTrue(perf.complete(self.good()))
        mutations=[lambda d:d.update(results=[]),lambda d:d.update(source_freeze_passed=False),
                   lambda d:d["results"][0].update(status="NOT_RUN"),
                   lambda d:d["results"][0].update(cases=[]),
                   lambda d:d["results"][0]["cases"][0].update(correctness_passed=False),
                   lambda d:d["results"][0]["cases"][0].update(samples_ns={"current":[1]}),
                   lambda d:d["results"][0]["cases"][0].update(samples_ns={"current":[0,1]})]
        for change in mutations:
            data=self.good();change(data);self.assertFalse(perf.complete(data))

    def test_nonlc_expected_case_set_is_reconstructed_from_current_catalog(self):
        data=self.good();data["size_profile"]="full_representative"
        data["results"][0].update(verify_id="a",case_source="deterministic_non_lc")
        generator=SimpleNamespace(plans_for_library=lambda library,quick=False:[{"name":"one"},{"name":"missing_max"}])
        self.assertFalse(perf.complete(data,entries={"a":{}},nonlc=generator))

    def test_official_self_declared_case_subset_is_not_complete(self):
        data=self.good();data["size_profile"]="full_representative"
        data["results"][0].update(verify_id="a",case_source="official_library_checker",
            official={"expected_cases":["one","killer_00"]},official_case_input_bytes={"one":1,"killer_00":100})
        self.assertFalse(perf.complete(data,entries={"a":{}},nonlc=None))

    def test_failed_generation_without_case_manifest_keeps_original_error(self):
        data=self.good();data["size_profile"]="full_representative"
        row=data["results"][0]
        row.update(verify_id="a",case_source="official_library_checker",status="FAIL",
                   current_measurements_passed=False,error="official_generation_unavailable",cases=[])
        row.pop("expected_case_names")
        before=dict(row)
        with patch.object(perf,"official_selection",side_effect=AssertionError("failed row has no cases")):
            self.assertFalse(perf.complete(data,entries={"a":{}},nonlc=None))
        self.assertEqual(row,before)

    def test_successful_official_row_requires_nonempty_manifest(self):
        data=self.good();data["size_profile"]="full_representative"
        row=data["results"][0]
        row.update(verify_id="a",case_source="official_library_checker")
        for value in ({}, {"official": {"expected_cases": []}, "official_case_input_bytes": {}}):
            row.update(value)
            self.assertFalse(perf.complete(data,entries={"a":{}},nonlc=None))

    def test_duplicate_declared_case_names_are_not_complete(self):
        data=self.good();row=data["results"][0];row["cases"]*=2;row["expected_case_names"]*=2
        self.assertFalse(perf.complete(data))


class RunnerTests(unittest.TestCase):
    def test_generation_failure_report_survives_completion_and_cleanup(self):
        with tempfile.TemporaryDirectory()as temporary:
            folder=Path(temporary);root=folder/"repo";root.mkdir()
            library=root/"A.cpp";library.write_text("struct A {};\n")
            (root/"driver.cpp").write_text("int main(){}\n")
            entry={"id":"a","libraries":["A.cpp"],"driver":"driver.cpp","allow_custom_io":True,
                   "problem_url":"https://judge.yosupo.jp/problem/unionfind"}
            rows=[{"library":"A.cpp","verify_id":"a","libraries":["A.cpp"],
                   "case_source":"official_library_checker","problem_id":"unionfind"}]
            args=SimpleNamespace(cache=folder/"cache",output=folder/"reports",base=None,compiler="fake",
                repetitions=1,warmups=0,quick=False,plan=False,library=None,shard_index=0,shard_count=1,
                compile_timeout=1,case_timeout=1,generation_timeout=1,memory_mib=64,
                limits=fixed_limits(folder,library="A.cpp"))
            generation=perf.official.RunnerError("official_generation_unavailable")
            generation.details={"returncode":-11,"diagnostic_excerpt":"reference failed"}
            cleanup=perf.official.RunnerError("test_data_cleanup_unavailable")
            cleanup.details={"phase":"cleanup"}
            with patch.object(perf,"ROOT",root),patch.object(perf,"nonlc_module",return_value=SimpleNamespace()), \
                 patch.object(perf.prepare_verify,"collect",return_value=[entry]),patch.object(perf,"catalog",return_value=rows), \
                 patch.object(perf.shutil,"which",return_value="/bin/true"), \
                 patch.object(perf.subprocess,"check_output",return_value="fake compiler\n"), \
                 patch.object(perf.subprocess,"Popen",side_effect=AssertionError("no children")), \
                 patch.object(perf,"freeze_sources",return_value={}),patch.object(perf,"check_frozen"), \
                 patch.object(perf.official,"load_pin",return_value={}), \
                 patch.object(perf,"build",return_value={"status":"PASS","binary":"fake"}), \
                 patch.object(perf.official,"checkout_official",return_value=folder/"checkout"), \
                 patch.object(perf.official,"official_problem",side_effect=generation), \
                 patch.object(perf.official,"find_problem",return_value=folder/"problem"), \
                 patch.object(perf.official,"discard_test_data",side_effect=cleanup):
                self.assertEqual(perf.run(args),1)
            report=json.loads((args.output/"result.json").read_text());row=report["results"][0]
            self.assertEqual(report["status"],"FAIL")
            self.assertNotIn("error",report)
            self.assertEqual(row["error"],"official_generation_unavailable")
            self.assertEqual(row["error_details"],generation.details)
            self.assertEqual(row["cleanup_error_details"],cleanup.details)
            self.assertFalse(report["current_measurements_passed"])

    def test_generation_diagnostics_and_cleanup_failure_preserve_original_cause(self):
        generation=perf.official.RunnerError("official_generation_unavailable")
        generation.details={"returncode":-11,"diagnostic_excerpt":"reference received SIGSEGV","stack_bytes":67108864}
        row={}
        perf.record_failure(row,generation)
        cleanup=perf.official.RunnerError("test_data_cleanup_unavailable")
        cleanup.details={"phase":"cleanup"}
        perf.record_failure(row,cleanup,key="cleanup_error")
        self.assertEqual(row["status"],"FAIL")
        self.assertEqual(row["error"],"official_generation_unavailable")
        self.assertEqual(row["error_details"],generation.details)
        self.assertEqual(row["cleanup_error_details"],cleanup.details)
        report={}
        perf.record_failure(report,generation)
        self.assertEqual(report["error_details"],generation.details)
        self.assertIsNot(report["error_details"],generation.details)

    def test_real_103_library_plan_spawns_no_children(self):
        with tempfile.TemporaryDirectory()as temporary,redirect_stdout(io.StringIO())as stream, \
             patch.object(perf.subprocess,"Popen",side_effect=AssertionError("plan must not spawn children")), \
             patch.object(perf.official,"official_problem",side_effect=AssertionError("no generator")):
            folder=Path(temporary)
            self.assertEqual(perf.main(["--plan","--cache",str(folder/"cache"),"--output",str(folder/"report")]),0)
            report=json.loads(stream.getvalue())
            self.assertEqual(len(report["all_library_paths"]),103)
            self.assertEqual(report["all_library_paths"],report["selected_library_paths"])
            self.assertFalse(report["executed"])

    def run_fake_quick(self, folder, *, cpu_ns=50, wall_ns=100, missing_budget=False, fixed_budget=True,
                       compile_profile="standard", paired_cpu_ns=None):
        folder=Path(folder);root=folder/"repo";root.mkdir();library=root/"DataStructure/A.cpp";library.parent.mkdir();library.write_text("struct A {};\n")
        driver=root/"driver.cpp";driver.write_text("int main(){}\n")
        num_inputs = len(paired_cpu_ns) if paired_cpu_ns is not None else 1
        samples = [{"input": f"{index+1}\n", "output": "42\n"} for index in range(num_inputs)]
        entry={"id":"a","libraries":["DataStructure/A.cpp"],"driver":"driver.cpp","allow_custom_io":True,"problem_url":"https://judge.yosupo.jp/problem/unionfind","samples":samples}
        rows=[{"library":"DataStructure/A.cpp","verify_id":"a","libraries":entry["libraries"],"case_source":"official_library_checker","problem_id":"unionfind","combined_library_scope":False}]
        base = "base" if paired_cpu_ns is not None else None
        args=SimpleNamespace(cache=folder/"cache",output=folder/"reports",base=base,compiler="fake",repetitions=2,warmups=0,quick=True,plan=False,library=None,shard_index=0,shard_count=1,compile_timeout=1,case_timeout=1,generation_timeout=1,memory_mib=64,
            compile_profile=compile_profile,
            limits=fixed_limits(folder,name="other_case" if missing_budget else "registered_sample_00") if fixed_budget else None)
        before=args.limits.read_bytes() if args.limits is not None else None
        def build(source,compiler,directory,timeout,flags):
            directory.mkdir(parents=True,exist_ok=True);binary=directory/"binary";binary.write_bytes(b"fake")
            return {"status":"PASS","binary":str(binary),"source_sha256":perf.digest(source),"flags":flags}
        def execute(command,**kwargs):
            kwargs["output"].write_bytes(b"42\n")
            elapsed_cpu = cpu_ns
            if paired_cpu_ns is not None:
                index = int(kwargs["stdin"].read_text()) - 1
                elapsed_cpu = paired_cpu_ns[index][Path(command[0]).parent.name]
            return {"returncode":0,"timed_out":False,"elapsed_ns":wall_ns,"cpu_ns":elapsed_cpu}
        baseline = ({"DataStructure/A.cpp": b"struct OldA {};\n"}, {}) if base is not None else (None, {})
        with patch.object(perf,"ROOT",root),patch.object(perf,"nonlc_module",return_value=SimpleNamespace()), \
             patch.object(perf.prepare_verify,"collect",return_value=[entry]),patch.object(perf,"catalog",return_value=rows), \
             patch.object(perf.shutil,"which",return_value="/bin/true"), \
             patch.object(perf.subprocess,"check_output",return_value="fake compiler\n"), \
             patch.object(perf,"freeze_sources",return_value={}),patch.object(perf,"check_frozen"), \
             patch.object(perf.official,"load_pin",return_value={}),patch.object(perf,"build",side_effect=build), \
             patch.object(perf,"execute",side_effect=execute) as executed, \
             patch.object(perf.ci_benchmark,"resolve_base",return_value="a"*40 if base is not None else None), \
             patch.object(perf,"baseline_libraries",return_value=baseline), \
             patch.object(perf.official,"checkout_official",side_effect=AssertionError("no network")), \
             patch.object(perf.official,"official_problem",side_effect=AssertionError("no Max generator")):
            result=perf.run(args)
        report=json.loads((args.output/"result.json").read_text())
        if args.limits is not None:
            self.assertEqual(args.limits.read_bytes(),before)
        return result,report,executed.call_count

    def test_shard_counts_one_worst_case_decision_per_library(self):
        for current, expected, executions in ((9_500_000, "PASS", 8), (10_600_000, "FAIL", 16)):
            timings = [{"base": 10_000_000, "current": 9_000_000}, {"base": 5_000_000, "current": current}]
            with self.subTest(current=current), tempfile.TemporaryDirectory() as temporary:
                result, report, calls = self.run_fake_quick(temporary, fixed_budget=False, paired_cpu_ns=timings)
                summary = (Path(temporary) / "reports/summary.md").read_text()
            self.assertEqual(calls, executions)
            self.assertEqual(result, int(expected == "FAIL"))
            self.assertEqual(report["regression_policy"], "library_worst_case")
            self.assertEqual(report["performance_regression_counts"], {expected: 1})
            row = report["results"][0]
            self.assertEqual(row["cases"][1]["performance_regression"]["status"], "FAIL")
            self.assertEqual(row["worst_case_regression"]["observed_case"], "registered_sample_01")
            self.assertIn("個別比較 FAIL（参考）", summary)
            self.assertIn("基準の最大ケース", summary)

    def test_current_only_quick_uses_explicit_budget_without_git_or_official_generator(self):
        with tempfile.TemporaryDirectory()as temporary:
            result,report,_=self.run_fake_quick(temporary)
        self.assertEqual(result,0)
        self.assertEqual(report["status"],"PASS")
        self.assertIsNone(report["base_revision"])
        self.assertEqual(report["results"][0]["cases"][0]["comparison"],"NOT_REQUESTED")
        self.assertIn("cpu_model",report)
        self.assertTrue(report["source_freeze_passed"])
        self.assertTrue(report["performance_limits_passed"])

    def test_current_only_without_explicit_budget_records_no_performance_gate(self):
        with (
            tempfile.TemporaryDirectory() as temporary,
            patch.object(perf.performance_limits, "load", side_effect=AssertionError("no fixed budget")),
        ):
            result, report, _ = self.run_fake_quick(temporary, fixed_budget=False, compile_profile="atcoder")
        self.assertEqual(result, 0)
        self.assertEqual(report["compile_profile"], "atcoder")
        self.assertIn("-std=gnu++23", report["flags"])
        self.assertIn("-march=native", report["flags"])
        self.assertIn("-DNDEBUG", report["flags"])
        self.assertEqual(report["results"][0]["current_build"]["flags"], report["flags"])
        self.assertTrue(report["current_measurements_passed"])
        self.assertNotIn("performance_limits_passed", report)
        self.assertNotIn("performance_regression_passed", report)
        self.assertNotIn("performance_limit", report["results"][0]["cases"][0])

    def test_gate_uses_child_cpu_not_startup_or_runner_wall_time(self):
        for cpu_ns,expected in ((9_000_000,0),(10_000_000,0),(10_000_001,1)):
            with self.subTest(cpu_ns=cpu_ns),tempfile.TemporaryDirectory()as temporary:
                result,report,_=self.run_fake_quick(temporary,cpu_ns=cpu_ns,wall_ns=100_000_000)
            self.assertEqual(result,expected)
            self.assertEqual(report["status"],"FAIL" if expected else "PASS")
            self.assertTrue(report["current_measurements_passed"])
            row=report["results"][0]["cases"][0]
            self.assertEqual(row["status"],"PASS")
            self.assertTrue(row["correctness_passed"])
            self.assertEqual(row["median_ns"]["current"],100_000_000)
            self.assertEqual(row["performance_limit"]["observed_median_ns"],cpu_ns)
            self.assertEqual(row["performance_limit"]["status"],"FAIL" if expected else "PASS")

    def test_quick_mode_does_not_bypass_missing_fixed_budget(self):
        with tempfile.TemporaryDirectory()as temporary:
            result,report,executions=self.run_fake_quick(temporary,missing_budget=True)
        self.assertEqual(result,1)
        self.assertEqual(report["status"],"FAIL")
        self.assertIn("固定性能予算がありません",report["results"][0]["error"])
        self.assertEqual(executions,0)


if __name__=="__main__":
    unittest.main()
