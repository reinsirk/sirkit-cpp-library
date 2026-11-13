# SPDX-License-Identifier: CC0-1.0
from contextlib import contextmanager
from pathlib import Path
import json
import shutil
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import prepare_verify as verify
import update_verify_status as status


class CustomInputOutputTests(unittest.TestCase):
    def setUp(self):
        self.directory = tempfile.TemporaryDirectory()
        self.addCleanup(self.directory.cleanup)
        self.root = Path(self.directory.name)
        self.write("Scripts/library_layout.json", '["DataStructure"]\n')
        self.write("DataStructure/example.cpp", "int library_value = 2;\n")
        self.write("Verify/current/common/fast_io.hpp", "int common_io = 3;\n")
        self.driver = ('#include <cstdio>\nint main() { int value=0; '
                       'if(std::scanf("%d",&value)!=1)return 0; '
                       'std::printf("%d\\n",value+library_value); }\n')
        self.write("Verify/current/drivers/example.cpp", self.driver)
        self.entry = {"id": "example", "title": "Example", "libraries": ["DataStructure/example.cpp"],
                      "driver": "Verify/current/drivers/example.cpp",
                      "problem_url": "https://judge.yosupo.jp/problem/example", "samples": []}
        self.group()

    def write(self, name, data):
        path = self.root / name
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(data)

    def group(self):
        self.write("Verify/current/groups/example.json", json.dumps({"entries": [self.entry]}))

    @contextmanager
    def repository(self):
        with patch.object(verify, "ROOT", self.root), patch.object(verify, "BASE", self.root / "Verify/current"):
            yield

    def render(self, **options):
        with self.repository():
            return verify.render(self.entry, **options)[0]

    def prepare(self):
        self.group()
        with self.repository():
            return verify.prepare(check=False)[0]

    def test_default_library_checker_still_requires_common_io(self):
        with self.assertRaisesRegex(ValueError, "共通高速入出力"):
            self.render()
        self.entry["allow_custom_io"] = False
        with self.assertRaisesRegex(ValueError, "共通高速入出力"):
            self.render()

    def test_explicit_library_checker_accepts_driver_owned_io(self):
        self.entry["allow_custom_io"] = True
        result = self.render()
        self.assertTrue(result.startswith(b"#define SKIP_RUNTIME_CHECKS\n"))
        self.assertIn(self.driver.encode(), result)
        self.assertNotIn(b"common_io", result)
        self.assertNotIn(b'#include "', result)
        checked = self.render(skip_runtime_checks=False)
        self.assertFalse(checked.startswith(b"#define SKIP_RUNTIME_CHECKS"))

    def test_custom_manifest_marks_full_driver_and_empty_common_io(self):
        self.entry["allow_custom_io"] = True
        entry = self.prepare()
        self.assertEqual(entry["io_mode"], "custom_online_io")
        self.assertEqual(entry["io_source_hashes"], {})
        self.assertEqual(entry["support_source_hashes"], {})
        self.assertEqual(entry["driver_sha256"], verify.digest(self.driver.encode()))
        self.assertEqual(entry["submission_sha256"], verify.digest(self.render()))

    def test_atcoder_rejects_the_library_checker_only_flag(self):
        self.entry["problem_url"] = "https://atcoder.jp/contests/example/tasks/example_a"
        for flag in (True, False):
            with self.subTest(flag=flag):
                self.entry["allow_custom_io"] = flag
                with self.assertRaisesRegex(ValueError, "Library Checker専用"):
                    self.render()
                self.group()
                with self.repository(), self.assertRaisesRegex(ValueError, "Library Checker専用"):
                    verify.collect()

    def test_local_entries_cannot_opt_in(self):
        self.entry.pop("problem_url")
        self.entry["allow_custom_io"] = True
        with self.assertRaisesRegex(ValueError, "Library Checker専用"):
            self.render()

    def test_unknown_and_lookalike_hosts_cannot_bypass_restriction(self):
        for host in ("unknown.example", "judge.yosupo.jp.example.com", "judge.yosupo.jp@unknown.example"):
            for flag in (False, True):
                with self.subTest(host=host, flag=flag):
                    self.entry.update(problem_url="https://" + host + "/problem/example", allow_custom_io=flag)
                    with self.assertRaisesRegex(ValueError, "Verify先"):
                        self.render()

    def test_non_boolean_flag_is_rejected(self):
        for flag in (None, 0, 1, "true", "false", [], {}):
            with self.subTest(flag=flag):
                self.entry["allow_custom_io"] = flag
                with self.assertRaisesRegex(ValueError, "bool"):
                    self.render()

    def test_quoted_unexpanded_includes_are_still_rejected(self):
        self.entry["allow_custom_io"] = True
        self.write(self.entry["driver"], '#include "some_dependency.hpp"\n' + self.driver)
        with self.assertRaisesRegex(ValueError, "展開されていない"):
            self.render()

    def test_duplicate_common_io_is_still_rejected_when_opted_in(self):
        self.entry["allow_custom_io"] = True
        self.write(self.entry["driver"], (verify.FAST_IO_INCLUDE + "\n") * 2 + self.driver)
        with self.assertRaisesRegex(ValueError, "重複"):
            self.render()

    def test_defaults_preserve_generated_bytes_and_shared_metadata(self):
        driver = verify.FAST_IO_INCLUDE + "\n" + self.driver
        self.write(self.entry["driver"], driver)
        result = self.render()
        expected = ('#define SKIP_RUNTIME_CHECKS\n// Verify: ' + self.entry["problem_url"] + '\n\n'
                    '// Library: DataStructure/example.cpp\nint library_value = 2;\n\n\n'
                    '// Verify I/O: Verify/current/common/fast_io.hpp\nint common_io = 3;\n\n'
                    + self.driver).encode()
        self.assertEqual(result, expected)
        entry = self.prepare()
        self.assertEqual(entry["io_mode"], "shared_fast_io")
        self.assertEqual(entry["io_source_hashes"], {verify.FAST_IO: verify.digest(b"int common_io = 3;\n")})
        for flag in (False, True):
            self.entry["allow_custom_io"] = flag
            self.assertEqual(self.render(), result)

    def test_custom_common_helpers_keep_dependency_tracking(self):
        self.entry["allow_custom_io"] = True
        self.write("Verify/current/common/parser.hpp", '#include "Verify/current/common/reader.hpp"\nint parser;\n')
        self.write("Verify/current/common/reader.hpp", "int reader;\n")
        self.write(self.entry["driver"], '#include "Verify/current/common/parser.hpp"\n' + self.driver)
        entry = self.prepare()
        self.assertEqual(entry["io_mode"], "custom_online_io")
        self.assertEqual(entry["io_source_hashes"], {})
        self.assertEqual(set(entry["support_source_hashes"]),
                         {"Verify/current/common/parser.hpp", "Verify/current/common/reader.hpp"})
        self.assertNotIn(b'#include "', self.render())

    def test_nested_common_fast_io_is_recorded_when_actually_included(self):
        self.entry["allow_custom_io"] = True
        self.write("Verify/current/common/parser.hpp", verify.FAST_IO_INCLUDE + "\n")
        self.write(self.entry["driver"], '#include "Verify/current/common/parser.hpp"\n' + self.driver)
        entry = self.prepare()
        self.assertEqual(entry["io_mode"], "custom_online_io")
        self.assertEqual(entry["io_source_hashes"], {verify.FAST_IO: verify.digest(b"int common_io = 3;\n")})

    def test_editing_custom_parser_requires_new_online_verification(self):
        self.entry["allow_custom_io"] = True
        original = self.prepare()
        self.write("Verify/current/judgements.json", json.dumps({"entries": [{
            "id": "example", "status": "AC", "url": "https://judge.yosupo.jp/submission/123",
            "submission_sha256": original["submission_sha256"]}]}))
        self.assertEqual(status.build_status(self.root)["entries"][0]["status"], "current_ac")
        new_driver = self.driver.replace('std::printf("%d\\n",', 'std::fprintf(stdout,"%d\\n",')
        self.write(self.entry["driver"], new_driver)
        updated = self.prepare()
        self.assertNotEqual(original["driver_sha256"], updated["driver_sha256"])
        self.assertNotEqual(original["submission_sha256"], updated["submission_sha256"])
        self.assertEqual(updated["io_source_hashes"], {})
        self.assertEqual(status.build_status(self.root)["entries"][0]["status"], "source_changed")

    @unittest.skipUnless(shutil.which("g++"), "g++ is required for the standalone custom IO sample")
    def test_standalone_custom_program_compiles_and_runs(self):
        self.entry["allow_custom_io"] = True
        for skip in (False, True):
            path = self.root / "program.cpp"
            binary = self.root / "program"
            path.write_bytes(self.render(skip_runtime_checks=skip))
            built = subprocess.run(["g++", "-std=c++23", "-O2", str(path), "-o", str(binary)],
                                   capture_output=True, text=True, timeout=60)
            self.assertEqual(built.returncode, 0, built.stderr)
            ran = subprocess.run([str(binary)], input="5\n", capture_output=True, text=True, timeout=10)
            self.assertEqual(ran.returncode, 0, ran.stderr)
            self.assertEqual(ran.stdout, "7\n")


if __name__ == "__main__":
    unittest.main()
