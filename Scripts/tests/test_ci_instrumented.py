# SPDX-License-Identifier: CC0-1.0
from pathlib import Path
import json
import os
import shutil
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import ci_instrumented as ci
import merge_ci_coverage


def fixture(root, body="int instrumented(int x) {\n if (x > 0) return 1;\n return 0;\n}\n",
            name="Algorithm/選択.cpp", input_text="1\n", output="1\n"):
    library = root / name
    library.parent.mkdir(parents=True, exist_ok=True)
    library.write_text(body, encoding="utf-8")
    driver = root / "Verify/current/drivers/example.cpp"
    driver.parent.mkdir(parents=True, exist_ok=True)
    driver.write_text('#include <iostream>\nint main() {int x;std::cin>>x;std::cout<<instrumented(x)<<"\\n";}\n')
    return {"id": "example", "libraries": [name], "driver": str(driver.relative_to(root)),
            "samples": [{"input": input_text, "output": output}]}


class UnitTests(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory()
        self.addCleanup(self.temporary.cleanup)
        self.root = Path(self.temporary.name)
        self.entry = fixture(self.root)

    def test_default_selection_keeps_all_registered_local_fixtures(self):
        entries = [{"id": str(i), "problem_url": None if i < 54 else "online"} for i in range(229)]
        self.assertEqual(ci.select_entries(entries, None), entries)
        self.assertEqual(sum(not e["problem_url"] for e in ci.select_entries(entries, None)), 54)
        self.assertEqual([e["id"] for e in ci.select_entries(entries, ["0", "100", "0"])], ["0", "100"])
        with self.assertRaises(ValueError):
            ci.select_entries(entries, ["unknown"])

    def test_coverage_registration_uses_driver_include_order_and_tracks_support(self):
        registry = self.root / ci.COVERAGE_TESTS
        registry.parent.mkdir(parents=True)
        driver = self.root / "Verify/current/tests/supplement_test.cpp"
        driver.parent.mkdir(parents=True)
        helper = driver.with_name("helper.hpp")
        helper.write_text("// helper\n")
        driver.write_text("int main() {}\n")
        registry.write_text(json.dumps({"format": 1, "tests": {driver.name: self.entry["libraries"]},
                                        "support": {driver.name: [str(helper.relative_to(self.root))]},
                                        "gcc_line_tests": [driver.name], "x86_baseline_tests": [driver.name]}))
        test, contracts = ci.coverage_tests(self.root)
        self.assertEqual(test["id"], "coverage_test_supplement_test")
        self.assertTrue(test["gcc_line_coverage"])
        self.assertTrue(test["x86_baseline_coverage"])
        self.assertEqual(set(test["support_source_hashes"]), {str(helper.relative_to(self.root))})
        source = ci.translation_unit(test, self.root)
        self.assertIn(str(driver), source)
        self.assertNotIn(str(self.root / self.entry["libraries"][0]), source)
        self.assertTrue(ci.output_matches(test, {"input": ""}, "test counters\n"))
        with self.assertRaises(ValueError):
            ci.output_matches(contracts, {"input": ""}, '{"format":1,"results":[]}')
        value = json.loads(registry.read_text())
        value["gcc_line_tests"] = ["unknown_test.cpp"]
        registry.write_text(json.dumps(value))
        with self.assertRaisesRegex(ValueError, "GCC行計測"):
            ci.coverage_tests(self.root)
        value["gcc_line_tests"] = []
        value["x86_baseline_tests"] = [driver.name, driver.name]
        registry.write_text(json.dumps(value))
        with self.assertRaisesRegex(ValueError, "x86代替経路"):
            ci.coverage_tests(self.root)
        registry.write_text(json.dumps({"format": 1, "tests": {"../bad_test.cpp": self.entry["libraries"]}}))
        with self.assertRaisesRegex(ValueError, "テスト名"):
            ci.coverage_tests(self.root)

    def test_checked_original_unicode_include_has_no_linked_submission(self):
        source = ci.translation_unit(self.entry, self.root)
        self.assertIn("選択.cpp", source)
        self.assertNotIn("\\u", source)
        self.assertIn(str(self.root / self.entry["libraries"][0]), source)
        self.assertNotIn("#define NDEBUG", source)
        self.assertNotIn("#define SKIP_RUNTIME_CHECKS", source)
        self.assertIn("#error", source)

    def test_failure_registration_requires_a_matching_standalone_fixture(self):
        registry = self.root / ci.COVERAGE_TESTS
        registry.parent.mkdir(parents=True)
        name = "negative_test.cpp"
        library = self.entry["libraries"][0]
        value = {"format": 1, "tests": {name: [library]}, "failure_tests": {library: name}}
        registry.write_text(json.dumps(value))
        test, _ = ci.coverage_tests(self.root)
        self.assertEqual(test["failure_libraries"], [library])
        for invalid in ({library: "unknown_test.cpp"}, {"Graph/B.cpp": name}, {library: [name]}, [name]):
            registry.write_text(json.dumps({**value, "failure_tests": invalid}))
            with self.assertRaisesRegex(ValueError, "失敗系テスト"):
                ci.coverage_tests(self.root)

    def test_i386_registration_requires_a_distinct_checked_target(self):
        registry = self.root / ci.COVERAGE_TESTS
        registry.parent.mkdir(parents=True)
        name = "limits_test.cpp"
        value = {"format": 1, "tests": {name: self.entry["libraries"]}, "i386_tests": [name]}
        registry.write_text(json.dumps(value))
        test, _ = ci.coverage_tests(self.root)
        self.assertEqual(test["coverage_target"], "i386")
        for key in ("gcc_line_tests", "x86_baseline_tests"):
            registry.write_text(json.dumps({**value, key: [name]}))
            with self.assertRaisesRegex(ValueError, "i386計測"):
                ci.coverage_tests(self.root)
        registry.write_text(json.dumps({**value, "clang_unchecked_tests": [name]}))
        with self.assertRaisesRegex(ValueError, "検査省略Clang計測"):
            ci.coverage_tests(self.root)

    def test_only_original_library_paths_are_allowed(self):
        for name in ("Fast/GCC/Algorithm/x.cpp", "Variants/Clang/Algorithm/x.cpp",
                     "Verify/current/vendor/x.cpp", "Verify/current/submissions/x.cpp",
                     "Algorithm/vendor/x.cpp", "../Algorithm/x.cpp", "/tmp/Algorithm/x.cpp"):
            with self.subTest(name=name), self.assertRaises(ValueError):
                ci.library_paths([{**self.entry, "libraries": [name]}], self.root)
        outside = self.root.parent / (self.root.name + "_outside.cpp")
        outside.write_text("int outside;\n")
        self.addCleanup(outside.unlink)
        (self.root / "Algorithm/link.cpp").symlink_to(outside)
        with self.assertRaises(ValueError):
            ci.library_paths([{**self.entry, "libraries": ["Algorithm/link.cpp"]}], self.root)

    def test_runtime_options_are_owned_without_mutating_callers_environment(self):
        base = {"ASAN_OPTIONS": "detect_leaks=0:halt_on_error=0", "UBSAN_OPTIONS": "halt_on_error=0",
                "LSAN_OPTIONS": "exitcode=0", "ASAN_SYMBOLIZER_PATH": "/untrusted",
                "LLVM_PROFILE_FILE": "/old", "KEEP": "yes"}
        sanitized = ci.runtime_environment("sanitizers", base=base)
        for key, value in ci.SANITIZER_ENV.items():
            self.assertEqual(sanitized[key], value)
        self.assertNotIn("ASAN_SYMBOLIZER_PATH", sanitized)
        self.assertEqual(sanitized["KEEP"], "yes")
        self.assertEqual(base["ASAN_OPTIONS"], "detect_leaks=0:halt_on_error=0")
        self.assertEqual(ci.runtime_environment("coverage", "/new", base)["LLVM_PROFILE_FILE"], "/new")
        with self.assertRaises(ValueError):
            ci.runtime_environment("coverage")

    def test_coverage_filters_files_and_denominator_not_just_presentation(self):
        good = self.root / self.entry["libraries"][0]
        names = [good, self.root / self.entry["driver"], self.root / "Fast/GCC/Algorithm/f.cpp",
                 self.root / "Verify/current/vendor/atcoder/x.hpp", self.root / "Algorithm/unselected.cpp"]
        data = {"type": "llvm.coverage.json.export", "version": "3.0.1", "data": [{
            "files": [{"filename": str(path), "summary": {
                "lines": {"count": 4, "covered": 3}, "branches": {"count": 2, "covered": 1}}}
                      for path in names], "totals": {"lines": {"count": 999, "covered": 999}}}]}
        filtered = ci.filter_coverage(data, [good])
        self.assertEqual([f["filename"] for f in filtered["data"][0]["files"]], [str(good)])
        self.assertEqual(filtered["data"][0]["totals"]["lines"], {"count": 4, "covered": 3, "percent": 75})
        self.assertEqual(filtered["data"][0]["totals"]["branches"]["percent"], 50)

    def test_llvm_major_mismatch_cannot_pass(self):
        with patch.object(ci, "command_version", side_effect=[("clang version 21", 21), ("LLVM version 18", 18)]):
            result = ci.run([self.entry], root=self.root, output=self.root / "result", mode="coverage",
                            compiler="clang", llvm_profdata="llvm-profdata")
        self.assertEqual(result["status"], "FAIL")
        self.assertIn("同じmajor", result["error"])
        self.assertEqual(result["results"], [])

    def test_profile_mismatch_warning_is_an_error_even_with_zero_exit(self):
        returned = subprocess.CompletedProcess([], 0, "{}", "warning: 2 functions have mismatched data\n")
        with patch.object(ci.subprocess, "run", return_value=returned), self.assertRaisesRegex(ValueError, "profile"):
            ci.coverage_command(["llvm-cov"], self.root / "warning.txt", 10)
        self.assertIn("mismatched", (self.root / "warning.txt").read_text())

    def test_union_normalizes_tu_file_ids_and_excludes_macro_interiors(self):
        path = self.root / self.entry["libraries"][0]
        def exported(file_id, true_count, false_count):
            return {"type": "llvm.coverage.json.export", "version": "3.0.1", "data": [{"files": [{
                "filename": str(path), "summary": {},
                "branches": [[2, 6, 2, 11, true_count, false_count, file_id, file_id, 4]],
                "expansions": [{"branches": [[99, 1, 99, 2, 5, 5, file_id, 0, 4]]}]}],
                "functions": [{"filenames": [str(path)] * (file_id + 1), "branches": [
                    [2, 6, 2, 11, true_count, false_count, file_id, file_id, 4],
                    [99, 1, 99, 2, 5, 5, file_id, 0, 4]]}]}]}
        lcov = f"SF:{path}\nDA:1,1\nDA:2,1\nDA:3,0\nend_of_record\n"
        first = ci.source_coverage(exported(7, 1, 0), lcov, [path])
        second = ci.source_coverage(exported(19, 0, 1), lcov.replace("DA:3,0", "DA:3,1"), [path])
        result = ci.coverage_union([first, second, first])
        self.assertEqual(result["totals"]["branches"], {"count": 2, "covered": 2, "percent": 100})
        self.assertEqual(result["totals"]["lines"], {"count": 3, "covered": 3, "percent": 100})
        self.assertEqual(result["files"][0]["branches"], [[2, 6, 2, 11, True, True]])

    def test_unreached_runtime_branch_overrides_folded_template_instance(self):
        fixture = self.measured_coverage(true_hit=False, false_hit=False)
        file = fixture["files"][0]
        region = [2, 6, 2, 11]
        folded_only = {"files": [{**file, "folded_branches": [region]}]}
        self.assertEqual(ci.coverage_union([folded_only])["files"][0]["folded_branches"], [region])
        self.assertNotIn("folded_branches", ci.coverage_union([folded_only, fixture])["files"][0])
        name = self.entry["libraries"][0]
        with self.assertRaisesRegex(ValueError, "原本の計測と一致しません"):
            ci.coverage_file_summary({**file, "branches": [region + [True, False]],
                                      "folded_branches": [region]}, name, "", {})

    def measured_coverage(self, *, line_hit=True, true_hit=True, false_hit=True):
        path = self.root / self.entry["libraries"][0]
        return ci.coverage_union([{"files": [{"filename": str(path),
            "lines": [[1, line_hit]], "branches": [[2, 6, 2, 11, true_hit, false_hit]]}]}])

    def test_gcov_only_confirms_existing_llvm_lines_with_positive_counts(self):
        measured = self.measured_coverage(line_hit=False, false_hit=False)
        measured["files"][0]["lines"].append((3, False))
        report = {"format_version": "2", "gcc_version": "13.3.0",
                  "current_working_directory": str(self.root), "files": [
            {"file": "ignored.cpp", "lines": [{"line_number": 3, "count": 3}]},
            {"file": self.entry["libraries"][0], "lines": [
                {"line_number": 1, "count": 7}, {"line_number": 2, "count": 4},
                {"line_number": 3, "count": 0}]}]}
        ci.record_gcov_lines(measured, report)
        self.assertEqual(measured["files"][0]["lines"], [(1, False), (3, False)])
        result = ci.coverage_union([measured])
        self.assertEqual(result["files"][0]["lines"], [(1, True), (3, False)])
        self.assertEqual(result["files"][0]["gcov_line_counts"], [(1, 7)])
        self.assertEqual(result["files"][0]["branches"], [[2, 6, 2, 11, True, False]])
        report["files"].pop()
        with self.assertRaisesRegex(ValueError, "対応が不完全"):
            ci.record_gcov_lines(measured, report)

    def test_unchecked_gcov_only_confirms_existing_lines_and_preserves_provenance(self):
        path = self.root / self.entry["libraries"][0]
        file = {"filename": str(path), "lines": [[1, False], [2, False]],
                "branches": [[2, 6, 2, 11, True, False]]}
        original = {"files": [file]}
        report = {"format_version": "2", "gcc_version": "13.3.0",
                  "current_working_directory": str(self.root), "files": [
                      {"file": str(path), "lines": [{"line_number": 1, "count": 0},
                                                   {"line_number": 2, "count": 4},
                                                   {"line_number": 3, "count": 7}]}]}
        ci.record_gcov_lines(original, report, unchecked=True)
        result = ci.coverage_union([original])
        row, = result["files"]
        self.assertEqual(row["lines"], [(1, False), (2, True)])
        self.assertEqual(row["gcov_unchecked_line_counts"], [(2, 4)])
        self.assertNotIn("gcov_line_counts", row)
        self.assertEqual(result["totals"]["branches"]["covered"], 1)
        self.assertEqual(result["totals"]["lines"]["count"], 2)
        with self.assertRaisesRegex(ValueError, "単独テスト"):
            ci.translation_unit(self.entry, self.root, unchecked=True)

    def gate(self, coverage, **kwargs):
        settings = {"expected_translation_units": 1, "measured_translation_units": 1, **kwargs}
        return ci.coverage_gate(coverage, ci.library_paths([self.entry], self.root), **settings)

    def test_coverage_gate_requires_both_line_and_branch_counts_to_be_complete(self):
        self.assertEqual(self.gate(self.measured_coverage())["status"], "PASS")
        for setting, reason in [({"line_hit": False}, "uncovered_lines"),
                                ({"false_hit": False}, "uncovered_branches")]:
            with self.subTest(setting=setting):
                gate = self.gate(self.measured_coverage(**setting))
                self.assertEqual(gate["required_percent"], 100)
                self.assertEqual(gate["status"], "FAIL")
                self.assertIn(reason, gate["reasons"])

    def test_coverage_shares_only_matching_measured_coordinates_in_identical_guard_blocks(self):
        body = ("#ifndef SIRKIT_SHARED_DEFINED\n#define SIRKIT_SHARED_DEFINED\n"
                "int shared(int x) {\n if (x) return 1;\n return 0;\n}\n#endif\n")
        original = self.root / "Algorithm/Shared.cpp"
        copy = original.with_name("Copy.cpp")
        different = original.with_name("Different.cpp")
        conditional = original.with_name("Conditional.cpp")
        original.write_text(body)
        copy.write_text("// Header\n\n" + body)
        different.write_text(body.replace("return 1;", "return 2;"))
        conditional.write_text("#ifdef optional\n" + body + "#endif\n")
        shared = ci.shared_coverage_blocks(self.root)
        self.assertEqual(len(shared), 1)
        self.assertCountEqual(shared[0], [(str(original), 1, 7), (str(copy), 3, 9)])
        measured = {"files": [
            {"filename": str(original), "lines": [[4, True]], "branches": [[4, 6, 4, 7, True, False]]},
            {"filename": str(copy), "lines": [[6, False], [7, False]], "branches": [[6, 6, 6, 7, False, True]]},
        ]}
        rows = {row["filename"]: row for row in ci.coverage_union([measured], shared)["files"]}
        self.assertEqual(rows[str(original)]["branches"], [[4, 6, 4, 7, True, True]])
        self.assertEqual(rows[str(copy)]["lines"], [(6, True), (7, False)])
        self.assertEqual(rows[str(copy)]["branches"], [[6, 6, 6, 7, True, True]])

    def test_shared_proof_respects_observed_hits_and_keeps_direct_proofs_source_bound(self):
        body = ("#ifndef SIRKIT_SHARED_DEFINED\n#define SIRKIT_SHARED_DEFINED\n"
                "int shared(int x) {\n if (x) return 1;\n return 0;\n}\n#endif\n")
        name, alias = "Algorithm/Shared.cpp", "Algorithm/Copy.cpp"
        original, copy = self.root / name, self.root / alias
        original.write_text(body)
        copy.write_text("// Header\n" + body)
        registered = self.root / ci.COVERAGE_EXCLUSIONS
        registered.parent.mkdir(parents=True)
        registered.write_text(json.dumps({"format": 1, "files": {
            name: {"sha256": ci.digest(original), "regions": [
                {"region": [4, 6, 4, 7], "side": "false", "reason": "Invariant for the original site"}
            ]}
        }}))
        shared = ci.shared_coverage_blocks(self.root)
        exclusions = ci.load_coverage_exclusions(self.root, shared)
        self.assertEqual(exclusions[alias]["regions"][0]["shared_from"], name)
        metrics, applied = ci.coverage_file_summary({
            "lines": [[5, True]], "branches": [[5, 6, 5, 7, True, False]],
        }, alias, ci.digest(copy), exclusions)
        self.assertEqual(metrics["branches"]["count"], 1)
        self.assertEqual(applied, exclusions[alias]["regions"])
        measured = {"files": [
            {"filename": str(original), "lines": [[4, True]], "branches": [[4, 6, 4, 7, True, False]]},
            {"filename": str(copy), "lines": [[5, True]], "branches": [[5, 6, 5, 7, False, True]]},
        ]}
        coverage = ci.coverage_union([measured], shared,
                                     direct_exclusions=ci.direct_exclusion_coordinates(self.root, exclusions))
        result = ci.apply_coverage_exclusions(coverage, self.root, {
            name: ci.digest(original), alias: ci.digest(copy),
        }, exclusions)
        rows = {row["filename"]: row for row in result["files"]}
        self.assertEqual(rows[str(original)]["branches"], [[4, 6, 4, 7, True, False]])
        self.assertEqual(rows[str(copy)]["branches"], [[5, 6, 5, 7, True, True]])
        self.assertEqual(rows[str(copy)]["exclusions"], [])
        self.assertEqual(result["totals"]["branches"]["percent"], 100)
        original.write_text("// Source digest changed\n" + body)
        self.assertNotIn(alias, ci.load_coverage_exclusions(self.root))

    def test_proved_exclusions_preserve_raw_evidence_and_only_remove_matching_coordinates(self):
        name = self.entry["libraries"][0]
        source = self.root / name
        digest = ci.digest(source)
        coverage = self.measured_coverage(false_hit=False)
        coverage["files"][0]["lines"].append([3, False])
        coverage = ci.coverage_union([coverage])
        exclusions = [{"line": 3, "reason": "戻り値の範囲から到達不能"},
                      {"region": [2, 6, 2, 11], "side": "false", "reason": "分岐時点の不変条件"},
                      {"line": 4, "reason": "このビルドでは計測対象外"}]
        policy = {name: {"sha256": digest, "regions": exclusions}}
        registered = self.root / ci.COVERAGE_EXCLUSIONS
        registered.parent.mkdir(parents=True)
        registered.write_text(json.dumps({"format": 1, "files": policy}))
        result = ci.apply_coverage_exclusions(coverage, self.root, {name: digest},
                                              ci.load_coverage_exclusions(self.root))
        self.assertEqual(result["raw_totals"], coverage["totals"])
        self.assertEqual(result["files"][0]["lines"], [(1, True), (3, False)])
        self.assertEqual(result["files"][0]["branches"], [[2, 6, 2, 11, True, False]])
        self.assertEqual(result["files"][0]["exclusions"], exclusions[:2])
        self.assertEqual(self.gate(result)["status"], "PASS")
        html = self.root / "coverage_html"
        ci.write_coverage_html(result, self.root, html)
        page = (html / "source_0000.html").read_text()
        self.assertIn("除外（戻り値の範囲から到達不能）", page)
        self.assertIn("T:✓ F:除外（分岐時点の不変条件）", page)
        self.assertNotIn("最初の未到達へ", page)

        with self.assertRaisesRegex(ValueError, "現在の原本と一致しません"):
            ci.apply_coverage_exclusions(coverage, self.root, {name: "0" * 64}, policy)
        with self.assertRaisesRegex(ValueError, "に到達しました"):
            ci.apply_coverage_exclusions(self.measured_coverage(), self.root, {name: digest}, policy)
        with self.assertRaisesRegex(ValueError, "に到達しました"):
            ci.coverage_file_summary(coverage["files"][0], name, digest, {
                name: {"sha256": digest, "regions": [{"line": 1, "reason": "不正な証明"}]}
            })

    def test_coverage_html_uses_counts_for_color_and_links_to_a_missed_branch(self):
        coverage = self.measured_coverage(false_hit=False)
        directory = self.root / "coverage_html"
        ci.write_coverage_html(coverage, self.root, directory)
        index = (directory / "index.html").read_text()
        self.assertIn('<td class="rate hit"><strong>100%</strong> <span class="figures">(1/1)', index)
        self.assertIn('<td class="rate miss"><strong>50.000%</strong> <span class="figures">(1/2)', index)
        page = (directory / "source_0000.html").read_text()
        self.assertIn('<a href="#L2">最初の未到達へ（2行）</a>', page)
        self.assertIn('<td class="miss">2:6–2:11 T:✓ F:×</td>', page)

        coverage["files"][0]["branches"] = []
        without_branches = self.root / "without_branches"
        ci.write_coverage_html(ci.coverage_union([coverage]), self.root, without_branches)
        index = (without_branches / "index.html").read_text()
        self.assertIn('<td class="rate not-measured"><strong>直接分岐なし</strong>', index)

    def test_exclusions_reject_missing_reason_or_duplicate_coordinates(self):
        name = self.entry["libraries"][0]
        registry = self.root / ci.COVERAGE_EXCLUSIONS
        registry.parent.mkdir(parents=True)
        for records in ([{"line": 3, "reason": " "}],
                        [{"line": 3, "reason": "理由A"}, {"line": 3, "reason": "理由B"}]):
            with self.subTest(records=records):
                registry.write_text(json.dumps({"format": 1, "files": {
                    name: {"sha256": ci.digest(self.root / name), "regions": records}
                }}))
                with self.assertRaises(ValueError):
                    ci.load_coverage_exclusions(self.root)

    def test_excluding_all_measured_lines_still_fails(self):
        name = self.entry["libraries"][0]
        digest = ci.digest(self.root / name)
        result = ci.apply_coverage_exclusions(self.measured_coverage(line_hit=False), self.root, {name: digest}, {
            name: {"sha256": digest, "regions": [{"line": 1, "reason": "合成テストの到達不能行"}]}
        })
        gate = self.gate(result)
        self.assertEqual(gate["status"], "FAIL")
        self.assertEqual(gate["unmapped_sources"], [str(self.root / name)])

    def test_coverage_gate_uses_exact_counts_not_a_rounded_percent(self):
        for key in ("lines", "branches"):
            with self.subTest(metric=key):
                coverage = self.measured_coverage()
                coverage["totals"][key] = {"count": 1000000, "covered": 999999, "percent": 100.0}
                gate = self.gate(coverage)
                self.assertEqual(gate["status"], "FAIL")
                self.assertEqual(gate[key]["uncovered"], 1)
                text = ci.summary_markdown({"mode": "coverage", "status": "FAIL", "execution_status": "PASS",
                    "selected_ids": ["example"], "results": [], "coverage_gate": gate,
                    "coverage_totals": coverage["totals"]})
                self.assertIn("<100%", text)

    def test_zero_branches_are_vacuous_only_with_nonzero_mapped_lines(self):
        coverage = self.measured_coverage()
        coverage["files"][0]["branches"] = []
        coverage["totals"]["branches"] = {"count": 0, "covered": 0, "percent": 0}
        gate = self.gate(coverage)
        self.assertEqual(gate["status"], "PASS")
        self.assertEqual(gate["branches"], {"count": 0, "covered": 0, "uncovered": 0})
        empty = ci.coverage_union([])
        gate = self.gate(empty)
        self.assertEqual(gate["status"], "FAIL")
        self.assertFalse(gate["nonzero_mapped_lines"])
        self.assertIn("no_mapped_lines", gate["reasons"])

    def test_missing_or_zero_line_selected_source_fails_instead_of_shrinking_denominator(self):
        coverage = self.measured_coverage()
        other = self.root / "Algorithm/other.cpp"
        other.write_text("int other(){return 1;}\n")
        for mapped_empty in (False, True):
            with self.subTest(mapped_empty=mapped_empty):
                data = {**coverage, "files": list(coverage["files"])}
                if mapped_empty:
                    data["files"].append({"filename": str(other), "lines": [], "branches": []})
                gate = ci.coverage_gate(data, [self.root / self.entry["libraries"][0], other],
                                        expected_translation_units=1, measured_translation_units=1)
                self.assertEqual(gate["status"], "FAIL")
                self.assertEqual(gate["unmapped_sources"], [str(other)])
                self.assertEqual(gate["lines"]["count"], coverage["totals"]["lines"]["count"])

    def test_partial_or_empty_translation_unit_measurement_cannot_pass(self):
        for expected, measured in ((2, 1), (0, 0)):
            with self.subTest(expected=expected, measured=measured):
                gate = self.gate(self.measured_coverage(), expected_translation_units=expected,
                                 measured_translation_units=measured)
                self.assertEqual(gate["status"], "FAIL")
                self.assertFalse(gate["complete_translation_units"])
                self.assertIn("incomplete_translation_units", gate["reasons"])

    def test_coverage_cli_propagates_failed_gate(self):
        with patch.object(ci.prepare_verify, "collect", return_value=[self.entry]), \
                patch.object(ci, "run", return_value={"status": "FAIL"}):
            self.assertEqual(ci.main(["--mode", "coverage", "--output", str(self.root / "result")]), 1)

    def test_timeout_or_missing_sample_never_passes(self):
        for missing in (False, True):
            entry = {**self.entry, "samples": []} if missing else self.entry
            responses = [subprocess.CompletedProcess([], 0, "", "")]
            if not missing:
                responses.append(subprocess.TimeoutExpired(["binary"], 0.01))
            with patch.object(ci, "command_version", return_value=("clang version 21", 21)), \
                    patch.object(ci, "gcc_install_arguments", return_value=[]), \
                    patch.object(ci.subprocess, "run", side_effect=responses):
                result = ci.run([entry], root=self.root, output=self.root / str(missing),
                                mode="sanitizers", compiler="clang")
            self.assertEqual(result["status"], "FAIL")
            self.assertEqual(result["results"][0]["status"], "FAIL")

    def test_no_target_and_existing_output_cannot_reuse_a_pass(self):
        result = ci.run([], root=self.root, output=self.root / "empty", mode="coverage", compiler="clang")
        self.assertEqual(result["status"], "FAIL")
        self.assertNotIn("状態: PASS", (self.root / "empty/summary.md").read_text())
        with self.assertRaises(FileExistsError):
            ci.run([self.entry], root=self.root, output=self.root / "empty", mode="coverage", compiler="clang")

    def test_qemu_sysroot_also_supplies_the_compiler_inputs(self):
        sysroot = self.root / "amd64"
        libraries = sysroot / "usr/lib/x86_64-linux-gnu"
        resource_dir = sysroot / "usr/lib/llvm-18/lib/clang/18"
        loader = libraries / "ld-linux-x86-64.so.2"
        for path in (loader, libraries / "Scrt1.o", libraries / "libc.so.6", libraries / "libm.so.6",
                     libraries / "libstdc++.so.6", libraries / "libgcc_s.so.1", resource_dir / "include/stddef.h",
                     resource_dir / "lib/linux/libclang_rt.profile-x86_64.a"):
            path.parent.mkdir(parents=True, exist_ok=True)
            path.touch()

        def execute(command, **options):
            if "env" in options:
                Path(options["env"]["LLVM_PROFILE_FILE"].replace("%p", "1")).write_bytes(b"profile")
            return subprocess.CompletedProcess(command, 0, stdout="1\n", stderr="")

        baseline = {**self.entry, "id": "baseline", "x86_baseline_coverage": True}
        with (patch.object(ci, "command_version", return_value=("tools", 18)),
              patch.object(ci, "gcc_install_arguments", return_value=["--gcc-install-dir=/host-gcc"]),
              patch.object(ci.subprocess, "run", side_effect=execute),
              patch.object(ci, "export_clang_coverage", return_value=ci.coverage_union([]))):
            report = ci.run([self.entry, baseline], root=self.root, output=self.root / "result",
                            mode="coverage", compiler="clang", x86_baseline_sysroot=sysroot)

        self.assertEqual(report["execution_status"], "PASS", report)
        native, emulated = report["results"]
        self.assertIn("--gcc-install-dir=/host-gcc", native["command"])
        self.assertNotIn("--gcc-install-dir=/host-gcc", emulated["command"])
        self.assertEqual(emulated["command"][1:5], [f"--sysroot={sysroot}", f"--gcc-toolchain={sysroot / 'usr'}",
                                                    f"-resource-dir={resource_dir}", "-march=x86-64"])
        self.assertEqual(report["commands"], [["qemu-x86_64", "-cpu", ci.X86_BASELINE_CPU, str(loader),
                                                "--inhibit-cache", "--library-path", str(libraries),
                                                emulated["command"][-1]]])


def toolchain(coverage):
    for suffix in ("-21", "-18", "-20", "-19", ""):
        compiler = shutil.which("clang++" + suffix)
        profdata = shutil.which("llvm-profdata" + suffix)
        cov = shutil.which("llvm-cov" + suffix)
        if compiler and (not coverage or (profdata and cov)):
            return compiler, profdata, cov
    raise unittest.SkipTest("必要なClang/LLVMツールがありません")


class CoverageIntegrationTests(unittest.TestCase):
    def test_coverage_shards_use_the_same_full_gate_and_reject_missing_exports(self):
        compiler, profdata, cov = toolchain(True)
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            positive = fixture(root)
            negative = {**positive, "id": "negative", "samples": [{"input": "0\n", "output": "0\n"}]}
            entries = [positive, negative]
            for index, entry in enumerate(entries):
                partial = ci.run([entry], root=root, output=root / f"coverage-part-{index}",
                                 mode="coverage", compiler=compiler, llvm_profdata=profdata,
                                 llvm_cov=cov, available=entries, coverage_shard=(index, 2))
                self.assertEqual(partial["status"], "PASS", partial)
                self.assertNotIn("coverage_gate", partial)
            full = ci.run(entries, root=root, output=root / "full", mode="coverage", compiler=compiler,
                          llvm_profdata=profdata, llvm_cov=cov)
            report = merge_ci_coverage.merge(entries, root, root, root / "merged", 2, {})
            self.assertEqual(report["status"], "PASS", report)
            self.assertEqual(report["coverage_gate"], full["coverage_gate"])
            self.assertEqual((root / "merged/coverage.json").read_bytes(), (root / "full/coverage.json").read_bytes())
            (root / "coverage-part-1/coverage_entries/entry_0001.json").unlink()
            with self.assertRaises(FileNotFoundError):
                merge_ci_coverage.merge(entries, root, root, root / "incomplete", 2, {})
            self.assertFalse((root / "incomplete/results.json").exists())

    def test_binary128_measures_both_long_double_formats(self):
        if os.uname().machine != "x86_64":
            self.skipTest("x86-64のlong double設定を検証します")
        compiler, profdata, cov = toolchain(True)
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            entry = fixture(root, body="""int instrumented(int x) {
#if __LDBL_MANT_DIG__ == 113
    return x + 1 - 1;
#else
    return x;
#endif
}
""")
            entry["x86_binary128_coverage"] = True
            output = root / "result"
            report = ci.run([entry], root=root, output=output, mode="coverage", compiler=compiler,
                            llvm_profdata=profdata, llvm_cov=cov)
            self.assertEqual(report["execution_status"], "PASS", report)
            alternate = report["results"][0]["x86_binary128_coverage"]
            self.assertIn("-mlong-double-128", alternate["command"])
            self.assertTrue(alternate["samples"][0]["profile_written"])
            row, = json.loads((output / "coverage.json").read_text())["files"]
            self.assertTrue(dict(row["lines"])[3])
            self.assertTrue(dict(row["lines"])[5])

    def test_avx2_compiler_configuration_is_executed_on_matching_cpu(self):
        emulator = shutil.which("qemu-x86_64")
        if not emulator or os.uname().machine != "x86_64":
            self.skipTest("x86-64のQEMUがありません")
        compiler, profdata, cov = toolchain(True)
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            entry = fixture(root, body="""int instrumented(int x) {
#ifdef __AVX2__
    return x + 1 - 1;
#else
    return x;
#endif
}
""")
            entry["x86_avx2_coverage"] = True
            output = root / "result"
            report = ci.run([entry], root=root, output=output, mode="coverage", compiler=compiler,
                            llvm_profdata=profdata, llvm_cov=cov, qemu_x86=emulator)
            self.assertEqual(report["execution_status"], "PASS", report)
            alternate = report["results"][0]["x86_avx2_coverage"]
            self.assertIn("-mavx2", alternate["command"])
            self.assertEqual(alternate["execution_command"][2], ci.X86_AVX2_CPU)
            self.assertTrue(alternate["samples"][0]["profile_written"])
            row, = json.loads((output / "coverage.json").read_text())["files"]
            self.assertTrue(dict(row["lines"])[3])
            self.assertTrue(dict(row["lines"])[5])

    def test_clang_unchecked_measures_both_configurations_without_disabling_assertions(self):
        compiler, profdata, cov = toolchain(True)
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            entry = fixture(root, body="""int instrumented(int x) {
#ifdef SKIP_RUNTIME_CHECKS
    return x == 1 ? 10 : 20;
#else
    return x == 1 ? 11 : 21;
#endif
}
""")
            name = "alternate_test.cpp"
            driver = root / "Verify/current/tests" / name
            driver.parent.mkdir()
            driver.write_text("""#include "../../../Algorithm/選択.cpp"
#include <cassert>
int main() {
#ifdef SKIP_RUNTIME_CHECKS
    assert(instrumented(1) == 10);
    assert(instrumented(2) == 20);
#else
    assert(instrumented(1) == 11);
    assert(instrumented(2) == 21);
#endif
}
""")
            registry = root / ci.COVERAGE_TESTS
            registry.parent.mkdir()
            registry.write_text(json.dumps({"format": 1, "tests": {name: entry["libraries"]},
                                           "clang_unchecked_tests": [name]}))
            alternate, _ = ci.coverage_tests(root)
            output = root / "result"
            report = ci.run([alternate], root=root, output=output, mode="coverage", compiler=compiler,
                            llvm_profdata=profdata, llvm_cov=cov)
            self.assertEqual(report["execution_status"], "PASS", report)
            self.assertIn("-USKIP_RUNTIME_CHECKS", report["results"][0]["command"])
            supplementary = report["results"][0]["clang_unchecked_coverage"]
            self.assertIn("-DSKIP_RUNTIME_CHECKS", supplementary["command"])
            self.assertTrue(supplementary["samples"][0]["profile_written"])
            row, = json.loads((output / "coverage.json").read_text())["files"]
            self.assertEqual({branch[0]: branch[4:] for branch in row["branches"]},
                             {3: [True, True], 5: [True, True]})

    def test_compiler_folded_branches_are_distinct_from_unexecuted_runtime_conditions(self):
        compiler, profdata, cov = (shutil.which(tool + "-18") for tool in ("clang++", "llvm-profdata", "llvm-cov"))
        if not (compiler and profdata and cov):
            self.skipTest("CIと同じLLVM 18でコンパイル時確定分岐を検証します")
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            entry = fixture(root, body="""#define NESTED(x) ((x) ? 1 : 0)
static int nested(int x) {
    if (x < 0) return -1;
    return 0;
}
int instrumented(int x) {
    if (false) return -2;
    if (x == 0) return nested(x);
    return NESTED(x);
}
""")
            output = root / "result"
            report = ci.run([entry], root=root, output=output, mode="coverage", compiler=compiler,
                            llvm_profdata=profdata, llvm_cov=cov)
            self.assertEqual(report["execution_status"], "PASS", report)
            self.assertEqual(report["coverage_gate"]["status"], "FAIL")
            row, = json.loads((output / "coverage.json").read_text())["files"]
            folded, = row["folded_branches"]
            self.assertEqual(folded[0], 7)
            raw = {key[0]: key[4:] for key in row["branches"]}
            self.assertEqual(raw, {3: [False, False], 7: [False, False], 8: [False, True]})
            self.assertEqual(row["summary"]["branches"]["count"], 4)
            self.assertIn("コンパイル時に確定", (output / "html/source_0000.html").read_text())

    def test_qemu_cpu_feature_false_path_is_measured_by_the_same_binary(self):
        emulator = shutil.which("qemu-x86_64")
        if not emulator or os.uname().machine != "x86_64":
            self.skipTest("x86-64のQEMUがありません")
        compiler, profdata, cov = toolchain(True)
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            entry = fixture(root, body="""int instrumented(int x) {
    if (__builtin_cpu_supports("popcnt")) return x;
    return x < 0 ? -x : x;
}
""")
            entry["x86_baseline_coverage"] = True
            output = root / "result"
            report = ci.run([entry], root=root, output=output, mode="coverage", compiler=compiler,
                            llvm_profdata=profdata, llvm_cov=cov, qemu_x86=emulator)
            self.assertEqual(report["execution_status"], "PASS", report)
            sample, = report["results"][0]["samples"]
            self.assertTrue(sample["x86_baseline"]["profile_written"])
            row, = json.loads((output / "coverage.json").read_text())["files"]
            feature, = [branch for branch in row["branches"] if branch[0] == 2]
            self.assertTrue(feature[5])
            self.assertTrue(dict(row["lines"])[3])

    def test_gcov_distinguishes_reached_return_from_unreached_return(self):
        compiler, profdata, cov = toolchain(True)
        gcc, gcov = (shutil.which(tool, path=os.defpath) for tool in ("g++", "gcov"))
        if not gcc or not gcov:
            self.skipTest("GCCとgcovがありません")
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            entry = fixture(root, body="""template<bool A> static int choose(int x) {
    if (x > 0) {
        if constexpr (A) return 1;
        else return 2;
    }
    return 3;
}
int instrumented(int x) { return choose<true>(x) + choose<false>(x); }
""")
            for number, expected in ((1, 3), (-1, 6)):
                selected = {**entry, "gcc_line_coverage": True,
                            "samples": [{"input": f"{number}\n", "output": f"{expected}\n"}]}
                output = root / str(number)
                report = ci.run([selected], root=root, output=output, mode="coverage", compiler=compiler,
                                llvm_profdata=profdata, llvm_cov=cov, gcc=gcc, gcov=gcov)
                self.assertEqual(report["execution_status"], "PASS", report["results"])
                self.assertIn("version", report["results"][0]["gcc_line_coverage"])
                row = json.loads((output / "coverage.json").read_text())["files"][0]
                self.assertEqual(dict(row["lines"])[6], number < 0)
                if number > 0:
                    self.assertNotIn(6, dict(row.get("gcov_line_counts", [])))

    def test_real_original_lines_and_branches_exclude_driver_and_artifact_binaries(self):
        compiler, profdata, cov = toolchain(True)
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            entry = fixture(root)
            result = ci.run([entry], root=root, output=root / "result", mode="coverage", compiler=compiler,
                            llvm_profdata=profdata, llvm_cov=cov)
            self.assertEqual(result["status"], "FAIL", result.get("error"))
            self.assertEqual(result["execution_status"], "PASS")
            self.assertEqual(result["coverage_gate"]["status"], "FAIL")
            self.assertEqual(result["coverage_gate"]["branches"]["uncovered"], 1)
            data = json.loads((root / "result/coverage.json").read_text())
            self.assertEqual([f["filename"] for f in data["files"]], [str(root / entry["libraries"][0])])
            self.assertEqual(data["totals"]["branches"]["count"], 2)
            self.assertEqual(data["totals"]["branches"]["covered"], 1)
            self.assertIn([2, True], data["files"][0]["lines"])
            self.assertTrue((root / "result/html/index.html").is_file())
            self.assertFalse(list((root / "result").rglob("*.profraw")))
            self.assertFalse(list((root / "result").rglob("*.profdata")))
            self.assertFalse(any(path.is_file() and os.access(path, os.X_OK) for path in (root / "result").rglob("*")))
            text = (root / "result/summary.md").read_text()
            self.assertIn("未実体化template", text)
            self.assertIn("ローカル補足 1件", text)
            self.assertIn("ビルド・入力照合・計測: PASS", text)
            self.assertIn("100%必須判定（行・直接分岐）: FAIL", text)
            self.assertIn("未到達:", text)

    def test_two_real_translation_units_cover_opposite_sides_without_double_counting(self):
        compiler, profdata, cov = toolchain(True)
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            first = fixture(root)
            second_driver = root / "Verify/current/drivers/second.cpp"
            second_driver.write_text('#include <iostream>\nint main(){int x;std::cin>>x;'
                                     'volatile int extra=0;if(extra)return 1;std::cout<<instrumented(x)<<"\\n";}\n')
            second = {**first, "id": "negative", "driver": str(second_driver.relative_to(root)),
                      "samples": [{"input": "-1\n", "output": "0\n"}]}
            serial = ci.run([first, second], root=root, output=root / "serial", mode="coverage",
                            compiler=compiler, llvm_profdata=profdata, llvm_cov=cov)
            result = ci.run([first, second], root=root, output=root / "result", mode="coverage",
                            compiler=compiler, llvm_profdata=profdata, llvm_cov=cov, jobs=2)
            self.assertEqual(result["jobs"], 2)
            self.assertEqual(result["coverage_gate"], serial["coverage_gate"])
            self.assertEqual(result["status"], "PASS", result.get("error"))
            self.assertEqual(result["execution_status"], "PASS")
            self.assertEqual(result["coverage_gate"]["status"], "PASS")
            self.assertEqual(result["coverage_gate"]["expected_translation_units"], 2)
            self.assertEqual(result["coverage_gate"]["measured_translation_units"], 2)
            data = json.loads((root / "result/coverage.json").read_text())
            self.assertEqual(data, json.loads((root / "serial/coverage.json").read_text()))
            self.assertEqual(len(data["files"]), 1)
            self.assertEqual(data["totals"]["branches"], {"count": 2, "covered": 2, "percent": 100})
            self.assertEqual(data["totals"]["lines"]["count"], 4)
            self.assertEqual(data["totals"]["lines"]["covered"], 4)
            per_tu = [json.loads((root / "result" / row["coverage_path"]).read_text())
                      for row in result["results"]]
            self.assertEqual(per_tu[0]["files"][0]["branches"][0][-2:], [True, False])
            self.assertEqual(per_tu[1]["files"][0]["branches"][0][-2:], [False, True])
            self.assertTrue(all("-object=" not in arg for command in result["commands"] for arg in command))

    def test_real_no_direct_branches_passes_with_nonzero_lines(self):
        compiler, profdata, cov = toolchain(True)
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            entry = fixture(root, body="int instrumented(int x){return x;}\n")
            result = ci.run([entry], root=root, output=root / "result", mode="coverage", compiler=compiler,
                            llvm_profdata=profdata, llvm_cov=cov)
            self.assertEqual(result["status"], "PASS", result.get("error"))
            self.assertGreater(result["coverage_gate"]["lines"]["count"], 0)
            self.assertEqual(result["coverage_gate"]["branches"]["count"], 0)
            self.assertIn("直接分岐なし", (root / "result/summary.md").read_text())

    def test_real_unmapped_selected_source_fails_with_artifacts_preserved(self):
        compiler, profdata, cov = toolchain(True)
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            entry = fixture(root, body="int instrumented(int x){return x;}\n")
            unmapped = root / "Algorithm/unmapped.cpp"
            unmapped.write_text("namespace uninstantiated {}\n")
            entry["libraries"].append(str(unmapped.relative_to(root)))
            result = ci.run([entry], root=root, output=root / "result", mode="coverage", compiler=compiler,
                            llvm_profdata=profdata, llvm_cov=cov)
            self.assertEqual(result["status"], "FAIL", result.get("error"))
            self.assertEqual(result["execution_status"], "PASS")
            self.assertEqual(result["coverage_gate"]["unmapped_sources"], [str(unmapped)])
            self.assertEqual(result["coverage_gate"]["lines"]["uncovered"], 0)
            self.assertEqual(result["coverage_gate"]["branches"]["uncovered"], 0)
            self.assertTrue((root / "result/coverage.json").is_file())
            self.assertTrue((root / "result/html/index.html").is_file())


class SanitizerIntegrationTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.compiler, _, _ = toolchain(False)
        cls.temporary = tempfile.TemporaryDirectory()
        cls.addClassCleanup(cls.temporary.cleanup)
        root = Path(cls.temporary.name)
        entry = fixture(root)
        result = ci.run([entry], root=root, output=root / "probe", mode="sanitizers", compiler=cls.compiler)
        logs = "\n".join(path.read_text() for path in (root / "probe/logs").glob("*_sample_*.txt"))
        if "LeakSanitizer does not work under ptrace" in logs:
            raise unittest.SkipTest("この実行環境はptrace下のためLSanを実行できません。制約外の実測が必要です")
        if result["status"] != "PASS":
            raise AssertionError((result, logs))

    def test_asan_and_ubsan_failures_are_not_swallowed(self):
        cases = [
            ("address", "int instrumented(int x){auto*p=new volatile int[1];p[x]=4;int r=p[0];delete[]p;return r;}\n",
             "2\n", "AddressSanitizer: heap-buffer-overflow"),
            ("undefined", "int instrumented(int x){volatile int a=2147483647;return a+x;}\n",
             "1\n", "runtime error: signed integer overflow"),
        ]
        for name, body, value, diagnostic in cases:
            with self.subTest(name=name), tempfile.TemporaryDirectory() as temporary:
                root = Path(temporary)
                entry = fixture(root, body=body, input_text=value)
                with patch.dict(os.environ, {"ASAN_OPTIONS": "halt_on_error=0:detect_leaks=0", "UBSAN_OPTIONS": "halt_on_error=0"}):
                    result = ci.run([entry], root=root, output=root / "result", mode="sanitizers", compiler=self.compiler)
                self.assertEqual(result["status"], "FAIL")
                self.assertNotEqual(result["results"][0]["samples"][0]["returncode"], 0)
                log = (root / "result/logs/entry_0000_sample_0.txt").read_text()
                self.assertIn(diagnostic, log)
                self.assertIn("-fno-sanitize-recover=all", result["results"][0]["command"])


if __name__ == "__main__":
    unittest.main()
