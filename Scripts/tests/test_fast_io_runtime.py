# SPDX-License-Identifier: CC0-1.0
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest


ROOT = Path(__file__).resolve().parents[2]


class FastInputOutputRuntimeTests(unittest.TestCase):
    def run_reference(self, skip):
        compiler = shutil.which("g++")
        if compiler is None:
            self.skipTest("g++ is required for the fast I/O boundary test")
        with tempfile.TemporaryDirectory(prefix="sirkit-fast-io-") as directory:
            executable = Path(directory) / "reference"
            command = [compiler, "-std=c++23", "-O2", "-Wall", "-Wextra"]
            if skip:
                command.append("-DSKIP_RUNTIME_CHECKS")
            command.extend([str(ROOT / "Verify/current/tests/fast_io_test.cpp"), "-o", str(executable)])
            built = subprocess.run(command, capture_output=True, text=True, timeout=60)
            self.assertEqual(built.returncode, 0, built.stdout + built.stderr)
            run = subprocess.run([str(executable)], capture_output=True, text=True, timeout=60)
            self.assertEqual(run.returncode, 0, run.stdout + run.stderr)
            self.assertIn("checks", run.stdout)

    def test_checked_input_output_boundaries(self):
        self.run_reference(skip=False)

    def test_unchecked_input_output_boundaries(self):
        self.run_reference(skip=True)


if __name__ == "__main__":
    unittest.main()
