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

    def test_checked_original_unicode_include_has_no_linked_submission(self):
        source = ci.translation_unit(self.entry, self.root)
        self.assertIn("選択.cpp", source)
        self.assertNotIn("\\u", source)
        self.assertIn(str(self.root / self.entry["libraries"][0]), source)
        self.assertNotIn("#define NDEBUG", source)
        self.assertNotIn("#define SKIP_RUNTIME_CHECKS", source)
        self.assertIn("#error", source)

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
                "branches": [[2, 6, 2, 11, true_count, false_count, file_id, file_id + 1, 4]],
                "expansions": [{"branches": [[99, 1, 99, 2, 5, 5, file_id, 0, 4]]}]}]}]}
        lcov = f"SF:{path}\nDA:1,1\nDA:2,1\nDA:3,0\nend_of_record\n"
        first = ci.source_coverage(exported(7, 1, 0), lcov, [path])
        second = ci.source_coverage(exported(19, 0, 1), lcov.replace("DA:3,0", "DA:3,1"), [path])
        result = ci.coverage_union([first, second, first])
        self.assertEqual(result["totals"]["branches"], {"count": 2, "covered": 2, "percent": 100})
        self.assertEqual(result["totals"]["lines"], {"count": 3, "covered": 3, "percent": 100})
        self.assertEqual(result["files"][0]["branches"], [[2, 6, 2, 11, True, True]])

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


def toolchain(coverage):
    for suffix in ("-21", "-18", "-20", "-19", ""):
        compiler = shutil.which("clang++" + suffix)
        profdata = shutil.which("llvm-profdata" + suffix)
        cov = shutil.which("llvm-cov" + suffix)
        if compiler and (not coverage or (profdata and cov)):
            return compiler, profdata, cov
    raise unittest.SkipTest("必要なClang/LLVMツールがありません")


class CoverageIntegrationTests(unittest.TestCase):
    def test_real_original_lines_and_branches_exclude_driver_and_artifact_binaries(self):
        compiler, profdata, cov = toolchain(True)
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            entry = fixture(root)
            result = ci.run([entry], root=root, output=root / "result", mode="coverage", compiler=compiler,
                            llvm_profdata=profdata, llvm_cov=cov)
            self.assertEqual(result["status"], "PASS", result.get("error"))
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
            result = ci.run([first, second], root=root, output=root / "result", mode="coverage",
                            compiler=compiler, llvm_profdata=profdata, llvm_cov=cov)
            self.assertEqual(result["status"], "PASS", result.get("error"))
            data = json.loads((root / "result/coverage.json").read_text())
            self.assertEqual(len(data["files"]), 1)
            self.assertEqual(data["totals"]["branches"], {"count": 2, "covered": 2, "percent": 100})
            self.assertEqual(data["totals"]["lines"]["count"], 4)
            self.assertEqual(data["totals"]["lines"]["covered"], 4)
            per_tu = [json.loads((root / "result" / row["coverage_path"]).read_text())
                      for row in result["results"]]
            self.assertEqual(per_tu[0]["files"][0]["branches"][0][-2:], [True, False])
            self.assertEqual(per_tu[1]["files"][0]["branches"][0][-2:], [False, True])
            self.assertTrue(all("-object=" not in arg for command in result["commands"] for arg in command))


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
