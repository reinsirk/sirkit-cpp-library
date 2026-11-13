#!/usr/bin/env python3
# SPDX-License-Identifier: CC0-1.0
"""生成時にガードや利用側の設定を壊さないことを確認する。"""
import importlib.util
import itertools
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

PATH = Path(__file__).resolve().parents[1] / "generate_variants.py"
SPEC = importlib.util.spec_from_file_location("variants", PATH)
MODULE = importlib.util.module_from_spec(SPEC)
sys.modules[SPEC.name] = MODULE
SPEC.loader.exec_module(MODULE)


class VariantsTest(unittest.TestCase):
    def test_known_branch(self):
        source = "#if defined(__clang__)\nCLANG\n#else\nGCC\n#endif\n"
        self.assertEqual(MODULE.specialize(source, "GCC"), "GCC\n")
        self.assertEqual(MODULE.specialize(source, "Clang"), "CLANG\n")

    def test_promote_elif(self):
        source = "#if defined(__clang__)\nC\n#elif OPTION\nA\n#else\nB\n#endif\n"
        self.assertEqual(MODULE.specialize(source, "GCC"), "#if OPTION\nA\n#else\nB\n#endif\n")

    def test_definitions_and_guards_preserved(self):
        source = "#ifndef GUARD // comment\n#define GUARD\n#if defined(__GNUC__)\nVALUE\n#endif\n#endif\n"
        self.assertEqual(MODULE.specialize(source, "GCC"), "#ifndef GUARD // comment\n#define GUARD\nVALUE\n#endif\n")

    def test_multiline_and_tail(self):
        source = "#if defined(__GNUC__) && " + "\\\n" + " (defined(__clang__) || OPTION)\nA\n#else\nB\n#endif\nTAIL"
        result = MODULE.specialize(source, "GCC")
        self.assertEqual(result, "#if OPTION\nA\n#else\nB\n#endif\nTAIL")

    def test_fast_does_not_define_global_skip(self):
        source = "#ifndef SKIP_RUNTIME_CHECKS\nCHECK\n#else\nUNCHECKED\n#endif\nCODE\n"
        self.assertEqual(MODULE.specialize(source, "GCC", True), "UNCHECKED\nCODE\n")
        self.assertEqual(MODULE.specialize(source, "Clang", False), source)

    def test_refuse_macro_mutation_and_bad_structure(self):
        for source in ["#undef __clang__\n", "#define __GNUC__ 12\n", "#endif\n", "#if A\n", "#if A\n#else\n#elif B\n#endif\n"]:
            with self.assertRaises(ValueError):
                MODULE.specialize(source, "GCC")

    def test_truth_table_against_real_preprocessor(self):
        conditions = [
            "defined(__GNUC__)", "!defined(__clang__)",
            "defined(__GNUC__) && defined(A)",
            "defined(__clang__) || (defined(A) && !defined(B))",
            "!(defined(__clang__) && defined(A)) && (defined(B) || defined(__GNUC__))",
            "!defined(SKIP_RUNTIME_CHECKS) && (defined(__GNUC__) || defined(__clang__))",
            "defined(__clang__) && (A + B > 0)",
            "defined(__GNUC__) && __has_include(<cstdint>)",
            "!defined(__clang__) == 2",
            "!defined(__GNUC__) != 3",
            "defined(__GNUC__) ? 0 || defined(A) : 1",
            "defined(__clang__) ? (defined(A) && !defined(B)) : 1",
        ]
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "active.cpp"
            for compiler, fast, a, b in itertools.product(("GCC", "Clang"), (False, True), (False, True), (False, True)):
                source = "".join(f"#if {condition}\nYES_{i}\n#elif defined(A)\nALT_{i}\n#else\nNO_{i}\n#endif\n" for i, condition in enumerate(conditions))
                profile = ["-D__clang__=1"] if compiler == "Clang" else []
                flags = ["-DA=1" if a else "-UA", "-DB=1" if b else "-UB"]
                # A/Bの裸の識別子も、未定義ならCプリプロセッサは0と解釈する。
                baseline_flags = profile + flags + (["-DSKIP_RUNTIME_CHECKS"] if fast else [])
                outputs = []
                for text, extra in [(source, baseline_flags), (MODULE.specialize(source, compiler, fast), profile + flags)]:
                    path.write_text(text)
                    result = subprocess.run(["g++", "-std=c++23", "-E", "-P", *extra, str(path)], capture_output=True, text=True)
                    self.assertEqual(result.returncode, 0, result.stderr)
                    outputs.append(result.stdout)
                self.assertEqual(*outputs, (compiler, fast, a, b))


if __name__ == "__main__":
    unittest.main()
