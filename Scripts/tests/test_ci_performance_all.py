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


class MeasurementTests(unittest.TestCase):
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

    def test_duplicate_declared_case_names_are_not_complete(self):
        data=self.good();row=data["results"][0];row["cases"]*=2;row["expected_case_names"]*=2
        self.assertFalse(perf.complete(data))


class RunnerTests(unittest.TestCase):
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

    def test_current_only_quick_uses_no_git_baseline_or_official_generator(self):
        with tempfile.TemporaryDirectory()as temporary:
            folder=Path(temporary);root=folder/"repo";root.mkdir();library=root/"DataStructure/A.cpp";library.parent.mkdir();library.write_text("struct A {};\n")
            driver=root/"driver.cpp";driver.write_text("int main(){}\n")
            entry={"id":"a","libraries":["DataStructure/A.cpp"],"driver":"driver.cpp","allow_custom_io":True,"problem_url":"https://judge.yosupo.jp/problem/unionfind","samples":[{"input":"1\n","output":"42\n"}]}
            rows=[{"library":"DataStructure/A.cpp","verify_id":"a","libraries":entry["libraries"],"case_source":"official_library_checker","problem_id":"unionfind","combined_library_scope":False}]
            args=SimpleNamespace(cache=folder/"cache",output=folder/"reports",base=None,compiler="fake",repetitions=2,warmups=0,quick=True,plan=False,library=None,shard_index=0,shard_count=1,compile_timeout=1,case_timeout=1,generation_timeout=1,memory_mib=64)
            def build(source,compiler,directory,timeout):
                directory.mkdir(parents=True,exist_ok=True);binary=directory/"binary";binary.write_bytes(b"fake")
                return {"status":"PASS","binary":str(binary),"source_sha256":perf.digest(source)}
            def execute(command,**kwargs):
                kwargs["output"].write_bytes(b"42\n");return {"returncode":0,"timed_out":False,"elapsed_ns":100,"cpu_ns":50}
            with patch.object(perf,"ROOT",root),patch.object(perf,"nonlc_module",return_value=SimpleNamespace()), \
                 patch.object(perf.prepare_verify,"collect",return_value=[entry]),patch.object(perf,"catalog",return_value=rows), \
                 patch.object(perf.shutil,"which",return_value="/bin/true"), \
                 patch.object(perf.subprocess,"check_output",return_value="fake compiler\n"), \
                 patch.object(perf,"freeze_sources",return_value={}),patch.object(perf,"check_frozen"), \
                 patch.object(perf.official,"load_pin",return_value={}),patch.object(perf,"build",side_effect=build), \
                 patch.object(perf,"execute",side_effect=execute), \
                 patch.object(perf.ci_benchmark,"resolve_base",side_effect=AssertionError("no base requested")), \
                 patch.object(perf.official,"checkout_official",side_effect=AssertionError("no network")), \
                 patch.object(perf.official,"official_problem",side_effect=AssertionError("no Max generator")):
                self.assertEqual(perf.run(args),0)
            report=json.loads((args.output/"result.json").read_text())
            self.assertEqual(report["status"],"PASS")
            self.assertIsNone(report["base_revision"])
            self.assertEqual(report["results"][0]["cases"][0]["comparison"],"NOT_REQUESTED")
            self.assertIn("cpu_model",report)
            self.assertTrue(report["source_freeze_passed"])


if __name__=="__main__":
    unittest.main()
