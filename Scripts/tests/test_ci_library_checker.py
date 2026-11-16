# SPDX-License-Identifier: CC0-1.0
import hashlib
import json
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import ci_library_checker as lc


PIN = {"repository": "https://github.com/yosupo06/library-checker-problems.git",
       "revision": "a" * 40,
       "license_url": "https://github.com/yosupo06/library-checker-problems/blob/" + "a" * 40 + "/LICENSE"}


class CacheBoundaryTests(unittest.TestCase):
    def setUp(self):
        self.directory = tempfile.TemporaryDirectory()
        self.addCleanup(self.directory.cleanup)
        self.base = Path(self.directory.name)
        self.root = self.base / "repo"
        self.root.mkdir()

    def test_reject_inside_cache_and_symlink_before_creating_directories(self):
        alias = self.base / "alias"
        alias.symlink_to(self.root, target_is_directory=True)
        for name in [self.root, self.root / "new", alias / "new"]:
            with self.subTest(name=name), self.assertRaisesRegex(ValueError, "リポジトリ外"):
                lc.external_cache(name, self.base / "reports", self.root)
        self.assertFalse((self.root / "new").exists())
        self.assertFalse((self.base / "reports").exists())

    def test_output_and_cache_cannot_overlap(self):
        for cache, output in [(self.base / "cache", self.base / "cache/out"),
                              (self.base / "out/cache", self.base / "out")]:
            with self.assertRaisesRegex(ValueError, "別ディレクトリ"):
                lc.external_cache(cache, output, self.root)

    def test_derived_cache_symlink_cannot_escape(self):
        cache = self.base / "cache"
        cache.mkdir()
        (cache / "runs").symlink_to(self.root, target_is_directory=True)
        with self.assertRaisesRegex(ValueError, "別の場所"):
            lc.cache_path(cache.resolve(), "runs", "new")

    def test_main_does_not_download_with_invalid_cache(self):
        with patch.object(lc, "ROOT", self.root), patch.object(lc, "load_pin") as pin:
            # external_cacheのrootは関数定義時のROOTなので、こちらも実rootを明示する。
            original = lc.external_cache
            with patch.object(lc, "external_cache", side_effect=lambda c, o: original(c, o, self.root)):
                result = lc.main(["--cache", str(self.root / "cache"), "--output", str(self.base / "reports")])
        self.assertEqual(result, 1)
        pin.assert_not_called()
        self.assertFalse((self.root / "cache").exists())

    def test_pin_must_use_official_url_and_full_commit(self):
        file = self.base / "pin.json"
        file.write_text(json.dumps(PIN))
        self.assertEqual(lc.load_pin(file), PIN)
        for key, value in [("revision", "main"), ("repository", "https://example.invalid/repo"),
                           ("license_url", "https://example.invalid/LICENSE")]:
            altered = PIN | {key: value}
            file.write_text(json.dumps(altered))
            with self.assertRaises(ValueError):
                lc.load_pin(file)

    def test_nonfinite_timeout_is_rejected_before_network(self):
        with patch.object(lc, "load_pin") as pin:
            result = lc.main(["--cache", str(self.base / "cache"), "--output", str(self.base / "reports"),
                              "--case-timeout", "nan"])
        self.assertEqual(result, 1)
        pin.assert_not_called()

    def test_cached_revision_mismatch_is_not_reset_or_pulled(self):
        cache = self.base / "cache"
        (cache / "official" / PIN["revision"]).mkdir(parents=True)
        with patch.object(lc, "git_text", return_value="b" * 40), patch.object(lc, "execute") as execute:
            with self.assertRaisesRegex(lc.RunnerError, "revision_mismatch"):
                lc.checkout_official(cache.resolve(), PIN)
        execute.assert_not_called()


