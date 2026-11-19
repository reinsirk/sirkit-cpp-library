# SPDX-License-Identifier: CC0-1.0
from pathlib import Path
import subprocess
import sys
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


if __name__ == "__main__":
    unittest.main()
