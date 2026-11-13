# SPDX-License-Identifier: CC0-1.0
from pathlib import Path
import shlex
import shutil
import subprocess
import tempfile
import unittest


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "Verify/current/tests/dijkstra_bulk_test.cpp"


class DijkstraBulkRuntimeTests(unittest.TestCase):
    def run_command(self, command, phase, timeout):
        diagnostic = f"{phase}: {shlex.join(map(str, command))}"
        try:
            result = subprocess.run(command, cwd=ROOT, capture_output=True, text=True, timeout=timeout)
        except subprocess.TimeoutExpired as error:
            self.fail(f"{diagnostic}\nTimed out after {timeout} seconds.\n"
                      f"stdout: {error.stdout!r}\nstderr: {error.stderr!r}")
        self.assertEqual(result.returncode, 0,
                         f"{diagnostic}\nExit status: {result.returncode}\n"
                         f"stdout:\n{result.stdout}\nstderr:\n{result.stderr}")
        return result

    def compile_and_run(self, name, skip):
        compiler = shutil.which(name)
        if compiler is None:
            self.skipTest(f"{name} is required for the C++23 Dijkstra regression test")
        with tempfile.TemporaryDirectory(prefix="sirkit-dijkstra-") as directory:
            binary = Path(directory) / "dijkstra_bulk_test"
            command = [compiler, "-std=c++23", "-O2", "-Wall", "-Wextra"]
            if "clang" in name:
                gcc = shutil.which("g++")
                if gcc is not None:
                    found = self.run_command([gcc, "-print-libgcc-file-name"],
                                             "Find GCC installation for Clang", timeout=10)
                    library = Path(found.stdout.strip())
                    if library.is_absolute() and library.is_file() and library.name == "libgcc.a":
                        command.append("--gcc-install-dir=" + str(library.parent))
            # 追加ビルドなしで既定構築できない共通Edgeとの併用も確認する。
            if skip:
                command.extend(["-DSKIP_RUNTIME_CHECKS", "-DSIRKIT_DIJKSTRA_TEST_PREDEFINED_EDGE"])
            mode = f"{name} {'skip/predefined Edge' if skip else 'checked/default Edge'}"
            command.extend([str(SOURCE), "-o", str(binary)])
            self.run_command(command, f"Compile ({mode})", timeout=60)
            result = self.run_command([str(binary)], f"Run ({mode})", timeout=15)
            self.assertIn("PASS Dijkstra bulk/WGraph", result.stdout)
            self.assertRegex(result.stdout, r"cases=[1-9]\d* checks=[1-9]\d*")

    def test_gcc_checked(self):
        self.compile_and_run("g++", skip=False)

    def test_gcc_skip_runtime_checks(self):
        self.compile_and_run("g++", skip=True)

    def test_clang_checked(self):
        self.compile_and_run("clang++", skip=False)

    def test_clang_skip_runtime_checks(self):
        self.compile_and_run("clang++", skip=True)


if __name__ == "__main__":
    unittest.main()