class SelectionTests(unittest.TestCase):
    def setUp(self):
        self.entries = [{"id": "a", "problem_url": "https://judge.yosupo.jp/problem/unionfind"},
                        {"id": "b", "problem_url": "https://atcoder.jp/contests/a/tasks/b"},
                        {"id": "c", "problem_url": "https://judge.yosupo.jp/problem/aplusb"}]

    def test_all_and_shards_cover_library_checker_once(self):
        plans = [lc.selection(self.entries, [], None, True, i, 3) for i in range(3)]
        self.assertEqual([p["selected_ids"] for p in plans], [["a"], ["c"], []])
        self.assertEqual(plans[0]["excluded_non_library_checker_ids"], ["b"])
        self.assertEqual(plans[0]["all_selected_ids"], ["a", "c"])

    def test_runner_change_selects_all_and_non_lc_is_explicitly_excluded(self):
        plan = lc.selection(self.entries, ["Scripts/ci_library_checker.py"], None, False, 0, 1)
        self.assertEqual(plan["selected_ids"], ["a", "c"])
        only_other = lc.selection(self.entries, [], ["b"], False, 0, 1)
        self.assertEqual(only_other["selected_ids"], [])
        self.assertEqual(only_other["excluded_non_library_checker_ids"], ["b"])

    def test_invalid_ids_shards_and_urls_fail_closed(self):
        for index, count in [(0, 0), (-1, 2), (2, 2)]:
            with self.assertRaises(ValueError):
                lc.selection(self.entries, [], None, True, index, count)
        with self.assertRaises(ValueError):
            lc.selection([{"id": "../escape"}], [], None, True, 0, 1)
        for url in ["https://judge.yosupo.jp.evil/problem/unionfind", "https://judge.yosupo.jp/problem/../x"]:
            self.assertIsNone(lc.problem_id({"problem_url": url}))


class OfficialCaseTests(unittest.TestCase):
    def setUp(self):
        self.directory = tempfile.TemporaryDirectory()
        self.addCleanup(self.directory.cleanup)
        self.cache = Path(self.directory.name).resolve()
        self.checkout = self.cache / "checkout"
        self.problem = self.checkout / "owned_problem"
        self.problem.mkdir(parents=True)
        (self.problem / "info.toml").write_text("timelimit = 1.0\n[[tests]]\nname='example.in'\nnumber=1\n")
        (self.problem / "checker.cpp").write_text("// own unit fixture\n")
        self.hashes = {}
        for subdir, extension, text in [("in", ".in", "own input\n"), ("out", ".out", "canonical\n")]:
            path = self.problem / subdir / ("example_00" + extension)
            path.parent.mkdir()
            path.write_text(text)
            self.hashes[path.name] = lc.digest(path)
        (self.problem / "hash.json").write_text(json.dumps(self.hashes))
        self.script(self.problem / "checker", "import sys\nsys.exit(0)\n")

    @staticmethod
    def script(path, body):
        path.write_text("#!" + sys.executable + "\n" + body)
        path.chmod(0o700)

    def test_generated_case_set_and_hashes_are_checked(self):
        with patch.object(lc, "execute", return_value=(0, 1)):
            _, metadata = lc.official_problem(self.checkout, "owned_problem", self.cache, PIN, 10)
            self.assertEqual(metadata["expected_cases"], ["example_00"])
            self.assertEqual(metadata["checker_sha256"], lc.digest(self.problem / "checker"))
            (self.problem / "in/example_00.in").write_text("tampered\n")
            with self.assertRaisesRegex(lc.RunnerError, "case_hash_mismatch"):
                lc.official_problem(self.checkout, "owned_problem", self.cache, PIN, 10)
            (self.problem / "in/example_00.in").unlink()
            with self.assertRaisesRegex(lc.RunnerError, "case_set_mismatch"):
                lc.official_problem(self.checkout, "owned_problem", self.cache, PIN, 10)

    def test_empty_and_duplicate_case_sets_are_not_successful(self):
        for data in [{"tests": []}, {"tests": [{"name": "a.in", "number": 0}]},
                     {"tests": [{"name": "a.in", "number": 1}, {"name": "a.cpp", "number": 1}]}]:
            with self.assertRaises(lc.RunnerError):
                lc.expected_cases(data)

    def test_official_checker_accepts_nonidentical_witness(self):
        binary = self.cache / "own_solution"
        self.script(binary, "print('alternative')\n")
        self.script(self.problem / "checker", "import pathlib,sys\n"
                    "assert pathlib.Path(sys.argv[1]).read_text() == 'own input\\n'\n"
                    "assert pathlib.Path(sys.argv[2]).read_text() == 'alternative\\n'\n"
                    "assert pathlib.Path(sys.argv[3]).read_text() == 'canonical\\n'\n")
        row = lc.run_case(binary, self.problem, "example_00", self.cache, 5)
        self.assertEqual(row["status"], "PASS")
        self.assertEqual(row["checker_returncode"], 0)
        self.assertGreater(row["elapsed_ns"], 0)
        self.assertNotIn("alternative", json.dumps(row))

    def test_checker_rejection_and_solution_error_fail(self):
        binary = self.cache / "own_solution"
        self.script(binary, "print('canonical')\n")
        self.script(self.problem / "checker", "import sys\nsys.exit(1)\n")
        row = lc.run_case(binary, self.problem, "example_00", self.cache, 5)
        self.assertEqual(row["status"], "FAIL")
        self.assertEqual(row["error_kind"], "checker_rejected")
        self.script(binary, "import sys\nsys.exit(7)\n")
        row = lc.run_case(binary, self.problem, "example_00", self.cache, 5)
        self.assertEqual(row["error_kind"], "runtime_error")
        self.assertIsNone(row["checker_returncode"])

    def test_timeout_does_not_run_checker(self):
        binary = self.cache / "own_solution"
        self.script(binary, "import time\ntime.sleep(60)\n")
        row = lc.run_case(binary, self.problem, "example_00", self.cache, 0.02)
        self.assertEqual(row["status"], "FAIL")
        self.assertEqual(row["error_kind"], "time_limit")
        self.assertIsNone(row["returncode"])
        self.assertIsNone(row["checker_returncode"])


