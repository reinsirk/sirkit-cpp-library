#!/usr/bin/env python3
# SPDX-License-Identifier: CC0-1.0
"""checker更新時も実行し直し、同一条件のビルドだけを再利用する。"""
import argparse
import os
from pathlib import Path
import subprocess
import sys
import tempfile
import types
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import verify_variant_programs as verification


class CompilationCacheTest(unittest.TestCase):
    def test_recheck_and_invalidation(self):
        affinity = os.sched_getaffinity(0)
        with tempfile.TemporaryDirectory(prefix="sirkit_cache_test_") as directory:
            options = argparse.Namespace(output_dir=Path(directory), cpu=min(affinity), resume=True,
                                         compile_timeout=60, run_timeout=10)
            versions = {"GCC": subprocess.check_output(["/usr/bin/g++", "--version"], text=True)}
            compiler_calls = []

            def runner(checker):
                result = verification.Runner(options)
                result.versions = versions.copy()
                result.checker_sha = checker
                result.checker = types.SimpleNamespace(matches=lambda entry, sample, output:
                                                       output.split() == sample["output"].split())
                execute = result.execute

                def counted(command, timeout, folder, name, input_text=None):
                    if name == "compile":
                        compiler_calls.append(command)
                    return execute(command, timeout, folder, name, input_text)

                result.execute = counted
                return result

            source = b'#include <iostream>\nint main(){int a,b;std::cin>>a>>b;std::cout<<a+b<<"\\n";}\n'
            first = runner("checker-a")
            job = {"kind": "program", "id": "cache_test", "profile": "Variants/GCC", "setting": "default",
                   "compiler": "GCC", "source_sha256": verification.sha(source), "source": str(first.source(source)),
                   "entry": {}, "samples": [{"input": "2 3\n", "output": "5\n"}]}
            try:
                answer = first.validate(job)
                self.assertEqual(answer["status"], "LOCAL_SAMPLE_PASS")
                self.assertEqual(len(compiler_calls), 1)
                first.checker_sha = "checker-b"
                first.checker.matches = lambda entry, sample, output: False
                answer = first.validate(job)
                self.assertEqual(answer["status"], "SAMPLE_FAIL")
                self.assertTrue(answer["compile_reused"])
                self.assertEqual(len(compiler_calls), 1)
            finally:
                first.lock.close()
            second = runner("checker-c")
            try:
                answer = second.validate(job)
                self.assertEqual(answer["status"], "LOCAL_SAMPLE_PASS")
                self.assertTrue(answer["compile_reused"])
                self.assertEqual(len(compiler_calls), 1)
                Path(answer["binary_path"]).write_bytes(b"corrupted test binary")
                second.checker_sha = "checker-d"
                answer = second.validate(job)
                self.assertEqual(answer["status"], "LOCAL_SAMPLE_PASS")
                self.assertFalse(answer["compile_reused"])
                self.assertEqual(len(compiler_calls), 2)
                changed = source.replace(b"a+b<<", b"a+b+1<<")
                job.update(source_sha256=verification.sha(changed), source=str(second.source(changed)),
                           samples=[{"input": "2 3\n", "output": "6\n"}])
                self.assertEqual(second.validate(job)["status"], "LOCAL_SAMPLE_PASS")
                self.assertEqual(len(compiler_calls), 3)
                job["setting"] = "native"
                self.assertEqual(second.validate(job)["status"], "LOCAL_SAMPLE_PASS")
                self.assertEqual(len(compiler_calls), 4)
            finally:
                second.lock.close()
                os.sched_setaffinity(0, affinity)


if __name__ == "__main__":
    unittest.main()
