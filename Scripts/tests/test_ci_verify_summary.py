# SPDX-License-Identifier: CC0-1.0
from copy import deepcopy
from html.parser import HTMLParser
from pathlib import Path
import json
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import ci_verify_summary as summary
import ci_verify


def report(ids=("alpha", "beta"), *, compiler="/usr/bin/g++", fallback=False):
    return {"format": 1, "status": "PASS", "compiler": compiler,
            "standard_fallback": fallback, "plan": {"selected_ids": list(ids)},
            "online_status": "not_evaluated",
            "results": [{"id": identifier, "skip_runtime_checks": skip, "compiler": compiler,
                         "standard_fallback": fallback, "passed": True, "compile_ok": True,
                         "samples": [{"passed": True, "stdout": "ignored"}]}
                        for identifier in ids for skip in (False, True)]}


class SummaryTests(unittest.TestCase):
    def test_complete_report_counts_both_modes_and_preserves_environment(self):
        result = summary.summarize(report(fallback=True, compiler="clang++"))
        self.assertEqual(result, summary.Summary("PASS", "clang++", True, 2, 2, 2))
        text = summary.markdown([result])
        self.assertIn("clang++", text)
        self.assertIn("有効 | 2 | 2/2 | 2/2", text)
        self.assertIn("オンライン提出の判定（AC等）は含みません", text)

    def test_zero_targets_are_explicitly_unexecuted(self):
        result = summary.summarize(report(()))
        self.assertEqual(result.status, "対象なし")
        self.assertNotIn("PASS", summary.markdown([result]))
        self.assertIn("| 0 | — | — |", summary.markdown([result]))

    def test_missing_mode_or_running_report_cannot_pass(self):
        for mutation in (lambda data: data["results"].pop(),
                         lambda data: data.update(status="running")):
            data = report()
            mutation(data)
            self.assertEqual(summary.summarize(data).status, "未完了")
        data = report()
        data["results"].pop()
        self.assertEqual(summary.summarize(data).unsuccessful_ids, ("beta",))

    def test_compile_failure_sample_failure_or_failure_status_cannot_pass(self):
        for mode in ("compile", "sample", "reported", "empty_samples"):
            data = report()
            if mode == "compile":
                data["results"][0].update(compile_ok=False, samples=[], passed=False)
            elif mode == "sample":
                data["results"][0]["samples"][0]["passed"] = False
            elif mode == "empty_samples":
                data["results"][0]["samples"] = []
            else:
                data["status"] = "FAIL"
            with self.subTest(mode=mode):
                self.assertEqual(summary.summarize(data).status, "FAIL")
        data = report()
        data.update(status="FAIL", results=[])
        self.assertEqual(summary.summarize(data).status, "FAIL")

    def test_inconsistent_or_duplicate_results_are_rejected(self):
        for mode in ("duplicate", "unknown_id", "row_compiler", "row_fallback", "skip_type"):
            data = report()
            if mode == "duplicate":
                data["results"].append(deepcopy(data["results"][0]))
            elif mode == "unknown_id":
                data["results"][0]["id"] = "unknown"
            elif mode == "row_compiler":
                data["results"][0]["compiler"] = "another"
            elif mode == "row_fallback":
                data["results"][0]["standard_fallback"] = True
            else:
                data["results"][0]["skip_runtime_checks"] = 0
            with self.subTest(mode=mode):
                self.assertEqual(summary.summarize(data).status, "不正な結果")

    def test_malformed_fields_and_truthy_nonbooleans_never_pass(self):
        bad = [None, [], {"format": True}, report(ids=("a", "a"))]
        for field, value in (("compiler", None), ("standard_fallback", "false"),
                             ("plan", {}), ("plan", {"selected_ids": [["nested"]]}),
                             ("results", None), ("status", "AC")):
            data = report()
            data[field] = value
            bad.append(data)
        for field in ("passed", "compile_ok"):
            data = report()
            data["results"][0][field] = "true"
            bad.append(data)
        data = report()
        del data["results"][0]["samples"]
        bad.append(data)
        for data in bad:
            with self.subTest(data=data):
                self.assertEqual(summary.summarize(data).status, "不正な結果")

    def test_untrusted_ids_compiler_and_diagnostics_cannot_inject_markup(self):
        attack = 'x` | [link](https://example.invalid) <img src=x onerror="bad">\n## fake\u202e'
        data = report((attack,), compiler=attack)
        data["results"][0]["passed"] = False
        data["error"] = "UNTRUSTED_DIAGNOSTIC"
        text = summary.markdown([summary.summarize(data)])
        self.assertNotIn("<img", text)
        self.assertNotIn("[link]", text)
        self.assertNotIn("\n## fake", text)
        self.assertNotIn("\u202e", text)
        self.assertNotIn("UNTRUSTED_DIAGNOSTIC", text)
        self.assertEqual(text.count("<code>"), 2)
        self.assertIn("&#124;", text)

        class Tags(HTMLParser):
            def __init__(self):
                super().__init__()
                self.tags = []

            def handle_starttag(self, tag, attrs):
                self.tags.append(tag)

        parser = Tags()
        parser.feed(summary.inline_code(attack))
        self.assertEqual(parser.tags, ["code"])

    def test_unsuccessful_list_is_bounded(self):
        data = report(tuple(f"id{i:02}" for i in range(12)))
        data.update(status="FAIL", results=[])
        text = summary.markdown([summary.summarize(data)])
        self.assertIn("ほか4件", text)
        self.assertNotIn("id08", text)