class ReportTests(unittest.TestCase):
    def setUp(self):
        self.directory = tempfile.TemporaryDirectory()
        self.addCleanup(self.directory.cleanup)
        self.root = Path(self.directory.name).resolve()
        self.cache = self.root / "cache"
        self.cache.mkdir()
        self.output = self.root / "metadata"
        self.entries = [{"id": "a", "problem_url": "https://judge.yosupo.jp/problem/unionfind"}]
        self.plan = {"selected_ids": ["a"]}
        self.submission = self.root / "Verify/current/submissions/a.cpp"
        self.submission.parent.mkdir(parents=True)
        self.submission.write_bytes(b"// own sample source\n")

    def provenance(self, entry, root):
        return {"id": entry["id"], "libraries": ["DataStructure/A.cpp"],
                "source_hashes": {"DataStructure/A.cpp": "b" * 64},
                "submission": "Verify/current/submissions/a.cpp", "submission_sha256": lc.digest(self.submission),
                "driver": "Verify/current/drivers/a.cpp", "driver_sha256": "d" * 64,
                "support_source_hashes": {}, "io_source_hashes": {}, "problem_url": entry["problem_url"]}

    def compile(self, command, **kwargs):
        Path(command[-1]).write_bytes(b"own test executable placeholder")
        return 0, 1

    def run_with(self, cases):
        details = PIN | {"expected_cases": ["a", "b"], "checker_sha256": "e" * 64}
        with patch.object(lc.subprocess, "run", return_value=subprocess.CompletedProcess([], 0, "own compiler\n")), \
             patch.object(lc, "checkout_official", return_value=self.cache), \
             patch.object(lc, "official_problem", return_value=(self.cache, details)), \
             patch.object(lc, "provenance", side_effect=self.provenance), \
             patch.object(lc, "execute", side_effect=self.compile), \
             patch.object(lc, "run_case", side_effect=cases):
            return lc.run_selected(self.entries, self.plan, self.cache, self.output, PIN, root=self.root)

    def test_pass_requires_all_nonempty_cases_and_metadata_only(self):
        cases = [{"name": n, "status": "PASS", "elapsed_ns": 123,
                  "returncode": 0, "checker_returncode": 0} for n in ["a", "b"]]
        report = self.run_with(cases)
        self.assertEqual(report["status"], "PASS")
        self.assertEqual(report["online_status"], "not_evaluated")
        self.assertEqual(report["counts"]["case_passed"], 2)
        self.assertEqual(report["results"][0]["elapsed_ns"], 246)
        snapshot = Path(report["results"][0]["compile_command"][-3])
        self.assertTrue(snapshot.is_relative_to(self.cache))
        self.assertEqual(lc.digest(snapshot), report["results"][0]["submission_sha256"])
        self.assertEqual(sorted(p.name for p in self.output.iterdir()), ["result.json"])

    def test_failed_case_is_failure_and_remaining_absence_is_explicit(self):
        report = self.run_with([{"name": "a", "status": "FAIL", "elapsed_ns": 123,
                                 "returncode": 0, "checker_returncode": 1}])
        self.assertEqual(report["status"], "FAIL")
        self.assertEqual(report["counts"]["case_expected"], 2)
        self.assertEqual(report["counts"]["case_completed"], 1)
        self.assertEqual(report["counts"]["case_failed"], 1)

    def test_unavailable_checkout_cannot_become_pass_or_zero_success(self):
        with patch.object(lc.subprocess, "run", return_value=subprocess.CompletedProcess([], 0, "own compiler\n")), \
             patch.object(lc, "checkout_official", side_effect=lc.RunnerError("offline")), \
             patch.object(lc, "provenance", side_effect=self.provenance):
            report = lc.run_selected(self.entries, self.plan, self.cache, self.output, PIN, root=self.root)
        self.assertEqual(report["status"], "FAIL")
        self.assertEqual(report["counts"]["unavailable"], 1)
        self.assertEqual(report["results"][0]["error_kind"], "offline")

    def test_zero_selection_skips_without_network_or_compilation(self):
        with patch.object(lc.subprocess, "run") as process, patch.object(lc, "checkout_official") as checkout:
            report = lc.run_selected(self.entries, {"selected_ids": []}, self.cache, self.output, PIN)
        self.assertEqual(report["status"], "SKIPPED")
        self.assertEqual(report["counts"]["passed"], 0)
        process.assert_not_called()
        checkout.assert_not_called()

    def test_duplicate_plan_cannot_inflate_pass_counts(self):
        with patch.object(lc, "checkout_official") as checkout:
            with self.assertRaisesRegex(ValueError, "重複"):
                lc.run_selected(self.entries, {"selected_ids": ["a", "a"]}, self.cache, self.output, PIN)
        checkout.assert_not_called()

    def test_discard_mode_groups_shared_problem_and_cleans_after_last_id(self):
        entries = [{"id": name, "problem_url": "https://judge.yosupo.jp/problem/" + problem}
                   for name, problem in [("a", "unionfind"), ("b", "aplusb"), ("c", "unionfind")]]
        for name in ["b", "c"]:
            (self.submission.parent / (name + ".cpp")).write_bytes(self.submission.read_bytes())
        executed = []
        clean_at = []
        def source(entry, root):
            return self.provenance(entry, root) | {"submission": "Verify/current/submissions/" + entry["id"] + ".cpp"}
        def case(binary, directory, name, work, timeout):
            executed.append(name)
            return {"name": name, "status": "PASS", "elapsed_ns": 1,
                    "returncode": 0, "checker_returncode": 0}
        def clean(checkout, directory):
            clean_at.append(len(executed))
            return ["in", "out"]
        details = PIN | {"expected_cases": ["one", "two"], "checker_sha256": "e" * 64}
        with patch.object(lc.subprocess, "run", return_value=subprocess.CompletedProcess([], 0, "own compiler\n")), \
             patch.object(lc, "checkout_official", return_value=self.cache), \
             patch.object(lc, "official_problem", return_value=(self.cache, details)) as generate, \
             patch.object(lc, "find_problem", return_value=self.cache), \
             patch.object(lc, "discard_test_data", side_effect=clean), \
             patch.object(lc, "provenance", side_effect=source), \
             patch.object(lc, "execute", side_effect=self.compile), \
             patch.object(lc, "run_case", side_effect=case):
            report = lc.run_selected(entries, {"selected_ids": ["a", "b", "c"]}, self.cache,
                                     self.output, PIN, root=self.root, discard_data=True)
        self.assertEqual(report["status"], "PASS")
        self.assertEqual(report["selection"]["selected_ids"], ["a", "b", "c"])
        self.assertEqual(report["execution_order"], ["b", "a", "c"])
        self.assertEqual([row["id"] for row in report["results"]], ["b", "a", "c"])
        self.assertEqual(generate.call_count, 2)
        self.assertEqual(clean_at, [2, 6])
        self.assertTrue(all(row["test_data_cleanup"]["status"] == "PASS" for row in report["results"]))

    def test_unsynchronized_source_is_not_compiled(self):
        with patch.object(lc.prepare_verify, "render", return_value=(b"generated source", {})):
            with self.assertRaisesRegex(lc.RunnerError, "submission_not_synchronized"):
                lc.provenance(self.entries[0], self.root)


