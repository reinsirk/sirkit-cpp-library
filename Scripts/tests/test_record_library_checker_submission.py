# SPDX-License-Identifier: CC0-1.0
import copy
from email.message import Message
import io
import json
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import record_library_checker_submission as importer


class RecordSubmissionTest(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory()
        self.addCleanup(self.temporary.cleanup)
        self.root = Path(self.temporary.name)
        self.write("Scripts/library_layout.json", json.dumps(["DataStructure"]))
        self.write("DataStructure/example.cpp", "int library_value=1;\n")
        self.write("Verify/current/common/fast_io.hpp", "int io_value=2;\n")
        self.write("Verify/current/drivers/example.cpp", importer.verify_status.prepare_verify.FAST_IO_INCLUDE
                   + "\nint main() {return library_value+io_value;}\n")
        self.entry = dict(id="example", title="Example", libraries=["DataStructure/example.cpp"],
                          driver="Verify/current/drivers/example.cpp",
                          problem_url="https://judge.yosupo.jp/problem/example", samples=[])
        self.write("Verify/current/groups/example.json", json.dumps(dict(entries=[self.entry])))
        self.url = "https://judge.yosupo.jp/submission/321"
        with importer.verify_status.repository(self.root):
            self.source = importer.verify_status.prepare_verify.render(self.entry)[0]
        self.response = dict(can_rejudge=False, overview=dict(
            id=321, is_latest=True, lang="cpp", memory=1024, problem_name="example",
            problem_title="Example", status="AC", submission_time="2026-11-13T01:02:03Z", time=0.012,
            user_name="example_user"), source=self.source.decode(),
            case_results=[dict(case="example_00", status="AC", time=0.012, memory=1024)])

    def write(self, name, text):
        path = self.root / name
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(text)

    def payload(self, response=None):
        return json.dumps(self.response if response is None else response).encode()

    def prepare(self, response=None):
        return importer.prepare_import(self.root, "example", self.url, self.payload(response),
                                       checked_at="2026-11-13T04:00:00+00:00")

    def journal(self):
        return json.loads((self.root / "Verify/current/judgements.json").read_text())

    def test_exact_source_record_contains_bound_hashes(self):
        result = self.prepare()
        record = result["record"]
        self.assertEqual(record["status"], "AC")
        self.assertEqual(record["time_ms"], 12)
        self.assertEqual(record["submission_sha256"], importer.digest(self.source))
        self.assertEqual(record["uploaded_source_sha256"], importer.digest(self.source))
        self.assertEqual(record["source_match_method"], "exact")
        self.assertIn("DataStructure/example.cpp", record["source_hashes"])
        self.assertIn(importer.verify_status.prepare_verify.FAST_IO, record["io_source_hashes"])
        self.assertEqual(record["evidence_kind"], "supplied_api_response")
        self.assertFalse((self.root / "Verify/current/judgements.json").exists())

    def test_live_response_provenance_is_distinct(self):
        result = importer.prepare_import(self.root, "example", self.url, self.payload(), fetched_live=True)
        self.assertEqual(result["record"]["evidence_kind"], "public_judge_api")

    def test_exact_record_survives_submission_url_update(self):
        record = self.prepare()["record"]
        self.entry["submission_urls"] = [self.url]
        with importer.verify_status.repository(self.root):
            updated = importer.verify_status.prepare_verify.render(self.entry)[0]
        self.assertEqual(importer.verify_status.source_match(record, updated), "submission_metadata_normalized")
        self.assertIsNone(importer.verify_status.source_match(record, updated + b"// implementation changed\n"))

    def test_line_endings_record_survives_submission_url_update(self):
        self.response["source"] = self.source.decode().replace("\n", "\r\n")
        record = self.prepare()["record"]
        self.entry["submission_urls"] = [self.url]
        with importer.verify_status.repository(self.root):
            updated = importer.verify_status.prepare_verify.render(self.entry)[0]
        self.assertEqual(importer.verify_status.source_match(record, updated), "submission_metadata_normalized")

    def test_custom_online_io_matches_generation_metadata(self):
        self.entry["allow_custom_io"] = True
        self.write("Verify/current/groups/example.json", json.dumps(dict(entries=[self.entry])))
        self.write(self.entry["driver"], "int main() {return library_value;}\n")
        with importer.verify_status.repository(self.root):
            self.response["source"] = importer.verify_status.prepare_verify.render(self.entry)[0].decode()
        record = self.prepare()["record"]
        self.assertEqual(record["io_mode"], "custom_online_io")
        self.assertEqual(record["io_source_hashes"], {})

    def test_crlf_source_matches_with_all_body_bytes_preserved(self):
        self.response["source"] = self.source.decode().replace("\n", "\r\n")
        record = self.prepare()["record"]
        self.assertEqual(record["source_match_method"], "line_endings")
        self.assertNotEqual(record["uploaded_source_sha256"], record["submission_sha256"])

    def test_narrow_metadata_and_trailing_whitespace_match(self):
        self.response["source"] = "// Submission: https://judge.yosupo.jp/submission/1\r\n\r\n" + self.source.decode().replace("\n", " \r\n")
        record = self.prepare()["record"]
        self.assertEqual(record["source_match_method"], "submission_metadata_normalized")
        self.assertEqual(record["source_normalization"], "submission-metadata-v1")
        self.assertEqual(importer.verify_status.source_match(record, self.source), "exact")

    def test_main_library_and_io_tampering_rejected(self):
        for old, new in (("library_value=1", "library_value=9"), ("io_value=2", "io_value=7"),
                         ("return library_value+io_value", "return 0")):
            response = copy.deepcopy(self.response)
            response["source"] = response["source"].replace(old, new)
            with self.subTest(part=old), self.assertRaisesRegex(ValueError, "提出ソース全文"):
                self.prepare(response)
        self.assertFalse((self.root / "Verify/current/judgements.json").exists())

    def test_general_comment_changes_are_not_ignored(self):
        self.response["source"] += "// a real implementation comment\n"
        with self.assertRaises(ValueError):
            self.prepare()

    def test_unsafe_cpp_does_not_use_line_metadata_normalization(self):
        for source in (b'const char* text=R"(line\n\n)";\n', b'int value=1; /* note */\n',
                       b'#define VALUE \\\n+ 1\n'):
            uploaded = source + b"// Submission: https://judge.yosupo.jp/submission/1\n"
            with self.subTest(source=source), self.assertRaises(ValueError):
                importer.compare_sources(source, uploaded)
            self.assertEqual(importer.compare_sources(source, source.replace(b"\n", b"\r\n")), "line_endings")

    def test_url_and_id_mismatch_rejected(self):
        response = copy.deepcopy(self.response)
        response["overview"]["id"] = 322
        with self.assertRaisesRegex(ValueError, "提出番号"):
            self.prepare(response)
        with self.assertRaisesRegex(ValueError, "見つかりません"):
            importer.prepare_import(self.root, "missing", self.url, self.payload())

    def test_problem_mismatch_or_non_library_checker_target_rejected(self):
        response = copy.deepcopy(self.response)
        response["overview"]["problem_name"] = "other_problem"
        with self.assertRaisesRegex(ValueError, "問題が一致"):
            self.prepare(response)
        self.entry["problem_url"] = "https://atcoder.jp/contests/example/tasks/example"
        self.write("Verify/current/groups/example.json", json.dumps(dict(entries=[self.entry])))
        with self.assertRaisesRegex(ValueError, "Library Checker"):
            self.prepare()

    def test_pending_and_unknown_status_cannot_be_written(self):
        for value in ("WJ", "JUDGING", "PENDING", "new_unknown_status", {}, None):
            response = copy.deepcopy(self.response)
            response["overview"]["status"] = value
            with self.subTest(status=value), self.assertRaises(ValueError):
                self.prepare(response)
        self.assertFalse((self.root / "Verify/current/judgements.json").exists())

    def test_ac_requires_terminal_consistent_cases(self):
        for cases in ([], None, [dict(case="x", status="WA")], [dict(case="x", status="WJ")]):
            response = copy.deepcopy(self.response)
            response["case_results"] = cases
            with self.subTest(cases=cases), self.assertRaises(ValueError):
                self.prepare(response)

    def test_matching_non_ac_can_be_recorded_without_claiming_ac(self):
        self.response["overview"]["status"] = "WA"
        self.response["case_results"][0]["status"] = "WA"
        proposal = self.prepare()
        importer.write_import(proposal)
        self.assertEqual(self.journal()["entries"][0]["status"], "WA")
        self.assertTrue(self.journal()["entries"][0]["needs_resubmission"])
        result = importer.verify_status.build_status(self.root)
        self.assertEqual(result["entries"][0]["status"], "non_ac")

    def test_missing_or_non_string_source_rejected(self):
        for value in (None, "", 12, {"code": self.source.decode()}, "code\0text"):
            response = copy.deepcopy(self.response)
            response["source"] = value
            with self.subTest(source=value), self.assertRaises(ValueError):
                self.prepare(response)

    def test_unknown_schema_and_duplicate_json_keys_rejected(self):
        for content in (b'{}', b'{"submission":{}}', b'[]', b'{',
                        b'{"source":"a","source":"b","overview":{},"case_results":[]}'):
            with self.subTest(content=content), self.assertRaises(ValueError):
                importer.parse_response(content)

    def test_invalid_metadata_rejected(self):
        for key, value in (("time", -1), ("time", float("nan")), ("time", True),
                           ("memory", -1), ("memory", 1.5), ("id", True),
                           ("submission_time", "not-a-date"), ("submission_time", "2026-11-13T00:00:00"),
                           ("lang", "<script>")):
            response = copy.deepcopy(self.response)
            response["overview"][key] = value
            with self.subTest(key=key, value=value), self.assertRaises(ValueError):
                self.prepare(response)

    def test_size_limits_enforced(self):
        with self.assertRaises(ValueError):
            importer.parse_response(b" " * (importer.MAX_RESPONSE + 1))
        self.response["source"] = "a" * (importer.MAX_SOURCE + 1)
        with self.assertRaisesRegex(ValueError, "512 KiB"):
            self.prepare()

    def test_previous_record_and_all_custom_fields_preserved_as_history(self):
        old = dict(id="example", status="AC", url="https://judge.yosupo.jp/submission/1",
                   submission_sha256="a" * 64, custom={"original": [1, 2, 3]},
                   history=[dict(status="WA", url="https://judge.yosupo.jp/submission/2")])
        other = dict(id="other", status="AC", url="https://judge.yosupo.jp/submission/3", note="keep")
        self.write("Verify/current/judgements.json", json.dumps(dict(entries=[old, other], custom_top="keep")))
        proposal = self.prepare()
        importer.write_import(proposal)
        journal = self.journal()
        self.assertEqual(journal["entries"][0]["history"], [old])
        self.assertEqual(journal["entries"][1], other)
        self.assertEqual(journal["custom_top"], "keep")
        self.assertEqual(importer.verify_status.build_status(self.root)["entries"][0]["status"], "current_ac")

    def test_repeat_import_is_noop_and_preserves_exact_file_bytes(self):
        importer.write_import(self.prepare())
        path = self.root / "Verify/current/judgements.json"
        before = path.read_bytes()
        proposal = self.prepare()
        self.assertFalse(proposal["changed"])
        importer.write_import(proposal)
        self.assertEqual(path.read_bytes(), before)

    def test_corrupt_or_duplicate_journal_fails_before_mutation(self):
        for content in ('{', '{}', '{"entries":[null]}',
                        '{"entries":[{"id":"x","status":"AC"},{"id":"x","status":"AC"}]}'):
            self.write("Verify/current/judgements.json", content)
            before = (self.root / "Verify/current/judgements.json").read_bytes()
            with self.assertRaises(ValueError):
                self.prepare()
            self.assertEqual((self.root / "Verify/current/judgements.json").read_bytes(), before)

    def test_concurrent_change_rejected(self):
        proposal = self.prepare()
        self.write("Verify/current/judgements.json", '{"entries":[],"changed":true}\n')
        before = (self.root / "Verify/current/judgements.json").read_bytes()
        with self.assertRaisesRegex(ValueError, "変更されました"):
            importer.write_import(proposal)
        self.assertEqual((self.root / "Verify/current/judgements.json").read_bytes(), before)

    def test_failed_atomic_replace_keeps_old_bytes_and_cleans_temp(self):
        self.write("Verify/current/judgements.json", '{"entries":[]}\n')
        proposal = self.prepare()
        with patch.object(importer.os, "replace", side_effect=OSError("injected")):
            with self.assertRaises(OSError):
                importer.write_import(proposal)
        self.assertEqual((self.root / "Verify/current/judgements.json").read_text(), '{"entries":[]}\n')
        self.assertEqual(list((self.root / "Verify/current").glob('.judgements-*.tmp')), [])

    def test_redirects_and_untrusted_submission_urls_rejected(self):
        for url in ("http://judge.yosupo.jp/submission/321", "https://evil.test/submission/321",
                    "https://judge.yosupo.jp.evil.test/submission/321", "https://judge.yosupo.jp/submission/321?next=x",
                    "https://judge.yosupo.jp/submission/../1", "https://judge.yosupo.jp/submission/0",
                    "https://user@judge.yosupo.jp/submission/321", "https://judge.yosupo.jp:443/submission/321"):
            with self.subTest(url=url), self.assertRaises(ValueError):
                importer.submission_id(url)
        with self.assertRaisesRegex(ValueError, "リダイレクト"):
            importer.NoRedirect().redirect_request(None, None, 302, "Found", {}, "http://127.0.0.1/")

    def test_fetch_only_uses_fixed_public_api_and_size_limit(self):
        class Response(io.BytesIO):
            status = 200
            headers = Message()
            headers["Content-Type"] = "application/json"
            def geturl(self):
                return "https://v3.api.judge.yosupo.jp/submissions/321"
        for content in (self.payload(), b"x" * (importer.MAX_RESPONSE + 1)):
            with patch.object(importer.urllib.request, "build_opener") as build:
                build.return_value.open.return_value = Response(content)
                if len(content) > importer.MAX_RESPONSE:
                    with self.assertRaises(ValueError):
                        importer.fetch_response(321)
                else:
                    self.assertEqual(importer.fetch_response(321), content)
                request = build.return_value.open.call_args.args[0]
                self.assertEqual(request.full_url, "https://v3.api.judge.yosupo.jp/submissions/321")
                self.assertNotIn("Authorization", request.headers)

    def test_cli_defaults_to_dry_run_and_never_uses_network_with_response_json(self):
        response = self.root / "response.json"
        response.write_bytes(self.payload())
        cmd = [sys.executable, str(Path(importer.__file__)), "--root", str(self.root), "--id", "example",
               "--url", self.url, "--response-json", str(response)]
        run = subprocess.run(cmd, capture_output=True, text=True)
        self.assertEqual(run.returncode, 0, run.stderr)
        result = json.loads(run.stdout)
        self.assertFalse(result["written"])
        self.assertFalse((self.root / "Verify/current/judgements.json").exists())
        run = subprocess.run(cmd + ["--write"], capture_output=True, text=True)
        self.assertEqual(run.returncode, 0, run.stderr)
        self.assertTrue(json.loads(run.stdout)["written"])
        run = subprocess.run(cmd + ["--write"], capture_output=True, text=True)
        self.assertEqual(run.returncode, 0, run.stderr)
        self.assertFalse(json.loads(run.stdout)["written"])


if __name__ == "__main__":
    unittest.main()
