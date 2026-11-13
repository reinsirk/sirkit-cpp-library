# SPDX-License-Identifier: CC0-1.0
from pathlib import Path
import json
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import ci_verify as ci


class SelectionTests(unittest.TestCase):
    def setUp(self):
        self.directory = tempfile.TemporaryDirectory()
        self.addCleanup(self.directory.cleanup)
        self.root = Path(self.directory.name)
        self.entries = [
            {"id": "alpha", "group": "first", "libraries": ["DataStructure/A.cpp"],
             "driver": "Verify/current/drivers/a.cpp"},
            {"id": "beta", "group": "second", "libraries": ["Math/B.cpp"],
             "driver": "Verify/current/drivers/b.cpp"},
            {"id": "gamma", "group": "third", "libraries": ["Geometry/C.cpp"],
             "driver": "Verify/current/drivers/c.cpp"},
        ]
        self.write("Verify/current/drivers/a.cpp", '#include "Verify/current/common/io.hpp"\n')
        self.write("Verify/current/common/io.hpp", '#include "Verify/current/common/buffer.hpp"\n')
        self.write("Verify/current/common/buffer.hpp", "int buffer;\n")
        self.write("Verify/current/drivers/b.cpp", "int main() {}\n")
        self.write("Verify/current/drivers/c.cpp", "int main() {}\n")

    def write(self, name, text):
        path = self.root / name
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(text)

    def choose(self, *paths, **options):
        return ci.select_entries(self.entries, list(paths), root=self.root, **options)

    def test_library_driver_and_generated_program_select_their_entry(self):
        for path in ("DataStructure/A.cpp", "Verify/current/drivers/a.cpp",
                     "Verify/current/submissions/alpha.cpp"):
            with self.subTest(path=path):
                self.assertEqual(self.choose(path)["selected_ids"], ["alpha"])

    def test_recursive_shared_headers_select_consumers(self):
        result = self.choose("Verify/current/common/buffer.hpp")
        self.assertEqual(result["selected_ids"], ["alpha"])
        self.assertIn("Verify/current/common/buffer.hpp", result["reasons"]["alpha"])

    def test_component_inputs_conservatively_select_their_family(self):
        self.assertEqual(self.choose("Math/FPS/components/formal_power_series_body.inc")["selected_ids"], ["beta"])
        self.assertEqual(self.choose("Geometry/components/point.inc")["selected_ids"], ["gamma"])

    def test_embedded_dependency_selects_consumer(self):
        self.entries[0]["libraries"] = ["DataStructure/Dynamic_Bitset.cpp"]
        self.entries[1]["libraries"] = ["Math/Binary_Matrix.cpp"]
        self.assertEqual(self.choose("DataStructure/Dynamic_Bitset.cpp")["selected_ids"], ["alpha", "beta"])
        self.entries[0]["libraries"] = ["Math/NTT.cpp"]
        self.entries[1]["libraries"] = ["Math/Polynomial.cpp"]
        self.assertEqual(self.choose("Math/NTT.cpp")["selected_ids"], ["alpha", "beta"])

    def test_global_renderer_vendor_and_generator_select_all(self):
        for path in ("Scripts/prepare_verify.py", "Scripts/verify_graph_samples.py",
                     "Scripts/generate_binary_matrix.py", "Verify/current/vendor/atcoder/modint.hpp"):
            self.assertEqual(self.choose(path)["selected_ids"], ["alpha", "beta", "gamma"])

    def test_group_addition_and_removal(self):
        self.assertEqual(self.choose("Verify/current/groups/first.json")["selected_ids"], ["alpha"])
        self.assertEqual(self.choose("Verify/current/groups/removed.json")["selected_ids"], ["alpha", "beta", "gamma"])

    def test_documentation_and_evidence_do_not_compile_programs(self):
        result = self.choose("Readme.md", "Document/a.md", "Audit/result.json", "AGENTS.md",
                             "Verify/current/judgements.json", "Distribution/manifest.json")
        self.assertEqual(result["selected_ids"], [])
        self.assertEqual(result["online_status"], "not_evaluated")

    def test_explicit_all_and_id_filters(self):
        self.assertEqual(self.choose(requested=["gamma", "alpha", "alpha"])["selected_ids"], ["alpha", "gamma"])
        self.assertEqual(self.choose(all_entries=True)["selected_ids"], ["alpha", "beta", "gamma"])
        with self.assertRaisesRegex(ValueError, "存在しない"):
            self.choose(requested=["not-an-id"])

    def test_unassigned_new_library_is_an_error(self):
        self.write("Graph/New.cpp", "int new_library;\n")
        with self.assertRaisesRegex(ValueError, "未割当"):
            self.choose("Graph/New.cpp")
        self.assertEqual(self.choose("Graph/Removed.cpp")["selected_ids"], ["alpha", "beta", "gamma"])

    def test_missing_shared_header_is_not_silently_ignored(self):
        (self.root / "Verify/current/common/buffer.hpp").unlink()
        with self.assertRaises(FileNotFoundError):
            self.choose("Document/a.md")

    def test_duplicate_ids_are_rejected(self):
        self.entries.append(dict(self.entries[0]))
        with self.assertRaisesRegex(ValueError, "重複"):
            self.choose()