class CleanupSafetyTests(unittest.TestCase):
    def setUp(self):
        self.directory = tempfile.TemporaryDirectory()
        self.addCleanup(self.directory.cleanup)
        self.base = Path(self.directory.name)
        self.checkout = self.base / "checkout"
        self.checkout.mkdir()
        subprocess.run(["git", "init", "-q", str(self.checkout)], check=True, capture_output=True)
        self.problem = self.checkout / "data_structure/own_problem"
        self.problem.mkdir(parents=True)
        for name in ["in", "out"]:
            (self.problem / name).mkdir()
            (self.problem / name / "example.txt").write_text("own generated test fixture")
        self.source = self.problem / "checker.cpp"
        self.source.write_text("// own tracked source fixture\n")
        self.binary = self.problem / "checker"
        self.binary.write_text("own generated binary fixture")
        subprocess.run(["git", "-C", str(self.checkout), "add", "data_structure/own_problem/checker.cpp"],
                       check=True, capture_output=True)

    def test_removes_only_untracked_in_out_and_preserves_source_and_checker(self):
        self.assertEqual(lc.discard_test_data(self.checkout, self.problem), ["in", "out"])
        self.assertFalse((self.problem / "in").exists())
        self.assertFalse((self.problem / "out").exists())
        self.assertTrue(self.source.exists())
        self.assertTrue(self.binary.exists())
        self.assertEqual(lc.discard_test_data(self.checkout, self.problem), [])

    def test_tracked_data_is_not_removed_and_neither_directory_is_partially_deleted(self):
        subprocess.run(["git", "-C", str(self.checkout), "add", "data_structure/own_problem/out/example.txt"],
                       check=True, capture_output=True)
        with self.assertRaisesRegex(lc.RunnerError, "tracked_files"):
            lc.discard_test_data(self.checkout, self.problem)
        self.assertTrue((self.problem / "in/example.txt").exists())
        self.assertTrue((self.problem / "out/example.txt").exists())

    def test_directory_symlink_is_rejected_without_touching_target(self):
        outside = self.base / "outside"
        outside.mkdir()
        sentinel = outside / "keep"
        sentinel.write_text("keep")
        shutil.rmtree(self.problem / "in")
        (self.problem / "in").symlink_to(outside, target_is_directory=True)
        with self.assertRaisesRegex(lc.RunnerError, "symlink"):
            lc.discard_test_data(self.checkout, self.problem)
        self.assertEqual(sentinel.read_text(), "keep")
        self.assertTrue((self.problem / "out").exists())

    def test_nested_symlink_and_other_problem_path_are_rejected(self):
        (self.problem / "in/link").symlink_to(self.source)
        with self.assertRaisesRegex(lc.RunnerError, "symlink"):
            lc.discard_test_data(self.checkout, self.problem)
        with self.assertRaisesRegex(lc.RunnerError, "outside_checkout"):
            lc.discard_test_data(self.checkout, self.base / "not-in-checkout")
        self.assertTrue(self.source.exists())


if __name__ == "__main__":
    unittest.main()
