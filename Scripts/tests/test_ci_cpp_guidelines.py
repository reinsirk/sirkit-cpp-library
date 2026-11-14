# SPDX-License-Identifier: CC0-1.0
from pathlib import Path
import contextlib
import io
import json
import os
import shutil
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import ci_cpp_guidelines as ci


class DiagnosticTests(unittest.TestCase):
    def setUp(self):
        self.directory = tempfile.TemporaryDirectory()
        self.addCleanup(self.directory.cleanup)
        self.root = Path(self.directory.name)

    def test_only_original_libraries_are_counted(self):
        self.assertEqual(ci.source_relative(self.root / "Graph/A.cpp", self.root), "Graph/A.cpp")
        for name in ["Fast/GCC/Graph/A.cpp", "Variants/Clang/Graph/A.cpp",
                     "Verify/current/vendor/Graph/A.cpp", "Verify/current/submissions/a.cpp",
                     "Examples/a.cpp", "Verify/current/drivers/a.cpp"]:
            self.assertIsNone(ci.source_relative(self.root / name, self.root), name)
        self.assertIsNone(ci.source_relative(self.root.parent / "Graph/A.cpp", self.root))

    def test_strict_and_report_only_checks_remain_distinct(self):
        source = self.root / "DataStructure/Some.cpp"
        raw = (f"{source}:25:12: warning: owner [cppcoreguidelines-owning-memory]\n"
               f"{source}:26:9: warning: init [cppcoreguidelines-pro-type-member-init]\n"
               f"{self.root}/Fast/GCC/Graph/A.cpp:7:2: warning: owner [cppcoreguidelines-owning-memory]\n")
        rows, errors = ci.parse_diagnostics(raw, self.root)
        self.assertEqual(len(rows), 2)
        self.assertEqual([r["strict"] for r in rows], [True, False])
        self.assertEqual(errors, [])

    def test_compiler_error_in_driver_still_fails(self):
        rows, errors = ci.parse_diagnostics(
            f"{self.root}/driver.cpp:4:7: error: unexpected token [clang-diagnostic-error]\n", self.root)
        self.assertEqual(rows, [])
        self.assertEqual(len(errors), 1)
        self.assertEqual(errors[0]["severity"], "error")

    def test_multiple_check_names_and_deduplication_preserve_instantiation_ids(self):
        raw = f"{self.root}/Graph/A.cpp:5:2: warning: bad [some-check,cppcoreguidelines-slicing]\n"
        rows, _ = ci.parse_diagnostics(raw, self.root)
        self.assertTrue(rows[0]["strict"])
        result = ci.unique_diagnostics([dict(rows[0], verify_id="a"), dict(rows[0], verify_id="b"),
                                        dict(rows[0], verify_id="a")])
        self.assertEqual(len(result), 1)
        self.assertEqual(result[0]["verify_ids"], ["a", "b"])

    def test_summary_displays_report_findings_without_publishing_message_markup(self):
        raw = f"{self.root}/Graph/A.cpp:5:2: warning: <img src=x> [cppcoreguidelines-pro-type-member-init]\n"
        rows, _ = ci.parse_diagnostics(raw, self.root)
        text = ci.markdown({"status": "PASS", "runs": [{}], "diagnostics": rows})
        self.assertIn("重大な指摘 0件、確認済み例外 0件、参考の指摘 1件", text)
        self.assertIn("Fast/", text)
        self.assertIn("未実体化API", text)
        self.assertNotIn("<img", text)


