# SPDX-License-Identifier: CC0-1.0
from contextlib import contextmanager
import json
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import prepare_verify as verify


class CompileSettingsTests(unittest.TestCase):
    def setUp(self):
        directory = tempfile.TemporaryDirectory()
        self.addCleanup(directory.cleanup)
        self.root = Path(directory.name)
        self.entry = {"id": "example", "libraries": ["DataStructure/example.cpp"],
                      "driver": "Verify/current/drivers/example.cpp",
                      "problem_url": "https://judge.yosupo.jp/problem/example", "samples": []}
        self.library = "int library_value = 2;\n"
        self.driver = verify.FAST_IO_INCLUDE + '\n#include <cstdio>\nint main() { std::puts("ok"); }\n'
        self.write("Scripts/library_layout.json", '["DataStructure"]\n')
        self.write(self.entry["libraries"][0], self.library)
        self.write(self.entry["driver"], self.driver)
        self.write(verify.FAST_IO, "// shared IO fixture\n")
        self.group()

    def write(self, name, text):
        path = self.root / name
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(text)

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

    def test_default_source_and_metadata_preserve_previous_bytes(self):
        expected = ('#define SKIP_RUNTIME_CHECKS\n// Verify: ' + self.entry["problem_url"] + '\n\n'
                    '// Library: DataStructure/example.cpp\n' + self.library + '\n\n'
                    'namespace sirkit {}\nusing namespace sirkit;\n\n'
                    '// Verify I/O: ' + verify.FAST_IO + '\n// shared IO fixture\n'
                    '\n#include <cstdio>\nint main() { std::puts("ok"); }\n').encode()
        self.assertEqual(self.render(), expected)
        self.assertEqual(self.render(skip_runtime_checks=False), expected.removeprefix(b"#define SKIP_RUNTIME_CHECKS\n"))
        self.assertNotIn("compile_settings", self.prepare())

    def test_each_online_profile_is_guarded_and_precedes_includes(self):
        for host in ("judge.yosupo.jp", "atcoder.jp"):
            for profile in ("O2", "O3", "O3,unroll-loops"):
                with self.subTest(host=host, profile=profile):
                    self.entry.update(problem_url=f"https://{host}/problem/example", gcc_optimization=profile)
                    source = self.render().decode()
                    options = ", ".join(json.dumps(option) for option in profile.split(","))
                    block = ('#if defined(__GNUC__) && !defined(__clang__)\n'
                             f'#pragma GCC optimize({options})\n#endif\n')
                    self.assertTrue(source.startswith("#define SKIP_RUNTIME_CHECKS\n" + block))
                    self.assertLess(source.index(block), source.index("#include"))
                    checked = self.render(skip_runtime_checks=False).decode()
                    self.assertTrue(checked.startswith(block))
                    self.assertNotIn("#define NDEBUG", checked)

    def test_invalid_profiles_are_rejected_in_collect_and_render(self):
        for value in (None, 0, True, [], {}, "", "O0", "Ofast", "O3,fast-math",
                      "O3,-march=native", "O3 -funroll-loops", "O3\n#error injected"):
            with self.subTest(value=value):
                self.entry["gcc_optimization"] = value
                with self.assertRaisesRegex(ValueError, "gcc_optimization"):
                    self.render()
                self.group()
                with self.repository(), self.assertRaisesRegex(ValueError, "gcc_optimization"):
                    verify.collect()

    def test_profiles_require_an_approved_online_judge(self):
        self.entry["gcc_optimization"] = "O3"
        for url in (None, "", "https://unknown.example/problem/a",
                    "https://judge.yosupo.jp.example.com/problem/a"):
            with self.subTest(url=url):
                self.entry["problem_url"] = url
                with self.assertRaises(ValueError):
                    self.render()
                self.group()
                with self.repository(), self.assertRaises(ValueError):
                    verify.collect()

    def test_assert_scan_excludes_comments_strings_characters_and_static_assert(self):
        source = r'''// assert(false);
        /* assert(false); */
        const char* text = "assert(false);";
        const char* escaped = "\"assert(false);\"";
        const auto raw = u8R"tag(assert(false); " // text )tag";
        char character = '(';
        int value = 1'000'000;
        static_assert(true);
        int assertion = 0;
        '''
        self.assertFalse(verify.has_runtime_assert(source))
        self.assertTrue(verify.has_runtime_assert(source + "\nassert /* comment */ (value > 0);"))
        self.assertTrue(verify.has_runtime_assert("int n=1'000; assert(n);"))
        self.assertTrue(verify.has_runtime_assert("assert\\\n(true);"))
        self.write(self.entry["libraries"][0], source)
        self.assertNotIn(b"#define NDEBUG", self.render())

    def test_online_skipped_asserts_add_ndebug_before_all_includes(self):
        self.write(self.entry["libraries"][0], "#include <cassert>\nint probe() { assert(true); return 0; }\n")
        self.entry["gcc_optimization"] = "O3"
        source = self.render().decode()
        self.assertTrue(source.startswith("#define SKIP_RUNTIME_CHECKS\n#define NDEBUG\n"))
        self.assertLess(source.index("#define NDEBUG"), source.index("#include <cassert>"))
        self.assertNotIn(b"#define NDEBUG", self.render(skip_runtime_checks=False))
        self.assertEqual(self.prepare()["compile_settings"], {"gcc_optimization": "O3", "ndebug": True})

    def test_asserts_expanded_from_acl_and_common_headers_are_detected(self):
        for origin in ("acl", "common"):
            with self.subTest(origin=origin):
                body = "#include <cassert>\ninline void inspect() { assert(true); }\n"
                if origin == "acl":
                    self.write("Verify/current/vendor/atcoder/assert_test.hpp", body)
                    self.write(self.entry["libraries"][0], "#include <atcoder/assert_test>\n")
                else:
                    self.write(self.entry["libraries"][0], self.library)
                    self.write(verify.FAST_IO, body)
                self.assertIn(b"#define NDEBUG\n", self.render())
                self.assertNotIn(b"#define NDEBUG", self.render(skip_runtime_checks=False))
                self.assertEqual(self.prepare()["compile_settings"], {"ndebug": True})

    def test_local_asserts_are_not_automatically_disabled(self):
        self.entry.pop("problem_url")
        self.write(self.entry["libraries"][0], "#include <cassert>\nvoid inspect() { assert(true); }\n")
        for skip in (False, True):
            self.assertNotIn(b"#define NDEBUG", self.render(skip_runtime_checks=skip))
        self.assertNotIn("compile_settings", self.prepare())

    def test_manifest_records_only_actual_compile_settings(self):
        self.entry["gcc_optimization"] = "O3,unroll-loops"
        entry = self.prepare()
        self.assertEqual(entry["compile_settings"], {"gcc_optimization": "O3,unroll-loops"})
        self.assertEqual(entry["submission_sha256"], verify.digest(self.render()))
        self.entry.pop("gcc_optimization")
        self.entry["compile_settings"] = {"ndebug": True}
        self.assertNotIn("compile_settings", self.prepare())

    def compile_and_run(self, compiler, source, name):
        path, binary = self.root / (name + ".cpp"), self.root / name
        path.write_bytes(source)
        command = [compiler, "-std=c++23", "-O0", "-Wall", "-Wextra", "-Werror=unknown-pragmas"]
        if "clang" in Path(compiler).name and Path("/usr/lib/gcc/x86_64-linux-gnu/15").is_dir():
            command += ["--gcc-install-dir=/usr/lib/gcc/x86_64-linux-gnu/15"]
        command += [str(path), "-o", str(binary)]
        built = subprocess.run(command, capture_output=True, text=True, timeout=60)
        self.assertEqual(built.returncode, 0, f"{command}\n{built.stdout}\n{built.stderr}")
        run = subprocess.run([str(binary)], capture_output=True, text=True, timeout=60)
        self.assertEqual(run.returncode, 0, run.stderr)
        return run.stdout

    @unittest.skipUnless(shutil.which("g++"), "g++ is required to test GCC optimization pragmas")
    def test_gcc_pragma_actually_enables_optimization_from_o0(self):
        self.write(self.entry["libraries"][0],
                   "int optimization_probe() { int x = 17; return __builtin_constant_p(x); }\n")
        self.write(self.entry["driver"], verify.FAST_IO_INCLUDE +
                   '\n#include <cstdio>\nint main() { std::printf("%d\\n", optimization_probe()); }\n')
        self.assertEqual(self.compile_and_run("g++", self.render(), "gcc_default"), "0\n")
        for profile in ("O2", "O3", "O3,unroll-loops"):
            self.entry["gcc_optimization"] = profile
            self.assertEqual(self.compile_and_run("g++", self.render(), "gcc_optimized"), "1\n")

    @unittest.skipUnless(shutil.which("clang++"), "clang++ is required to check the GCC-only guard")
    def test_clang_ignores_all_gcc_profiles(self):
        self.write(self.entry["libraries"][0],
                   "int optimization_probe() { int x = 17; return __builtin_constant_p(x); }\n")
        self.write(self.entry["driver"], verify.FAST_IO_INCLUDE +
                   '\n#include <cstdio>\nint main() { std::printf("%d\\n", optimization_probe()); }\n')
        for profile in ("O2", "O3", "O3,unroll-loops"):
            self.entry["gcc_optimization"] = profile
            self.assertEqual(self.compile_and_run("clang++", self.render(), "clang_guarded"), "0\n")

    @unittest.skipUnless(shutil.which("g++"), "g++ is required to test assertion execution")
    def test_real_asserts_remain_active_only_in_checked_render(self):
        self.write(self.entry["libraries"][0],
                   "#include <cassert>\nint assertion_probe() { int calls=0; assert(++calls); return calls; }\n")
        self.write(self.entry["driver"], verify.FAST_IO_INCLUDE +
                   '\n#include <cstdio>\nint main() { std::printf("%d\\n", assertion_probe()); }\n')
        self.assertEqual(self.compile_and_run("g++", self.render(), "assert_skipped"), "0\n")
        self.assertEqual(self.compile_and_run("g++", self.render(skip_runtime_checks=False), "assert_checked"), "1\n")

    @unittest.skipUnless(shutil.which("g++"), "g++ is required to test the checked sample runner")
    def test_sample_runner_renders_checked_source_without_generated_ndebug(self):
        self.write(self.entry["libraries"][0], "#include <cassert>\nvoid inspect() { assert(true); }\n")
        self.entry.update(gcc_optimization="O3", samples=[{"input": "", "output": "ok\n"}])
        entry = self.prepare()
        output = self.root / "results.json"
        with self.repository():
            verify.run_tests([entry], "g++", False, output)
        results = json.loads(output.read_text())["results"]
        self.assertEqual(len(results), 2)
        for result in results:
            self.assertTrue(result["passed"])
            expected = self.render(skip_runtime_checks=result["skip_runtime_checks"])
            self.assertEqual(result["tested_source_sha256"], verify.digest(expected))
        self.assertNotEqual(results[0]["tested_source_sha256"], results[1]["tested_source_sha256"])


if __name__ == "__main__":
    unittest.main()
