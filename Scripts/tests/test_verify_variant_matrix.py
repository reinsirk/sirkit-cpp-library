# SPDX-License-Identifier: CC0-1.0
from io import BytesIO, TextIOWrapper
from pathlib import Path
import subprocess
import sys
from tempfile import TemporaryDirectory
import unittest
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import verify_variants as variants


class VariantMatrixTests(unittest.TestCase):
    def test_common_variant_runs_under_every_requested_compiler(self):
        entries = [{"source": "Algorithm/a.cpp", "compiler": profile}
                   for profile in ("Portable", "GCC", "Clang", "MSVC")]
        selected = variants.selected_variants(entries, ("GCC", "Clang", "MSVC"), {"Algorithm/a.cpp"})
        self.assertEqual([(entry["compiler"], compiler) for compiler, entry in selected],
                         [("Portable", "GCC"), ("Portable", "Clang"), ("Portable", "MSVC"),
                          ("GCC", "GCC"), ("Clang", "Clang"), ("MSVC", "MSVC")])

    def test_only_checked_distribution_needs_skip_configuration(self):
        for compiler, skip_flag in (("GCC", "-DSKIP_RUNTIME_CHECKS"), ("Clang", "-DSKIP_RUNTIME_CHECKS"),
                                    ("MSVC", "/DSKIP_RUNTIME_CHECKS")):
            with self.subTest(compiler=compiler):
                self.assertEqual(variants.build_settings(compiler, fast=False, syntax_only=True),
                                 [("default", []), ("skip", [skip_flag])])
                self.assertEqual(variants.build_settings(compiler, fast=True, syntax_only=True), [("default", [])])

    def test_msvc_paths_with_spaces_are_separate_arguments(self):
        executable = r"C:\Program Files\Microsoft Visual Studio\VC\Tools\cl.exe"
        include = Path(r"C:\Users\Compile User\atcoder")
        with patch.object(variants.shutil, "which", return_value=executable):
            command = variants.compiler_command("MSVC", [include], None)
        self.assertEqual(command[0], executable)
        self.assertIn("/std:c++23preview", command)
        self.assertEqual(command[-2:], ["/I", str(include)])
        with patch.object(variants.shutil, "which", return_value=None):
            with self.assertRaisesRegex(ValueError, "cl.exe"):
                variants.compiler_command("MSVC", [include], None)

    def test_linux_commands_use_explicit_compiler_paths(self):
        tools = {"gcc": "g++-15", "clang": "clang++-18"}
        self.assertEqual(variants.compiler_command("GCC", [], None, **tools),
                         ["g++-15", "-std=c++23", "-O2"])
        with patch.object(variants, "gcc_install_arguments", return_value=["--gcc-install-dir=/gcc/15"]) as resolve:
            command = variants.compiler_command("Clang", [], "/gcc/15", **tools)
        resolve.assert_called_once_with("clang++-18", "/gcc/15")
        self.assertEqual(command, ["clang++-18", "--gcc-install-dir=/gcc/15", "-std=c++23", "-O2"])

    def test_fast_library_is_included_before_enabling_the_fixture_skip_guard(self):
        library = Path("temp root/library.hpp")
        fixture = Path("example.cpp")
        source = variants.runtime_source(library, fixture, "int main() {}\n", "HEADER", fast=True)
        self.assertIn('#define HEADER "temp root/library.hpp"', source)
        self.assertLess(source.index("#include HEADER"), source.index("#define SKIP_RUNTIME_CHECKS"))
        self.assertLess(source.index("#define SKIP_RUNTIME_CHECKS"), source.index("int main()"))
        safe = variants.runtime_source(library, fixture, "int main() {}\n", "HEADER", fast=False)
        self.assertNotIn("#define SKIP_RUNTIME_CHECKS", safe)

    def test_runtime_failure_is_reported_separately_from_the_build(self):
        root = Path("work dir")
        executable = root / "probe.exe"
        responses = [subprocess.CompletedProcess([], 0, stdout=b"", stderr=b""),
                     subprocess.CompletedProcess([], 2, stdout=b"", stderr=b"reference mismatch")]
        with patch.object(variants.subprocess, "run", side_effect=responses) as call:
            result = variants.run_runtime_test("MSVC", ["cl.exe"], root / "fixture.cpp", executable)
        self.assertEqual(result["compile_returncode"], 0)
        self.assertEqual(result["run_returncode"], 2)
        self.assertFalse(result["passed"])
        self.assertIn("/Fe" + str(executable), call.call_args_list[0].args[0])
        self.assertEqual(call.call_args_list[1].args[0], [str(executable)])
        summary = variants.summary_markdown({"results": [{
            "compiler": "MSVC", "path": "Fast/Portable/Example.cpp", "syntax_passed": True,
            "runtime_requested": True, "runtime": result,
        }]})
        self.assertIn("| MSVC | `Fast/Portable` | 1/1 | 0/1 |", summary)

    def test_cli_reports_status_with_windows_legacy_stdout(self):
        for passed, status in ((True, "PASS"), (False, "FAIL")):
            with self.subTest(status=status), TemporaryDirectory() as directory:
                summary = Path(directory) / "summary.md"
                args = ["verify_variants.py", "--compiler", "MSVC", "--output",
                        str(Path(directory) / "result.json"), "--summary", str(summary)]
                report = {"status": "passed" if passed else "failed", "results": [{
                    "compiler": "MSVC", "path": "Fast/Portable/Example.cpp", "passed": passed,
                    "syntax_passed": True, "runtime_requested": True, "runtime": {"passed": passed},
                }]}
                buffer = BytesIO()
                with TextIOWrapper(buffer, encoding="cp1252", errors="strict") as stdout:
                    with patch.object(sys, "stdout", stdout), patch.object(sys, "argv", args), \
                            patch.object(variants, "verify", return_value=report):
                        self.assertEqual(variants.main(), int(not passed))
                    stdout.flush()
                    self.assertIn(f"{int(passed)}/1 configurations: {status}", buffer.getvalue().decode("cp1252"))
                self.assertIn("配布版の検査", summary.read_text(encoding="utf-8"))


if __name__ == "__main__":
    unittest.main()