class TranslationUnitTests(unittest.TestCase):
    def setUp(self):
        self.directory = tempfile.TemporaryDirectory()
        self.addCleanup(self.directory.cleanup)
        self.root = Path(self.directory.name)
        self.entry = {"id": "sample", "libraries": ["Graph/A.cpp"],
                      "driver": "Verify/current/drivers/a.cpp"}
        driver = self.root / self.entry["driver"]
        driver.parent.mkdir(parents=True)
        driver.write_text("int main() { return f(); }\n")
        library = self.root / "Graph/A.cpp"
        library.parent.mkdir(parents=True)
        library.write_text("inline int f() { return 0; }\n")
        (self.root / "Scripts").mkdir()
        shutil.copyfile(ci.ROOT / "Scripts/ci_cpp_guidelines_exceptions.json",
                        self.root / "Scripts/ci_cpp_guidelines_exceptions.json")

    def test_original_header_is_included_with_checked_driver(self):
        with patch.object(ci.prepare_verify, "ROOT", self.root):
            source = ci.checked_translation_unit(self.entry, self.root).decode()
        self.assertIn("#include <" + str(self.root / "Graph/A.cpp") + ">", source)
        self.assertIn("return f();", source)
        self.assertNotIn("#define SKIP_RUNTIME_CHECKS", source)
        self.assertNotIn("#define NDEBUG", source)

    def test_fast_source_is_never_accepted_as_original(self):
        self.entry["libraries"] = ["Fast/GCC/Graph/A.cpp"]
        with self.assertRaisesRegex(ValueError, "通常版以外"):
            ci.checked_translation_unit(self.entry, self.root)

    def test_driver_cannot_silently_disable_checked_analysis(self):
        for macro in ["SKIP_RUNTIME_CHECKS", "NDEBUG"]:
            (self.root / self.entry["driver"]).write_text(f"#define {macro}\nint main() {{}}\n")
            with patch.object(ci.prepare_verify, "ROOT", self.root):
                with self.assertRaisesRegex(ValueError, "checked検査"):
                    ci.checked_translation_unit(self.entry, self.root)

    def test_missing_tool_or_empty_selection_cannot_pass(self):
        for entries in [[], [self.entry]]:
            out = self.root / ("empty" if not entries else "tool_missing")
            with contextlib.redirect_stdout(io.StringIO()):
                result = ci.run(entries, "/not-installed/clang-tidy", out, root=self.root)
            self.assertEqual(result["status"], "FAIL")
            self.assertEqual(json.loads((out / "result.json").read_text())["status"], "FAIL")
            self.assertIn("検査を完了できません", (out / "summary.md").read_text())


class ReviewedExceptionTests(unittest.TestCase):
    def setUp(self):
        self.directory = tempfile.TemporaryDirectory()
        self.addCleanup(self.directory.cleanup)
        self.root = Path(self.directory.name)
        self.path = self.root / "Graph/A.cpp"
        self.path.parent.mkdir()
        self.path.write_text("// reviewed example\n")
        import hashlib
        self.exception = {"path": "Graph/A.cpp", "source_sha256": hashlib.sha256(self.path.read_bytes()).hexdigest(),
                          "line": 5, "column": 2, "check": "bugprone-use-after-move",
                          "message": "'x' used after it was moved", "reason": "Reviewed valid swap only."}

    def diagnostic(self, **updates):
        row = {"path": "Graph/A.cpp", "line": 5, "column": 2, "severity": "warning",
               "checks": ["bugprone-use-after-move"], "message": "'x' used after it was moved", "strict": True}
        row.update(updates)
        return row

    def test_exact_review_is_visible_and_not_counted_as_a_new_strict_failure(self):
        row = self.diagnostic()
        ci.apply_reviewed_exceptions([row], [self.exception], self.root)
        self.assertFalse(row["strict"])
        self.assertEqual(row["reviewed_exception"], self.exception)
        text = ci.markdown({"status": "PASS", "runs": [{}], "diagnostics": [row]})
        self.assertIn("確認済み例外 1件", text)
        self.assertIn("| 確認済み例外 |", text)

    def test_source_change_invalidates_review_even_at_the_same_position(self):
        self.path.write_text("// changed source\n")
        row = self.diagnostic()
        ci.apply_reviewed_exceptions([row], [self.exception], self.root)
        self.assertTrue(row["strict"])
        self.assertIsNone(row["reviewed_exception"])

    def test_other_location_message_or_additional_rule_is_not_exempt(self):
        for update in [{"line": 6}, {"column": 3}, {"message": "new diagnostic"},
                       {"checks": ["bugprone-use-after-move", "cppcoreguidelines-owning-memory"]}]:
            row = self.diagnostic(**update)
            ci.apply_reviewed_exceptions([row], [self.exception], self.root)
            self.assertTrue(row["strict"], update)

    def test_missing_reason_noncanonical_path_and_bad_sha_are_rejected(self):
        for update in [{"reason": ""}, {"source_sha256": "not-a-sha"},
                       {"path": "Fast/GCC/Graph/A.cpp"}, {"path": "Graph/../A.cpp"}, {"line": True},
                       {"check": ["bugprone-use-after-move"]}]:
            path = self.root / "exceptions.json"
            path.write_text(json.dumps({"format": 1, "entries": [dict(self.exception, **update)]}))
            with self.assertRaises(ValueError):
                ci.load_reviewed_exceptions(path)


