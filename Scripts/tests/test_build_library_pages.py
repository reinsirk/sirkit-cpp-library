# SPDX-License-Identifier: CC0-1.0
import hashlib
import io
import json
from pathlib import Path
from html.parser import HTMLParser
import subprocess
import tempfile
import unittest
from unittest.mock import patch

from Scripts import build_library_pages as pages


class LibraryPagesTest(unittest.TestCase):
    @staticmethod
    def variants_manifest(source):
        return {"format": 1, "generated_by": "Scripts/generate_variants.py", "entries": [
            {"source": source, "origin": source, "compiler": compiler, "fast": fast,
             "path": f"{'Fast' if fast else 'Variants'}/{compiler}/{source}",
             "sha256": "a" * 64, "source_sha256": "b" * 64}
            for compiler in ("Portable", "GCC", "Clang", "MSVC") for fast in (False, True)
        ]}

    def test_home_offers_versions_and_each_list_uses_only_its_selected_download(self):
        source = "Math/prime_judge(Miller-Rabin).cpp"
        groups = pages.variant_entries(json.dumps(self.variants_manifest(source)).encode(), [source])
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root / "_includes").mkdir()
            downloads, links = pages.write_version_lists(root, groups, {source: "素数判定"}, {source: "現行ACあり"})
            picker = (root / "_includes/code-version-picker.html").read_text(encoding="utf-8")
            self.assertEqual(picker.count('class="code-version-link"'), 8)
            self.assertNotIn("prime_judge", picker)
            self.assertIn("#define SKIP_RUNTIME_CHECKS", picker)
            self.assertIn("通常版", picker)
            self.assertIn("競技プログラミングの提出向け", picker)
            self.assertEqual(len(downloads), 8)
            for compiler in ("Portable", "GCC", "Clang", "MSVC"):
                for fast, directory in ((False, "Variants"), (True, "Fast")):
                    key = ("fast-" if fast else "safe-") + compiler.lower()
                    self.assertIn(f"/versions/{key}.html", picker)
                    listing = (root / f"versions/{key}.md").read_text(encoding="utf-8")
                    self.assertIn("<h2>数学</h2>", listing)
                    self.assertIn("素数判定", listing)
                    self.assertIn('aria-label="現行ACあり" title="現行ACあり">✓</span>', listing)
                    asset = f"/assets/code/{directory}/{compiler}/Math/prime_judge%28Miller-Rabin%29.cpp"
                    self.assertIn(asset, listing)
                    self.assertIn(f"#code-version-{key}", listing)
                    self.assertEqual(listing.count('class="code-variant-download"'), 1)
                    self.assertEqual(links[f"versions/{key}"], {f"{source}.html#code-version-{key}"})
                    if fast:
                        self.assertIn("/fast-guide.html", listing)
                        self.assertIn("一部の実装ではメモリの解放も省きます", listing)
                        self.assertIn("通常版との速度差はライブラリ、入力、処理系によって変わります", listing)
                    else:
                        self.assertNotIn("/fast-guide.html", listing)
                    self.assertIn("/licenses/index.html", listing)
        context = pages.version_context()
        self.assertIn('id="code-version-fast-msvc">速度優先版・MSVC専用', context)
        self.assertIn("/versions/fast-msvc.html", context)
        self.assertNotIn("code-variant-download", context)

    def test_verification_label_requires_current_online_or_local_evidence(self):
        online = {"status": "current_ac"}
        local = {"status": "PASS", "tested_source_sha256": ["a" * 64, "b" * 64]}
        def quality(status, evidence):
            return {"categories": {"normal": {"status": status, "evidence": evidence}}}
        self.assertEqual(pages.library_verification_label([online], quality("PASS", [local])), "現行ACあり")
        self.assertEqual(pages.library_verification_label([], quality("PASS", [local])), "通常版のローカルVerify通過")
        self.assertEqual(pages.library_verification_label(
            [{**online, "submission_policy": {"kind": "reference"}}], None), "")
        self.assertEqual(pages.library_verification_label([], quality("PASS", [{"status": "PASS"}])), "")
        for status in ("UNAVAILABLE", "STALE", "PARTIAL", "FAIL"):
            self.assertEqual(pages.library_verification_label([], quality(status, [local])), "")

    def test_manifest_rejects_missing_duplicate_and_outside_downloads(self):
        source = "Graph/example.cpp"
        for change, hint in ((lambda rows: rows.pop(), "全4構成"),
                             (lambda rows: rows.append(rows[0]), "重複"),
                             (lambda rows: rows[0].update(path="../example.cpp"), "パス"),
                             (lambda rows: rows[0].update(origin="../example.cpp"), "パス")):
            with self.subTest(hint=hint):
                manifest = self.variants_manifest(source)
                change(manifest["entries"])
                with self.assertRaisesRegex(ValueError, hint):
                    pages.variant_entries(json.dumps(manifest).encode(), [source])

    def test_rendered_download_links_and_bytes_are_checked(self):
        source = "Math/test(api).cpp"
        asset = "assets/code/Fast/MSVC/" + source
        payload = b"int f() { return 42; }\n"
        expected = {source: {asset: hashlib.sha256(payload).hexdigest()}}
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root / "index.html").touch()
            (root / "quality.html").touch()
            (root / asset).parent.mkdir(parents=True)
            (root / asset).write_bytes(payload)
            page = root / (source + ".html")
            page.parent.mkdir()
            link = '<a class="code-variant-download" href="/prefix/assets/code/Fast/MSVC/Math/test%28api%29.cpp" download>取得</a>'
            page.write_text(link, encoding="utf-8")
            result = pages.validate_site(root, {source}, "/prefix", expected_downloads=expected)
            self.assertEqual(result["downloads_checked"], 1)
            self.assertTrue(result["passed"])
            page.write_text(link.replace("%28api%29", "(api)"), encoding="utf-8")
            self.assertTrue(pages.validate_site(root, {source}, "/prefix", expected_downloads=expected)["passed"])
            page.write_text(link.replace(" download>", ">"), encoding="utf-8")
            self.assertEqual(pages.validate_site(root, {source}, "/prefix", expected_downloads=expected)
                             ["download_link_mismatches"], [source])
            page.write_text(link, encoding="utf-8")
            (root / asset).write_bytes(payload + b"// changed")
            self.assertEqual(pages.validate_site(root, {source}, "/prefix", expected_downloads=expected)
                             ["download_mismatches"], [asset])

    def test_version_navigation_requires_rendered_links_and_detail_fragment(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root / "quality.html").touch()
            (root / "versions").mkdir()
            (root / "Graph").mkdir()
            selector = root / "index.html"
            listing = root / "versions/fast-gcc.html"
            detail = root / "Graph/example.cpp.html"
            targets = {selector: "versions/fast-gcc.html", listing: "Graph/example.cpp.html#code-version-fast-gcc"}
            for file, target in targets.items():
                file.write_text(f'<a class="code-version-link" href="/preview/{target}">開く</a>', encoding="utf-8")
            expected = {"index": {targets[selector]}, "versions/fast-gcc": {targets[listing]}}
            html_pages = {"versions/fast-gcc", "Graph/example.cpp"}
            detail.write_text('<p id="code-version-fast-gcc">選んだ版</p>', encoding="utf-8")
            self.assertTrue(pages.validate_site(root, html_pages, "/preview", expected_version_links=expected)["passed"])
            original = selector.read_text(encoding="utf-8")
            selector.write_text(original + '<a href="/preview/Graph/example.cpp.html">API</a>', encoding="utf-8")
            result = pages.validate_site(root, html_pages, "/preview", {"Graph/example.cpp": ""}, expected_version_links=expected)
            self.assertEqual(result["version_link_mismatches"], ["index"])
            selector.write_text(original, encoding="utf-8")
            detail.write_text('<p>選んだ版</p>', encoding="utf-8")
            result = pages.validate_site(root, html_pages, "/preview", expected_version_links=expected)
            self.assertEqual(result["missing_version_anchors"], [targets[listing]])
            selector.write_text("<p>選択リンクなし</p>", encoding="utf-8")
            result = pages.validate_site(root, html_pages, "/preview", expected_version_links=expected)
            self.assertEqual(result["version_link_mismatches"], ["index"])

    def test_reference_operators_do_not_become_tables_or_typographic_quotes(self):
        text = ('## operator<<\n\n```text\nvoid operator<<(T x);\n```\n\n'
                'bits |= bits << shiftを適用する。`a | b`はコード。\n'
                '```cpp\na |= b;\n```\n')
        actual = pages.protect_reference_literals(text)
        self.assertIn('## `operator<<`', actual)
        self.assertIn('bits \\|= bits << shift', actual)
        self.assertIn('`a | b`', actual)
        self.assertIn('```cpp\na |= b;\n```', actual)
        self.assertIn('```text\nvoid operator<<(T x);\n```', actual)
        self.assertEqual(pages.protect_reference_literals(actual), actual)

    def test_source_api_stops_before_implementation_and_keeps_exact_types(self):
        text = ('// SPDX-License-Identifier: CC0-1.0\n// API一覧\n'
                '// [Box<T>]\n// Box<T>{x}: O(1)。\n\n'
                '// SIRKIT_NAMESPACE_V1: internal marker\n#include <vector>\n'
                '// implementation details\nvoid hidden();\n')
        api = pages.source_api_text(text)
        self.assertEqual(api, '[Box<T>]\nBox<T>{x}: O(1)。')
        self.assertNotIn('hidden', api)
        self.assertNotIn('SIRKIT_NAMESPACE', api)
        with self.assertRaisesRegex(ValueError, 'API一覧'):
            pages.source_api_text('#include <vector>\n// API一覧\n// too late\n')

    def test_api_reference_preserves_cpp_and_does_not_evaluate_liquid(self):
        text = 'Foo<T>{x}: {% include private %}<script>example</script>'
        rendered = pages.api_reference(text)
        self.assertNotIn('{% include', rendered)
        self.assertNotIn('<script>', rendered)
        class Content(HTMLParser):
            def __init__(self): super().__init__(); self.inside = False; self.text = ''
            def handle_starttag(self, tag, attributes):
                if tag == 'pre': self.inside = True
            def handle_endtag(self, tag):
                if tag == 'pre': self.inside = False
            def handle_data(self, value):
                if self.inside: self.text += value
        parsed = Content()
        parsed.feed(rendered)
        self.assertEqual(parsed.text, text)

    def test_scoped_document_uses_only_selected_guide_and_rejects_escape(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory) / 'repo'
            (root / 'Document').mkdir(parents=True)
            (root / 'Document/circle.md').write_text('# 円')
            outside = Path(directory) / 'outside.md'
            outside.write_text('private')
            self.assertEqual(pages.scoped_document_path(root, 'Circle.cpp',
                             {'Circle.cpp': 'Document/circle.md'}), 'Document/circle.md')
            for target in ('Document/../../outside.md', 'Document/missing.md'):
                with self.assertRaises(ValueError):
                    pages.scoped_document_path(root, 'Circle.cpp', {'Circle.cpp': target})
            (root / 'Document/link.md').symlink_to(outside)
            with self.assertRaises(ValueError):
                pages.scoped_document_path(root, 'Circle.cpp', {'Circle.cpp': 'Document/link.md'})

    def test_all_library_pages_have_source_api_and_valid_guides(self):
        overrides = json.loads((pages.ROOT / '.competitive-verifier/page-documents.json').read_text())
        for source in pages.source_paths(pages.ROOT):
            with self.subTest(source=source):
                self.assertTrue(pages.source_api_text((pages.ROOT / source).read_text()))
                self.assertIsNotNone(pages.scoped_document_path(pages.ROOT, source, overrides))

    def test_licenses_are_fetched_from_pinned_commit_and_hash_checked(self):
        files = {"license.txt": b"upstream license", "notices.txt": b"third party notices"}
        entries = tuple((name, "upstream/" + name, hashlib.sha256(data).hexdigest())
                        for name, data in files.items())
        with patch.object(pages, "VERIFIER_LICENSES", entries), \
             patch.object(pages, "urlopen", side_effect=[io.BytesIO(data) for data in files.values()]) as request:
            self.assertEqual(pages.fetch_upstream_licenses(), files)
        for call in request.call_args_list:
            self.assertIn("/" + pages.VERIFIER_REVISION + "/upstream/", call.args[0])
            self.assertEqual(call.kwargs["timeout"], 30)

    def test_changed_upstream_license_is_rejected(self):
        with patch.object(pages, "urlopen", return_value=io.BytesIO(b"changed upstream")):
            with self.assertRaisesRegex(ValueError, "SHA-256"):
                pages.fetch_upstream_licenses()

    def test_failed_license_fetch_preserves_existing_preview(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory) / "repo"
            root.mkdir()
            output = Path(directory) / "preview"
            output.mkdir()
            (output / "index.html").write_text("previous preview")
            with patch.object(pages.importlib.metadata, "version", return_value=pages.VERIFIER_VERSION), \
                 patch.object(pages.subprocess, "check_output", side_effect=["a" * 40, b""]), \
                 patch.object(pages, "fetch_upstream_licenses", side_effect=OSError("connection failed")):
                with self.assertRaises(OSError):
                    pages.build(root, output, [])
            self.assertEqual((output / "index.html").read_text(), "previous preview")

    def test_upstream_licenses_are_not_stored_in_repository(self):
        for name, _, digest in pages.VERIFIER_LICENSES:
            self.assertFalse((pages.ROOT / "LICENSES" / name).exists())
            self.assertRegex(digest, r"^[0-9a-f]{64}$")
        self.assertRegex(pages.VERIFIER_REVISION, r"^[0-9a-f]{40}$")
        self.assertEqual((pages.ROOT / ".competitive-verifier/requirements.txt").read_text().strip(),
                         "competitive-verifier==" + pages.VERIFIER_VERSION)

    def test_dirty_production_is_rejected_before_overwriting_output(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            output = root.parent / (root.name + "-pages")
            with patch.object(pages.importlib.metadata, "version", return_value="4.2.2"), \
                 patch.object(pages.subprocess, "check_output", side_effect=["a" * 40, b" M Graph/test.cpp"]):
                with self.assertRaisesRegex(ValueError, "未コミット"):
                    pages.build(root, output, [], production=True)
            self.assertFalse(output.exists())

    def test_output_preserves_existing_data(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory) / "repo"
            root.mkdir()
            output = Path(directory) / "data"
            output.mkdir()
            (output / "user.txt").write_text("keep")
            with self.assertRaises(ValueError):
                pages.prepare_output(root, output)
            self.assertEqual((output / "user.txt").read_text(), "keep")
            with self.assertRaises(ValueError):
                pages.prepare_output(root, root)
            with self.assertRaises(ValueError):
                pages.prepare_output(root, root / "Graph")

    def test_performance_shows_absolute_times_and_workload_before_raw_data(self):
        text = pages.quality_table({"categories": {"performance": {"status": "PASS", "evidence": [{
            "status": "PASS", "scope": "Verify全体", "pattern": "chain", "size": 100,
            "size_unit": "input_bytes", "median_ns": {"current": 1234000},
            "cpu_median_ns": {"current": 1000000}, "workload": {"dimensions": {"n": 10}},
            "compiler": {"version": "GCC test"}, "flags": ["-O2"], "cpu_model": "unit CPU",
            "repetitions": 3, "warmups": 1}]}}})
        self.assertIn("wall ms", text)
        self.assertIn("1.234", text)
        self.assertIn("1.000", text)
        self.assertIn("GCC test", text)
        self.assertLess(text.index("wall ms"), text.index("測定の生データ"))
        self.assertNotIn("現在/基準", text)

    def test_missing_quality_is_not_pass(self):
        text = pages.quality_table(None)
        self.assertIn("| 失敗系 | 未測定 |", text)
        self.assertIn("| Library Checker公式ケース | 未測定 |", text)
        self.assertNotIn("| 合格 |", text)

    def test_library_checker_without_a_registered_problem(self):
        text = pages.quality_table({"categories": {"library_checker": {"status": "NOT_APPLICABLE"}}})
        self.assertIn("| Library Checker公式ケース | 無し |", text)

    def test_coverage_requires_hundred_and_shows_actual_counts(self):
        text = pages.quality_table({"categories": {"coverage": {"status": "FAIL", "evidence": [{
            "report": "one", "metrics": {"lines": {"count": 2, "covered": 1},
            "branches": {"count": 2, "covered": 1}}}]}}})
        self.assertIn("行・直接分岐とも100%", text)
        self.assertIn("行 1/2（50.00%）", text)
        self.assertIn("分岐 1/2（50.00%）", text)
        self.assertIn("失敗", text)

    def test_budget_status_and_limit_are_visible_beside_measurement(self):
        text = pages.quality_table({"categories": {"performance": {"status": "FAIL", "evidence": [{
            "status": "FAIL", "scope": "Verify全体", "pattern": "chain", "size": 100,
            "median_ns": {"current": 20000000}, "cpu_median_ns": {"current": 11000000},
            "performance_limit": {"status": "FAIL", "metric": "child_cpu_ns", "limit_ns": 10000000}}]}}})
        self.assertIn("上限 ms", text)
        self.assertIn("20.000 | 11.000 | — | — | 10.000 | 失敗（固定上限）", text)

    def test_performance_regression_displays_the_comparison_metric_and_retry(self):
        compared = {
            "status": "FAIL", "scope": "Verify全体", "pattern": "chain", "size": 100,
            "median_ns": {"current": 11_000_000, "base": 8_000_000},
            "cpu_median_ns": {"current": 6_000_000, "base": 5_000_000},
            "comparison": "MEASURED", "base_revision": "e" * 40,
            "performance_regression": {"status": "FAIL", "metric": "child_cpu_ns",
                                       "baseline_median_ns": 5_000_000, "limit_ns": 5_250_000,
                                       "observed_median_ns": 6_000_000},
            "regression_confirmation": {"base": [5_100_000], "current": [6_000_000]},
        }
        unavailable = {"status": "PASS", "scope": "Verify全体", "pattern": "new", "size": 5,
                       "median_ns": {"current": 3_000_000}, "comparison": "BASELINE_MISSING"}
        text = pages.performance_table([compared, unavailable])
        self.assertIn("比較元 ms", text)
        self.assertIn("11.000 | 6.000 | 5.000 | 1.200× | 5.250 | 退行（CPU・再測定）", text)
        self.assertIn("3.000 | 対象外 | — | — | — | 比較対象外（比較元なし）", text)
        self.assertIn("e" * 40, text)

    def test_performance_ratio_is_unavailable_when_old_cpu_median_is_zero(self):
        text = pages.performance_table([{
            "status": "PASS", "scope": "Verify全体", "pattern": "empty", "size": 3,
            "median_ns": {"current": 3_000_000, "base": 2_000_000},
            "cpu_median_ns": {"current": 80_000, "base": 0},
            "performance_regression": {"status": "PASS", "metric": "child_cpu_ns",
                                       "baseline_median_ns": 0, "limit_ns": 100_000,
                                       "observed_median_ns": 80_000},
        }])
        self.assertIn("3.000 | 0.080 | 0.000 | — | 0.100 | 通過（CPU）", text)

    def test_old_ac_is_labelled_stale(self):
        text = pages.online_table([dict(title="test", status="source_changed", problem_url=None,
                                        judgement=dict(status="AC", url="https://judge.yosupo.jp/submission/1"))])
        self.assertIn("現行未照合・要再提出", text)
        self.assertIn("[AC]", text)
        self.assertNotIn("| 現行AC |", text)
        self.assertNotIn("| ✓ 現行AC |", text)

    def test_current_ac_links_to_program_problem_and_submission(self):
        source = "Verify/current/submissions/union_find.cpp"
        entry = dict(title="Unionfind", submission=source, status="current_ac",
                     problem_url="https://judge.yosupo.jp/problem/unionfind",
                     judgement=dict(status="AC", url="https://judge.yosupo.jp/submission/1"))
        text = pages.online_table([entry], {source})
        self.assertIn(r"[union\_find.cpp]", text)
        self.assertIn(source + ".html", text)
        self.assertIn("[Unionfind](https://judge.yosupo.jp/problem/unionfind)", text)
        self.assertIn("[AC](https://judge.yosupo.jp/submission/1)", text)
        self.assertIn("| ✓ 現行AC |", text)
        self.assertNotIn(source + ".html", pages.online_table([entry], set()))

    def test_submission_policy_labels_do_not_promote_old_ac(self):
        for kind, label in [("documentation_only", "説明変更のみ・再提出不要"),
                            ("reference", "参考検証・移行先あり")]:
            entry = dict(title="test", status="source_changed", problem_url=None,
                         judgement=dict(status="AC", url="https://judge.yosupo.jp/submission/1"),
                         submission_policy=dict(kind=kind, reason="<script>do not render</script>"))
            before = json.dumps(entry, sort_keys=True)
            text = pages.online_table([entry])
            self.assertIn(label, text)
            self.assertNotIn("現行未照合・要再提出", text)
            self.assertIn("[AC]", text)
            self.assertNotIn("| 現行AC |", text)
            self.assertNotIn("<script>", text)
            self.assertEqual(json.dumps(entry, sort_keys=True), before)

    def test_unknown_submission_policy_keeps_default_status_label(self):
        text = pages.online_table([dict(title="test", status="source_changed", problem_url=None,
                                       submission_policy=dict(kind="unknown"))])
        self.assertIn("現行未照合・要再提出", text)

    def test_json_evidence_is_not_liquid_or_html(self):
        text = pages.quality_table({"categories": {"failure": {"status": "PASS", "evidence": [
            {"compiler": "{% include bad %}<script>x</script>"}]}}})
        self.assertNotIn("{% include bad %}", text)
        self.assertNotIn("<script>", text)
        self.assertIn("&lt;script&gt;", text)

    def test_rewrite_links_preserves_cpp_examples(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root / "Document").mkdir()
            (root / "Math").mkdir()
            path = "Math/prime_judge(Miller-Rabin).cpp"
            text = "[prime](../Math/prime_judge(Miller-Rabin).cpp)\n```cpp\n[i](x)\n```\n`[i](x)`\n"
            rendered = pages.rewrite_links(text, "Document/prime.md", root, {path}, "main")
            self.assertIn("prime_judge%28Miller-Rabin%29.cpp.html", rendered)
            self.assertIn("```cpp\n[i](x)\n```", rendered)
            self.assertIn("`[i](x)`", rendered)

    def test_symlink_library_is_rejected(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root / "Graph").mkdir()
            (root / "other.cpp").write_text("code")
            (root / "Graph/test.cpp").symlink_to(root / "other.cpp")
            with self.assertRaises(ValueError):
                pages.source_paths(root)

    def test_document_cpp_initializers_are_literal_liquid(self):
        example = "```cpp\nGraph tree{{1}, {0,2}, {1}};\n```\n{% include private %}"
        escaped = pages.literal_liquid(example)
        self.assertIn("Graph tree{{ '{{' }}1}, {0,2}, {1}};", escaped)
        self.assertIn("{{ '{%' }} include private %}", escaped)
        self.assertNotIn("{% include private %}", escaped)

    def test_explicit_links_do_not_change_source_identity(self):
        data = {"path": "Graph/test.cpp", "dependencies": [
            {"files": [{"path": "Graph/test.cpp", "icon": "LIBRARY_NO_TESTS"}]}]}
        pages.explicit_page_links(data, {"Graph/test.cpp"})
        self.assertEqual(data["path"], "Graph/test.cpp")
        self.assertEqual(data["dependencies"][0]["files"][0]["path"], "Graph/test.cpp.html")

    def test_html_validation_checks_baseurl_and_real_destinations(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root / "Graph").mkdir()
            (root / "Graph/test.cpp.html").write_text('<a href="/lib/quality.html">result</a>')
            (root / "quality.html").write_text('<a href="/lib/">home</a>')
            (root / "index.html").write_text('<a href="/lib/Graph/test.cpp.html">test</a>')
            result = pages.validate_site(root, {"Graph/test.cpp"}, "/lib")
            self.assertTrue(result["passed"])
            (root / "index.html").write_text('<a href="/lib/Graph/test.cpp">broken</a>')
            result = pages.validate_site(root, {"Graph/test.cpp"}, "/lib")
            self.assertFalse(result["passed"])
            self.assertEqual(len(result["broken_local_links"]), 1)

    def test_html_validation_detects_missing_or_changed_api_reference(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root / 'Graph').mkdir()
            (root / 'index.html').write_text('home')
            (root / 'quality.html').write_text('quality')
            page = root / 'Graph/test.cpp.html'
            page.write_text('<pre class="api-reference-text">Box&lt;T&gt;{x}</pre>')
            expected = {'Graph/test.cpp': 'Box<T>{x}'}
            self.assertTrue(pages.validate_site(root, set(expected), '', expected)['passed'])
            page.write_text('<pre class="api-reference-text">Other()</pre>')
            result = pages.validate_site(root, set(expected), '', expected)
            self.assertFalse(result['passed'])
            self.assertEqual(result['api_mismatches'], ['Graph/test.cpp'])
            page.write_text('missing API list')
            self.assertFalse(pages.validate_site(root, set(expected), '', expected)['passed'])

    def test_external_document_symlink_is_not_embedded(self):
        with tempfile.TemporaryDirectory() as directory:
            base = Path(directory)
            root = base / "repo"
            (root / "Graph").mkdir(parents=True)
            outside = base / "outside"
            outside.mkdir()
            (outside / "secret.md").write_text("private")
            (root / "Document").symlink_to(outside, target_is_directory=True)
            (root / "Graph/test.cpp").write_text("// Document/secret.md")
            self.assertIsNone(pages.document_path(root, "Graph/test.cpp"))

    def test_isolated_index_does_not_touch_index_or_objects(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            subprocess.run(["git", "init", "-q", "-b", "dev"], cwd=root, check=True)
            (root / "old.cpp").write_text("old")
            subprocess.run(["git", "add", "old.cpp"], cwd=root, check=True)
            before = (root / ".git/index").read_bytes()
            (root / "new.cpp").write_text("unique new blob for this test")
            blob = subprocess.check_output(["git", "hash-object", "new.cpp"], cwd=root, text=True).strip()
            with pages.isolated_index(root, ["new.cpp"]) as environment:
                actual = subprocess.check_output(["git", "ls-files"], cwd=root, env=environment, text=True)
                self.assertEqual(actual.strip(), "new.cpp")
                subprocess.run(["git", "cat-file", "-e", blob], cwd=root, env=environment, check=True)
            self.assertEqual((root / ".git/index").read_bytes(), before)
            absent = subprocess.run(["git", "cat-file", "-e", blob], cwd=root, capture_output=True)
            self.assertNotEqual(absent.returncode, 0)


if __name__ == "__main__":
    unittest.main()
