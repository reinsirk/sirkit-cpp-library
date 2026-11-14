# SPDX-License-Identifier: CC0-1.0
from pathlib import Path
import json
import shutil
import subprocess
import sys
import tempfile
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import public_namespace as ns


ROOT = Path(__file__).resolve().parents[2]


class NamespaceRoundtripTests(unittest.TestCase):
    def test_all_originals_roundtrip_and_idempotence(self):
        layout = json.loads((ROOT / "Scripts/library_layout.json").read_text())
        paths = sorted(p for directory in layout for p in (ROOT / directory).rglob("*.cpp"))
        self.assertEqual(len(paths), 103)
        for path in paths:
            with self.subTest(path=path.relative_to(ROOT)):
                raw = ns.unwrap_source(path.read_text())
                wrapped = ns.wrap_source(raw)
                self.assertEqual(ns.unwrap_source(wrapped), raw)
                self.assertEqual(ns.wrap_source(wrapped), wrapped)

    def test_comments_literals_raw_strings_and_digit_separators_are_not_rewritten(self):
        untouched = '''// reinsirk::detail and namespace reinsirk::detail { are documentation.
/* namespace reinsirk::detail { } */
const char* a = "reinsirk::detail::f";
const char* b = R"tag(
// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
/*SIRKIT_NS_REWRITE_END_V1*/
)tag";
constexpr auto value = 1'000'000ULL;
'''
        wrapped = ns.wrap_source(untouched)
        self.assertEqual(ns.unwrap_source(wrapped), untouched)
        self.assertIn(untouched.splitlines()[0], wrapped)
        self.assertIn('"reinsirk::detail::f"', wrapped)
        self.assertIn("1'000'000ULL", wrapped)

    def test_bare_detail_and_global_qualified_references_are_reversible(self):
        raw = '''namespace detail { inline int untouched() { return 1; } }
namespace reinsirk /* keep this */ :: detail { inline int f() { return 2; } }
inline int answer() { return ::reinsirk::detail::f() + detail::untouched(); }
'''
        wrapped = ns.wrap_source(raw)
        self.assertIn("namespace detail { inline int untouched()", wrapped)
        self.assertIn("/* keep this */", wrapped)
        self.assertIn("::sirkit::detail", wrapped)
        self.assertNotIn("::::sirkit", wrapped)
        self.assertEqual(ns.unwrap_source(wrapped), raw)

    def test_multiline_macros_are_logical_lines(self):
        raw = '#include <string>\n#define SIRKIT_TEXT \\\n "#include <vector>"\n#define SIRKIT_CALL(x) \\\n reinsirk::detail::f(x)\nnamespace reinsirk::detail { inline int f(int x) {return x;} }\n'
        wrapped = ns.wrap_source(raw)
        self.assertEqual(ns.unwrap_source(wrapped), raw)
        self.assertIn('"#include <vector>"', wrapped)
        self.assertEqual(wrapped.count(ns._CLOSE_LINE), 1)

    def test_using_directive_is_a_reference_not_a_namespace_definition(self):
        raw = 'namespace reinsirk::detail { inline int f() { return 1; } }\nusing namespace reinsirk::detail;\n'
        wrapped = ns.wrap_source(raw)
        self.assertIn('*/::sirkit::detail/*', wrapped)
        self.assertEqual(ns.unwrap_source(wrapped), raw)

    def test_empty_no_newline_crlf_and_include_only_sources_roundtrip(self):
        for raw in ["", "// license", "#include <vector>", "inline int f() {return 0;}",
                    "// license\r\n#include <vector>\r\ninline int f() {return 0;}\r\n"]:
            with self.subTest(raw=raw):
                self.assertEqual(ns.unwrap_source(ns.wrap_source(raw)), raw)

    def test_initial_headers_stay_before_the_first_namespace(self):
        raw = '// SPDX-License-Identifier: CC0-1.0\n// API list\n#include <vector>\n#if EXTRA\n#include <array>\n#endif\n#ifndef SIRKIT_A\n#define SIRKIT_A\nstruct A {};\n#endif\n'
        wrapped = ns.wrap_source(raw)
        self.assertTrue(wrapped.startswith('// SPDX-License-Identifier: CC0-1.0\n// API list\n'))
        self.assertLess(wrapped.index('#include <array>'), wrapped.index('namespace sirkit {'))
        self.assertLess(wrapped.index('namespace sirkit {'), wrapped.index('#ifndef SIRKIT_A'))

    def test_incomplete_or_edited_markers_fail_without_losing_user_changes(self):
        raw = 'namespace reinsirk::detail { inline int f() {return 0;} }\n'
        wrapped = ns.wrap_source(raw)
        cases = [wrapped.replace(ns._END, ""), wrapped.replace('namespace sirkit {', 'namespace changed {', 1),
                 wrapped.replace(ns._REWRITE_END, "", 1), wrapped.replace('*/detail/*', '*/changed/*', 1)]
        for changed in cases:
            with self.subTest(changed=changed):
                self.assertNotEqual(changed, wrapped)
                with self.assertRaises(ValueError):
                    ns.unwrap_source(changed)
                with self.assertRaises(ValueError):
                    ns.wrap_source(changed)


