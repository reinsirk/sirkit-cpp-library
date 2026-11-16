# SPDX-License-Identifier: CC0-1.0
"""Verify mainが公式制約の±2e18級の答えを切り詰めないことを独立参照で確認する。"""
from pathlib import Path
import random
import shutil
import subprocess
import tempfile
import unittest

from Scripts import prepare_verify


ROOT = Path(__file__).resolve().parents[2]


def oracle_cases():
    cases = []
    # 2^60を超える正負の値、1差の比較、同一傾きと追加を含む。
    for sign in (1, -1):
        lines = [(sign * 10**9, sign * 10**18)]
        operations = [(1, 10**9), (1, 10**9 - 1),
                      (0, sign * (10**9 - 1), sign * (10**18 - 1)),
                      (1, 10**9), (1, -10**9), (1, 0)]
        cases.append((lines, operations))
    rng = random.Random(931709)
    slopes = [-10**9, -10**9 + 1, -1, 0, 1, 10**9 - 1, 10**9]
    offsets = [-10**18, -10**18 + 1, -(1 << 60) + 1, -1, 0,
               1, (1 << 60) - 1, 10**18 - 1, 10**18]
    # b自身は公式上限1e18以内に限定する。
    offsets = [v for v in offsets if abs(v) <= 10**18]
    for case in range(20):
        lines = [(rng.choice(slopes), rng.choice(offsets)) for _ in range(1 + case % 7)]
        operations = []
        for i in range(80):
            if i % 3 == 1:
                a = rng.choice(slopes) if i % 2 else rng.randint(-10**9, 10**9)
                b = rng.choice(offsets) if i % 2 else rng.randint(-10**18, 10**18)
                operations.append((0, a, b))
            else:
                x = rng.choice(slopes) if i % 2 else rng.randint(-10**9, 10**9)
                operations.append((1, x))
        cases.append((lines, operations))
    result = []
    for initial, operations in cases:
        current = initial.copy()
        answers = []
        text = [f"{len(initial)} {len(operations)}"]
        text.extend(f"{a} {b}" for a, b in initial)
        for op in operations:
            text.append(" ".join(map(str, op)))
            if op[0] == 0:
                current.append((op[1], op[2]))
            else:
                answers.append(str(min(a * op[1] + b for a, b in current)))
        result.append(("\n".join(text) + "\n", "\n".join(answers) + "\n"))
    return result


class LineContainer2DVerifyRuntimeTests(unittest.TestCase):
    def check_compiler(self, name, portable):
        compiler = shutil.which(name)
        if compiler is None:
            self.skipTest(f"{name} is required for the line-container Verify regression")
        entry = next(e for e in prepare_verify.collect() if e["id"] == "line_container_2d_min")
        common = [compiler, "-std=c++23", "-O2"]
        if "clang" in name:
            gcc = shutil.which("g++")
            if gcc:
                found = subprocess.run([gcc, "-print-libgcc-file-name"], check=True,
                                       capture_output=True, text=True, timeout=10)
                lib = Path(found.stdout.strip())
                if lib.is_absolute() and lib.is_file():
                    common.append("--gcc-install-dir=" + str(lib.parent))
        if portable:
            common += ["-U__SIZEOF_INT128__", "-DSIRKIT_FORCE_STANDARD_ARITHMETIC",
                       "-DSIRKIT_DISABLE_SIMD"]
        with tempfile.TemporaryDirectory(prefix="sirkit-lc2d-verify-") as work:
            source, binary = Path(work) / "test.cpp", Path(work) / "test"
            for skip in (False, True):
                data, _ = prepare_verify.render(entry, skip_runtime_checks=skip)
                source.write_bytes(data)
                built = subprocess.run(common + [str(source), "-o", str(binary)],
                                       capture_output=True, text=True, timeout=60)
                self.assertEqual(built.returncode, 0, built.stderr)
                for index, (input_text, expected) in enumerate(oracle_cases()):
                    with self.subTest(compiler=name, portable=portable, skip=skip, case=index):
                        ran = subprocess.run([str(binary)], input=input_text, capture_output=True,
                                             text=True, timeout=10)
                        self.assertEqual(ran.returncode, 0, ran.stderr)
                        self.assertEqual(ran.stdout, expected)

    def test_gcc(self):
        self.check_compiler("g++", False)

    def test_clang(self):
        self.check_compiler("clang++", False)

    def test_gcc_portable(self):
        self.check_compiler("g++", True)

    def test_clang_portable(self):
        self.check_compiler("clang++", True)


if __name__ == "__main__":
    unittest.main()