class CommandLineTests(unittest.TestCase):
    def setUp(self):
        self.directory = tempfile.TemporaryDirectory()
        self.addCleanup(self.directory.cleanup)
        self.root = Path(self.directory.name)

    def run_cli(self, *names):
        return subprocess.run([sys.executable, str(Path(summary.__file__)),
                               *(str(self.root / name) for name in names)],
                              capture_output=True, text=True, timeout=10)

    def write(self, name, data):
        (self.root / name).write_text(json.dumps(data), encoding="utf-8")

    def test_multiple_reports_render_separate_matrix_rows(self):
        self.write("gcc.json", report())
        self.write("clang.json", report(compiler="clang++", fallback=True))
        result = self.run_cli("gcc.json", "clang.json")
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(result.stdout.count("| PASS |"), 2)
        self.assertEqual(result.stderr, "")

    def test_zero_targets_exit_zero_without_claiming_pass(self):
        self.write("empty.json", report(()))
        result = self.run_cli("empty.json")
        self.assertEqual(result.returncode, 0)
        self.assertIn("対象なし", result.stdout)
        self.assertNotIn("PASS", result.stdout)

    def test_missing_malformed_failed_and_partial_reports_exit_nonzero(self):
        (self.root / "malformed.json").write_text("{bad json", encoding="utf-8")
        (self.root / "encoding.json").write_bytes(b"\xff")
        failed = report()
        failed["status"] = "FAIL"
        self.write("failed.json", failed)
        partial = report()
        partial["results"].pop()
        self.write("partial.json", partial)
        for name in ("missing.json", "malformed.json", "encoding.json", "failed.json", "partial.json"):
            with self.subTest(name=name):
                result = self.run_cli(name)
                self.assertEqual(result.returncode, 1, result.stderr)
                self.assertNotIn("| PASS |", result.stdout)

    def test_a_missing_report_does_not_hide_successful_report(self):
        self.write("ok.json", report())
        result = self.run_cli("ok.json", "missing.json")
        self.assertEqual(result.returncode, 1)
        self.assertIn("| PASS |", result.stdout)
        self.assertIn("| 結果なし |", result.stdout)

    def test_current_ci_writer_reports_are_consumed_without_conversion(self):
        output = self.root / "actual.json"

        def run_tests(entries, compiler, fallback, destination):
            destination.write_text(json.dumps({"results": report()["results"]}), encoding="utf-8")

        with patch.object(ci_verify.prepare_verify, "run_tests", side_effect=run_tests):
            ci_verify.run_selected([{"id": "alpha"}, {"id": "beta"}],
                                   {"selected_ids": ["alpha", "beta"]}, "/usr/bin/g++", False, output)
        result = self.run_cli("actual.json")
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertIn("| PASS |", result.stdout)
        self.assertIn("2/2 | 2/2", result.stdout)
        ci_verify.run_selected([], {"selected_ids": []}, "/usr/bin/g++", False, output)
        result = self.run_cli("actual.json")
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertIn("対象なし", result.stdout)
        self.assertNotIn("PASS", result.stdout)


if __name__ == "__main__":
    unittest.main()
