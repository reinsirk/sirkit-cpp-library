# SPDX-License-Identifier: CC0-1.0
from pathlib import Path
import shlex
import shutil
import subprocess
import tempfile
import unittest


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "Verify/current/tests/z_algorithm_io_test.cpp"


class ZAlgorithmInputOutputTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.compiler = shutil.which("g++")
        if cls.compiler is None:
            raise unittest.SkipTest("g++ is required for the C++23 Z algorithm I/O regression test")

    def run_command(self, command, phase):
        diagnostic = f"{phase}: {shlex.join(map(str, command))}"
        try:
            result = subprocess.run(command, cwd=ROOT, capture_output=True, text=True, timeout=60)
        except subprocess.TimeoutExpired as error:
            self.fail(f"{diagnostic}\nTimed out after 60 seconds.\n"
                      f"stdout: {error.stdout!r}\nstderr: {error.stderr!r}")
        self.assertEqual(result.returncode, 0,
                         f"{diagnostic}\nExit status: {result.returncode}\n"
                         f"stdout:\n{result.stdout}\nstderr:\n{result.stderr}")

    def compile_and_run(self, fallback):
        with tempfile.TemporaryDirectory(prefix="sirkit-z-io-") as directory:
            binary = Path(directory) / "z_algorithm_io_test"
            command = [self.compiler, "-std=c++23", "-O2", "-Wall", "-Wextra"]
            mode = "SIRKIT_DISABLE_SIMD" if fallback else "default"
            if fallback:
                command.append("-DSIRKIT_DISABLE_SIMD")
            command.extend([str(SOURCE), "-o", str(binary)])
            self.run_command(command, f"Compile ({mode})")
            self.run_command([str(binary)], f"Run ({mode})")

    def test_default_io_against_standard_reference(self):
        self.compile_and_run(fallback=False)

    def test_fallback_io_against_standard_reference(self):
        self.compile_and_run(fallback=True)


if __name__ == "__main__":
    unittest.main()