RAW_FIXTURE = '''// SPDX-License-Identifier: CC0-1.0
// API overview remains at the top.
#include <vector>
#ifndef SIRKIT_NAMESPACE_FIXTURE_HELPER
#define SIRKIT_NAMESPACE_FIXTURE_HELPER
namespace reinsirk::detail { inline int plus(int x) { return x + 1; } }
#endif
#if defined(SIRKIT_NAMESPACE_EXTRA_HEADER)
#include <numeric>
#else
#include <cstdint>
#endif
#define SIRKIT_NAMESPACE_CALL(x) \\
  reinsirk::detail::plus(x)
#define SIRKIT_NAMESPACE_TEXT \\
  "#include <not_a_real_header> reinsirk::detail"
#ifndef SIRKIT_NAMESPACE_FIXTURE_PUBLIC
#define SIRKIT_NAMESPACE_FIXTURE_PUBLIC
namespace detail { inline int bare() { return 2; } }
namespace geo { struct Point { int x; }; }
struct Value { int x; };
inline int evaluate(Value value) { return SIRKIT_NAMESPACE_CALL(value.x) + detail::bare(); }
#endif
'''


class NamespaceCompileTests(unittest.TestCase):
    def compile(self, compiler: str, *, late: bool, two_tus: bool):
        executable = shutil.which(compiler)
        if executable is None:
            self.skipTest(compiler + " is required")
        with tempfile.TemporaryDirectory(prefix="sirkit-namespace-") as temporary:
            root = Path(temporary)
            header = root / "wrapped.hpp"
            header.write_text(ns.wrap_source(RAW_FIXTURE))
            first, second = root / "first.cpp", root / "second.cpp"
            first.write_text('#include "wrapped.hpp"\n#include "wrapped.hpp"\n'
                             'int probe();\nint main() { sirkit::geo::Point p{3}; '
                             'return sirkit::evaluate(sirkit::Value{p.x}) != 6' + (' || probe() != 6' if two_tus else '') + '; }\n')
            second.write_text('#include "wrapped.hpp"\nint probe() { return sirkit::evaluate(sirkit::Value{3}); }\n')
            command = [executable, "-std=c++23", "-O2", "-Wall", "-Wextra"]
            if "clang" in compiler:
                gcc = shutil.which("g++")
                if gcc:
                    found = subprocess.run([gcc, "-print-libgcc-file-name"], capture_output=True, text=True, check=True)
                    libgcc = Path(found.stdout.strip())
                    if libgcc.is_absolute() and libgcc.is_file():
                        command += ["--gcc-install-dir=" + str(libgcc.parent)]
            if late:
                command += ["-DSIRKIT_NAMESPACE_EXTRA_HEADER"]
            command += [str(first)]
            if two_tus:
                command += [str(second)]
            command += ["-o", str(root / "program")]
            result = subprocess.run(command, capture_output=True, text=True, timeout=60)
            self.assertEqual(result.returncode, 0, result.stderr)
            result = subprocess.run([str(root / "program")], capture_output=True, text=True, timeout=10)
            self.assertEqual(result.returncode, 0, result.stderr)

    def test_gcc_conditional_includes_and_duplicate_inclusion(self):
        for late in (False, True):
            self.compile("g++", late=late, two_tus=False)

    def test_clang_conditional_includes_and_duplicate_inclusion(self):
        for late in (False, True):
            self.compile("clang++", late=late, two_tus=False)

    def test_two_translation_units_link_with_shared_guards(self):
        for compiler in ("g++", "clang++"):
            self.compile(compiler, late=True, two_tus=True)


if __name__ == "__main__":
    unittest.main()