class GitTests(unittest.TestCase):
    def test_ref_resolved_before_diff_without_shell(self):
        calls = []
        def fake(command, **kwargs):
            calls.append((command, kwargs))
            if "rev-parse" in command:
                return subprocess.CompletedProcess(command, 0, "a" * 40 + "\n")
            if "diff" in command:
                return subprocess.CompletedProcess(command, 0, b"Math/A.cpp\0Document/note.md\0")
            return subprocess.CompletedProcess(command, 0, b"Math/A.cpp\0Scripts/new.py\0")
        with patch.object(ci.subprocess, "run", side_effect=fake):
            result = ci.git_changed_paths("--not-a-command", Path("/tmp"))
        self.assertEqual(result, ["Document/note.md", "Math/A.cpp", "Scripts/new.py"])
        self.assertIn("--end-of-options", calls[0][0])
        self.assertIn("--not-a-command^{commit}", calls[0][0])
        self.assertIn("a" * 40, calls[1][0])
        self.assertFalse(any(kwargs.get("shell", False) for _, kwargs in calls))

    def test_git_failure_propagates(self):
        error = subprocess.CalledProcessError(128, ["git", "rev-parse"])
        with patch.object(ci.subprocess, "run", side_effect=error), self.assertRaises(subprocess.CalledProcessError):
            ci.git_changed_paths("bad-ref")


class RunTests(unittest.TestCase):
    def setUp(self):
        self.directory = tempfile.TemporaryDirectory()
        self.addCleanup(self.directory.cleanup)
        self.output = Path(self.directory.name) / "results.json"
        self.entries = [{"id": "a"}, {"id": "b"}]
        self.plan = {"selected_ids": ["a"]}

    def fake_results(self, rows):
        def fake(entries, compiler, fallback, output):
            self.assertEqual([e["id"] for e in entries], ["a"])
            output.write_text(json.dumps({"results": rows}))
        return fake

    def test_two_check_configurations_are_required(self):
        rows = [{"id": "a", "skip_runtime_checks": s, "passed": True} for s in (False, True)]
        with patch.object(ci.prepare_verify, "run_tests", side_effect=self.fake_results(rows)):
            ci.run_selected(self.entries, self.plan, "g++", False, self.output)
        report = json.loads(self.output.read_text())
        self.assertEqual(report["status"], "PASS")
        self.assertEqual(report["online_status"], "not_evaluated")

    def test_missing_or_failed_results_cannot_pass(self):
        for rows in ([], [{"id": "a", "skip_runtime_checks": False, "passed": True}],
                     [{"id": "a", "skip_runtime_checks": s, "passed": False} for s in (False, True)]):
            with self.subTest(rows=rows), patch.object(ci.prepare_verify, "run_tests", side_effect=self.fake_results(rows)):
                with self.assertRaises(ValueError):
                    ci.run_selected(self.entries, self.plan, "g++", False, self.output)
                self.assertEqual(json.loads(self.output.read_text())["status"], "FAIL")

    def test_failed_command_is_recorded_and_propagated(self):
        with patch.object(ci.prepare_verify, "run_tests", side_effect=FileNotFoundError("compiler missing")):
            with self.assertRaises(FileNotFoundError):
                ci.run_selected(self.entries, self.plan, "missing-compiler", False, self.output)
        result = json.loads(self.output.read_text())
        self.assertEqual(result["status"], "FAIL")
        self.assertIn("compiler missing", result["error"])

    def test_failure_preserves_partial_results(self):
        def fail(entries, compiler, fallback, output):
            output.write_text(json.dumps({"results": [{"id": "a", "passed": False}]}))
            raise SystemExit("sample failed")
        with patch.object(ci.prepare_verify, "run_tests", side_effect=fail):
            with self.assertRaises(SystemExit):
                ci.run_selected(self.entries, self.plan, "g++", False, self.output)
        self.assertEqual(json.loads(self.output.read_text())["results"], [{"id": "a", "passed": False}])

    def test_document_only_plan_has_an_explicit_empty_report(self):
        with patch.object(ci.prepare_verify, "run_tests") as run:
            ci.run_selected(self.entries, {"selected_ids": []}, "g++", False, self.output)
        run.assert_not_called()
        self.assertEqual(json.loads(self.output.read_text())["status"], "PASS")


class CommandLineTests(unittest.TestCase):
    def test_list_does_not_mutate_generated_files_or_run_compiler(self):
        with patch.object(ci.prepare_verify, "collect", return_value=[{"id": "a"}]), \
             patch.object(ci.package_revised_libraries, "check_generated") as sync, \
             patch.object(ci.prepare_verify, "prepare") as prepare, \
             patch.object(ci, "run_selected") as run, patch("builtins.print"):
            self.assertEqual(ci.main(["--id", "a", "--list"]), 0)
        sync.assert_not_called(); prepare.assert_not_called(); run.assert_not_called()

    def test_unknown_id_and_git_error_exit_nonzero(self):
        with patch.object(ci.prepare_verify, "collect", return_value=[{"id": "a"}]), patch("sys.stderr"):
            self.assertEqual(ci.main(["--id", "unknown", "--list"]), 1)
        with patch.object(ci, "git_changed_paths", side_effect=subprocess.CalledProcessError(1, ["git"])), patch("sys.stderr"):
            self.assertEqual(ci.main(["--list"]), 1)


if __name__ == "__main__":
    unittest.main()