class ConfigurationTests(unittest.TestCase):
    def setUp(self):
        self.directory = tempfile.TemporaryDirectory()
        self.addCleanup(self.directory.cleanup)
        self.root = Path(self.directory.name)
        self.config = self.root / ".clang-tidy"
        self.config.write_text("Checks: configured\n")
        self.output = self.root / "output"
        self.output.mkdir()

    def test_enabled_rules_are_measured_and_checked_config_bytes_are_frozen(self):
        enabled = "Enabled checks:\n" + "".join("    " + c + "\n" for c in sorted(ci.REQUIRED_CHECKS))
        calls = []
        def tool(command, **kwargs):
            calls.append(command)
            text = enabled if "--list-checks" in command else self.config.read_text() if "--dump-config" in command else "OK\n"
            return subprocess.CompletedProcess(command, 0, text, "")
        result = {}
        with patch.object(ci.subprocess, "run", side_effect=tool):
            fixed = ci.validate_configuration("tool", self.config, self.output, result)
        self.assertEqual(result["status"], "PASS")
        self.assertEqual(set(result["enabled_checks"]), ci.REQUIRED_CHECKS)
        self.assertEqual(calls[0][1], "--verify-config")
        self.assertEqual(calls[1][1], "--list-checks")
        self.assertTrue(all("--config-file=" + str(fixed) in c for c in calls[-2:]))
        self.assertTrue(all("--config-file=" + str(self.output / "clang-tidy-source.yaml") in c for c in calls[:3]))
        original = self.config.read_bytes()
        self.config.write_text("Checks: '-*'\n")
        self.assertEqual(fixed.read_bytes(), original)
        import hashlib
        self.assertEqual(result["sha256"], hashlib.sha256(original).hexdigest())

    def test_profiles_have_distinct_effective_rule_sets_from_the_same_source_config(self):
        for guide, expected in [("core", ci.CORE_CHECKS), ("google", ci.GOOGLE_CHECKS), ("both", ci.REQUIRED_CHECKS)]:
            calls = []
            def tool(command, **kwargs):
                calls.append(command)
                source_config = any(x.endswith("clang-tidy-source.yaml") for x in command)
                enabled = ci.REQUIRED_CHECKS if source_config else expected
                text = "Enabled checks:\n" + "".join("    " + c + "\n" for c in sorted(enabled))
                if "--dump-config" in command:
                    text = "Checks: mock-resolved-config\n"
                return subprocess.CompletedProcess(command, 0, text, "")
            result = {}
            with patch.object(ci.subprocess, "run", side_effect=tool):
                ci.validate_configuration("tool", self.config, self.output, result, guide=guide)
            self.assertEqual(result["guide"], guide)
            self.assertEqual(set(result["enabled_checks"]), expected)
            self.assertEqual(set(result["required_checks"]), expected)
            self.assertIn("--checks=-*," + ",".join(sorted(expected)), calls[2])

    def test_empty_or_missing_mandatory_checks_cannot_pass(self):
        for enabled in [set(), {"cppcoreguidelines-owning-memory"}]:
            text = "Enabled checks:\n" + "".join("    " + c + "\n" for c in enabled)
            def tool(command, **kwargs):
                return subprocess.CompletedProcess(command, 0, text if "--list-checks" in command else "", "")
            result = {}
            with patch.object(ci.subprocess, "run", side_effect=tool), self.assertRaisesRegex(ValueError, "必須チェック"):
                ci.validate_configuration("tool", self.config, self.output, result)
            self.assertEqual(result["status"], "FAIL")
            self.assertEqual(set(result["missing_strict_checks"]), ci.STRICT_CHECKS - enabled)

    def test_google_report_rule_is_required_even_if_all_strict_checks_are_present(self):
        missing = "google-readability-casting"
        enabled = ci.REQUIRED_CHECKS - {missing}
        text = "Enabled checks:\n" + "".join("    " + c + "\n" for c in sorted(enabled))
        def tool(command, **kwargs):
            return subprocess.CompletedProcess(command, 0, text if "--list-checks" in command else "", "")
        result = {}
        with patch.object(ci.subprocess, "run", side_effect=tool), self.assertRaisesRegex(ValueError, missing):
            ci.validate_configuration("tool", self.config, self.output, result)
        self.assertEqual(result["status"], "FAIL")
        self.assertEqual(result["missing_strict_checks"], [])
        self.assertEqual(result["missing_required_checks"], [missing])

    def test_bad_configuration_is_a_failure_before_listing_or_any_translation_unit(self):
        result = {}
        with patch.object(ci.subprocess, "run", return_value=subprocess.CompletedProcess([], 1, "", "unknown check")) as tool:
            with self.assertRaisesRegex(ValueError, "設定を検証できません"):
                ci.validate_configuration("tool", self.config, self.output, result)
        self.assertEqual(tool.call_count, 1)
        self.assertEqual(result["status"], "FAIL")
        self.assertEqual(result["enabled_checks"], [])


class ActualClangTidyTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.tool = os.environ.get("SIRKIT_CLANG_TIDY") or shutil.which("clang-tidy-18")
        if not cls.tool:
            raise unittest.SkipTest("clang-tidy-18またはSIRKIT_CLANG_TIDYが必要です")

    def check(self, library: str, *, driver: str = "int main() { return use(); }\n",
              extra: dict[str, str] | None = None, config: str | None = None, expect_runs: int = 1,
              guide: str = "both") -> dict:
        with tempfile.TemporaryDirectory(prefix="sirkit-tidy-test-") as temporary:
            root = Path(temporary)
            (root / "Graph").mkdir()
            (root / "Graph/A.cpp").write_text(library)
            (root / "Verify/current/drivers").mkdir(parents=True)
            (root / "Verify/current/drivers/a.cpp").write_text(driver)
            shutil.copyfile(ci.ROOT / ".clang-tidy", root / ".clang-tidy")
            if config is not None:
                (root / ".clang-tidy").write_text(config)
            (root / "Scripts").mkdir()
            shutil.copyfile(ci.ROOT / "Scripts/ci_cpp_guidelines_exceptions.json",
                            root / "Scripts/ci_cpp_guidelines_exceptions.json")
            for name, text in (extra or {}).items():
                path = root / name
                path.parent.mkdir(parents=True, exist_ok=True)
                path.write_text(text)
            entry = {"id": "real", "libraries": ["Graph/A.cpp"],
                     "driver": "Verify/current/drivers/a.cpp"}
            with patch.object(ci.prepare_verify, "ROOT", root), contextlib.redirect_stdout(io.StringIO()):
                result = ci.run([entry], self.tool, root / "result",
                                os.environ.get("SIRKIT_TIDY_GCC_INSTALL_DIR"), root=root, guide=guide)
            self.assertEqual(len(result["runs"]), expect_runs, result)
            return result

    def test_good_owner_passes_and_raw_new_is_a_gate_failure(self):
        good = self.check("#include <memory>\ninline int use() { auto p = std::make_unique<int>(0); return *p; }\n")
        self.assertEqual(good["status"], "PASS", good)
        bad = self.check("inline int use() { int* p = new int(0); return *p; }\n")
        self.assertEqual(bad["status"], "FAIL", bad)
        self.assertTrue(any("cppcoreguidelines-owning-memory" in x["checks"] and x["strict"]
                            for x in bad["diagnostics"]))

    def test_report_only_initialization_is_visible_without_blanket_suppression(self):
        result = self.check("struct A { int x; A() {} }; inline int use() { A a; return 0; }\n")
        self.assertEqual(result["status"], "PASS", result)
        self.assertTrue(any("cppcoreguidelines-pro-type-member-init" in x["checks"]
                            for x in result["diagnostics"]), result)
        self.assertFalse(any(x["strict"] for x in result["diagnostics"]))

    def test_fast_warning_and_driver_warning_are_not_original_findings(self):
        result = self.check("inline int use() { return 0; }\n",
                            driver='#include <Fast/GCC/Graph/B.cpp>\nint main() { int* p = new int(0); return use() + other() + *p; }\n',
                            extra={"Fast/GCC/Graph/B.cpp": "inline int other() { int* p = new int(0); return *p; }\n"})
        self.assertEqual(result["status"], "PASS", result)
        self.assertEqual(result["diagnostics"], [])

    def test_compiler_failure_cannot_be_confused_with_no_findings(self):
        result = self.check("inline int use() { return missing_name; }\n")
        self.assertEqual(result["status"], "FAIL", result)
        self.assertTrue(result["runs"][0]["compiler_errors"])

    def test_actual_unknown_empty_and_missing_strict_configs_fail_preflight(self):
        for config in ["Checks: '-*'\n", "Checks: '-*,cppcoreguidelines-owning-memory'\n",
                       "Checks: '-*,not-a-real-check'\n", "Checks: [invalid\n"]:
            result = self.check("inline int use() { return 0; }\n", config=config, expect_runs=0)
            self.assertEqual(result["status"], "FAIL", result)
            self.assertEqual(result["configuration"]["status"], "FAIL")

    def test_actual_google_namespace_integer_and_address_operator_are_gates(self):
        cases = [
            ("#include <vector>\nusing namespace std;\ninline int use() { return 0; }\n", "google-build-using-namespace"),
            ("inline long long wide() { return 0; } inline int use() { return static_cast<int>(wide()); }\n", "google-runtime-int"),
            ("struct A { int operator&() const { return 0; } }; inline int use() { A a; return &a; }\n", "google-runtime-operator"),
        ]
        for source, check in cases:
            result = self.check(source)
            self.assertEqual(result["status"], "FAIL", result)
            self.assertTrue(any(check in x["checks"] and x["strict"] for x in result["diagnostics"]), result)

    def test_actual_google_cast_and_make_pair_are_visible_reference_findings(self):
        cases = [
            ("inline int use() { double x = 0; return (int)x; }\n", "google-readability-casting"),
            ("#include <utility>\ninline int use() { auto p=std::make_pair<int,int>(1,2); return p.first-1; }\n", "google-build-explicit-make-pair"),
        ]
        for source, check in cases:
            result = self.check(source)
            self.assertEqual(result["status"], "PASS", result)
            self.assertTrue(any(check in x["checks"] and not x["strict"] for x in result["diagnostics"]), result)

    def test_actual_core_and_google_profiles_are_independent_and_google_has_no_core_exceptions(self):
        source = "#include <vector>\nusing namespace std;\ninline int use() { return 0; }\n"
        core = self.check(source, guide="core")
        google = self.check(source, guide="google")
        self.assertEqual(core["status"], "PASS", core)
        self.assertEqual(google["status"], "FAIL", google)
        self.assertEqual(set(core["configuration"]["enabled_checks"]), ci.CORE_CHECKS)
        self.assertEqual(set(google["configuration"]["enabled_checks"]), ci.GOOGLE_CHECKS)
        self.assertEqual(len(core["reviewed_exception_policy"]["entries"]), 2)
        self.assertEqual(google["reviewed_exception_policy"]["entries"], [])
        self.assertIsNone(google["reviewed_exception_policy"]["sha256"])


if __name__ == "__main__":
    unittest.main()
