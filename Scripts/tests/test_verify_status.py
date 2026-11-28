# SPDX-License-Identifier: CC0-1.0
from pathlib import Path
import hashlib
import json
import subprocess
import sys
import tempfile
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import update_verify_status as status


class VerifyStatusTest(unittest.TestCase):
    def setUp(self):
        self.directory = tempfile.TemporaryDirectory()
        self.addCleanup(self.directory.cleanup)
        self.root = Path(self.directory.name)
        self.write("Scripts/library_layout.json", json.dumps(["DataStructure"]))
        self.write("DataStructure/example.cpp", "int library_value = 1;\n")
        self.write("Verify/current/common/fast_io.hpp", "int io_value = 2;\n")
        self.write("Verify/current/drivers/example.cpp", status.prepare_verify.FAST_IO_INCLUDE
                   + "\nint main() { return library_value + io_value; }\n")
        self.entry = dict(id="example", title="Example", libraries=["DataStructure/example.cpp"],
                          driver="Verify/current/drivers/example.cpp",
                          problem_url="https://judge.yosupo.jp/problem/example", samples=[])
        self.group([self.entry])
        self.write("Readme.md", "before\n" + status.START + "\nold\n" + status.END + "\nafter\n")

    def write(self, name, text):
        path = self.root / name
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(text)

    def group(self, entries):
        self.write("Verify/current/groups/example.json", json.dumps(dict(entries=entries)))

    def source(self):
        with status.repository(self.root):
            return status.prepare_verify.render(self.entry)[0]

    def record(self, **changes):
        return dict(id="example", status="AC", url="https://judge.yosupo.jp/submission/123",
                    submission_sha256=status.digest(self.source()), time_ms=12, **changes)

    def journal(self, records):
        self.write("Verify/current/judgements.json", json.dumps(dict(entries=records)))

    def row(self):
        return status.build_status(self.root)["entries"][0]

    def cli(self, *args):
        return subprocess.run([sys.executable, str(Path(status.__file__)), "--root", str(self.root), *args],
                              capture_output=True, text=True)

    def test_missing_judgements_are_unsubmitted(self):
        self.assertEqual(self.row()["status"], "unsubmitted")

    def test_exact_ac_current_and_json_deterministic(self):
        self.journal([self.record()])
        row = self.row()
        self.assertEqual(row["status"], "current_ac")
        self.assertEqual(row["judgement"]["source_match"], "exact")
        self.assertEqual(row["judgement"]["time_ms"], 12)
        self.assertEqual(status.generate(self.root), status.generate(self.root))

    def test_uploaded_full_source_hash_also_qualifies(self):
        record = self.record()
        record["uploaded_source_sha256"] = record.pop("submission_sha256")
        self.journal([record])
        self.assertEqual(self.row()["status"], "current_ac")

    def test_atcoder_user_confirmation_is_limited_to_the_recorded_atcoder_source(self):
        record = self.record(evidence_kind="atcoder_user_confirmed")
        self.journal([record])
        self.assertEqual(self.row()["status"], "source_changed")
        self.entry["problem_url"] = "https://atcoder.jp/contests/abc001/tasks/abc001_1"
        self.group([self.entry])
        record["url"] = "https://atcoder.jp/contests/abc001/submissions/123"
        record["submission_sha256"] = status.digest(self.source())
        self.journal([record])
        row = self.row()
        self.assertEqual(row["status"], "current_ac")
        self.assertEqual(row["judgement"]["source_match"], "atcoder_user_confirmed")
        self.assertNotIn("uploaded_source_sha256", row["judgement"])
        self.write("DataStructure/example.cpp", "int library_value = 2;\n")
        self.assertEqual(self.row()["status"], "source_changed")

    def test_stamped_source_contains_the_hash_of_the_code_without_submission_url(self):
        self.entry["problem_url"] = "https://atcoder.jp/contests/abc001/tasks/abc001_1"
        original = self.source()
        self.entry["submission_urls"] = ["https://atcoder.jp/contests/abc001/submissions/123"]
        with status.repository(self.root):
            stamped = status.prepare_verify.stamped_source(self.entry)
            self.write("DataStructure/example.cpp", "int library_value = 2;\n")
            updated = status.prepare_verify.stamped_source(self.entry)
        first, rest = original.split(b"\n", 1)
        self.assertEqual(stamped, first + b"\n// Verify-SHA256: " + status.digest(original).encode() + b"\n" + rest)
        self.assertNotEqual(stamped.splitlines()[1], updated.splitlines()[1])

    def test_default_stamp_only_applies_to_unsubmitted_atcoder_sources(self):
        atcoder = {**self.entry, "problem_url": "https://atcoder.jp/contests/abc001/tasks/abc001_1"}
        submitted_url = "https://atcoder.jp/contests/abc001/submissions/123"
        self.group([self.entry, {**atcoder, "id": "pending"},
                    {**atcoder, "id": "with_url", "submission_urls": [submitted_url]},
                    {**atcoder, "id": "with_judgement"}])
        self.journal([{"id": "with_judgement", "status": "AC", "url": submitted_url}])
        with status.repository(self.root):
            entries = status.prepare_verify.prepare(check=False)
            status.prepare_verify.prepare(check=True)
        files = {entry["id"]: (self.root / entry["submission"]).read_bytes() for entry in entries}
        self.assertEqual({name for name, data in files.items() if b"Verify-SHA256:" in data}, {"pending"})
        row = next(row for row in status.build_status(self.root)["entries"] if row["id"] == "pending")
        self.assertEqual(row["status"], "unsubmitted")
        self.assertEqual(row["submission_sha256"], status.digest(files["pending"]))
        self.assertTrue(row["submission_in_sync"])

    def test_adding_submission_url_keeps_the_existing_stamp(self):
        self.entry["problem_url"] = "https://atcoder.jp/contests/abc001/tasks/abc001_1"
        self.group([self.entry])
        submitted_url = "https://atcoder.jp/contests/abc001/submissions/123"
        with status.repository(self.root):
            entry, = status.prepare_verify.prepare(check=False)
            path = self.root / entry["submission"]
            header, stamp, problem, body = path.read_bytes().split(b"\n", 3)
            self.assertEqual(header, b"#define SKIP_RUNTIME_CHECKS")
            self.assertIn(b"Verify-SHA256:", stamp)
            annotated = b"\n".join((header, stamp, problem, b"// Submission: " + submitted_url.encode(), body))
            path.write_bytes(annotated)
            status.prepare_verify.prepare(check=False)
            self.assertEqual(path.read_bytes(), annotated)
            self.journal([{"id": "example", "status": "AC", "url": submitted_url,
                           "submission_sha256": status.digest(annotated)}])
            status.prepare_verify.prepare(check=False)
            status.prepare_verify.prepare(check=True)
        self.assertEqual(path.read_bytes(), annotated)
        self.assertEqual(self.row()["status"], "current_ac")

    def test_submitted_inline_stamp_stays_inline(self):
        self.entry["problem_url"] = "https://atcoder.jp/contests/abc001/tasks/abc001_1"
        self.group([self.entry])
        submitted_url = "https://atcoder.jp/contests/abc001/submissions/123"
        with status.repository(self.root):
            legacy, _ = status.prepare_verify.render({**self.entry, "stamp_submission": True,
                                                       "submission_urls": [submitted_url]})
            self.write("Verify/current/submissions/example.cpp", legacy.decode())
            status.prepare_verify.prepare(check=False)
            status.prepare_verify.prepare(check=True)
        self.assertEqual((self.root / "Verify/current/submissions/example.cpp").read_bytes(), legacy)
        self.assertIn(b" // Verify-SHA256: ", legacy.splitlines()[0])

    def test_current_metadata_fields_alone_never_qualify(self):
        record = self.record()
        record["current_submission_sha256"] = record.pop("submission_sha256")
        record["matched_current_generation"] = True
        self.journal([record])
        self.assertEqual(self.row()["status"], "source_changed")
        self.assertNotIn("time_ms", self.row()["judgement"])

    def test_library_driver_and_io_changes_require_resubmission(self):
        for path in ("DataStructure/example.cpp", "Verify/current/drivers/example.cpp",
                     "Verify/current/common/fast_io.hpp"):
            with self.subTest(path=path):
                original = (self.root / path).read_text()
                self.journal([self.record()])
                self.write(path, original + "int changed = 3;\n")
                self.assertEqual(self.row()["status"], "source_changed")
                self.assertNotIn("time_ms", self.row()["judgement"])
                self.write(path, original)

    def test_manifest_is_not_a_source_of_truth(self):
        self.write("Verify/current/manifest.json", '{"entries":[{"id":"example","judge_status":"AC"}]}')
        self.assertEqual(self.row()["status"], "unsubmitted")

    def test_generated_file_sync_is_reported_independently(self):
        row = self.row()
        self.assertFalse(row["submission_exists"])
        self.assertFalse(row["submission_in_sync"])
        self.write("Verify/current/submissions/example.cpp", self.source().decode())
        self.assertTrue(self.row()["submission_in_sync"])
        self.write("Verify/current/submissions/example.cpp", "stale source\n")
        self.assertFalse(self.row()["submission_in_sync"])
        text = status.status_markdown(status.build_status(self.root))
        self.assertIn("提出ファイル未同期", text)

    def test_historical_matching_ac_can_qualify(self):
        previous = self.record()
        current = self.record()
        current["submission_sha256"] = "a" * 64
        current["history"] = [previous]
        self.journal([current])
        self.assertEqual(self.row()["status"], "current_ac")

    def test_wrapped_history_and_notes(self):
        current = self.record()
        current["submission_sha256"] = "a" * 64
        current["history"] = [{"reason": "old local note", "reported_local_snapshot": {}},
                              {"reason": "old record", "record": self.record()}]
        self.journal([current])
        self.assertEqual(self.row()["status"], "current_ac")

    def test_other_id_history_is_not_evidence(self):
        current = self.record()
        current["submission_sha256"] = "a" * 64
        other = self.record()
        other["id"] = "other"
        current["history"] = [other]
        self.journal([current])
        self.assertEqual(self.row()["status"], "source_changed")

    def test_previous_other_judge_record_is_not_current(self):
        record = self.record()
        record["url"] = "https://atcoder.jp/contests/example/submissions/123"
        self.journal([record])
        self.assertEqual(self.row()["status"], "source_changed")

    def test_matching_non_ac_is_not_overridden_by_old_ac(self):
        current = self.record()
        current["status"] = "WA"
        current["history"] = [self.record()]
        self.journal([current])
        self.assertEqual(self.row()["status"], "non_ac")

    def test_no_hash_ac_remains_unconfirmed(self):
        record = self.record()
        record.pop("submission_sha256")
        self.journal([record])
        self.assertEqual(self.row()["status"], "source_changed")

    def test_no_url_ac_is_not_current(self):
        record = self.record()
        record.pop("url")
        self.journal([record])
        self.assertEqual(self.row()["status"], "source_changed")

    def test_normalization_requires_uploaded_proof_and_narrow_scope(self):
        source = self.source()
        normalized = status.digest(status.normalized_source(source))
        record = self.record()
        record["submission_sha256"] = "a" * 64
        record["normalized_source_sha256"] = normalized
        record["source_match"] = "CRLF/LF・空行・行端空白・Submissionコメントのみを正規化"
        self.journal([record])
        self.assertEqual(self.row()["status"], "source_changed")
        record["uploaded_source_sha256"] = "b" * 64
        self.journal([record])
        self.assertEqual(self.row()["status"], "current_ac")
        record["source_match"] = "全文コメント・Submissionコメントのみを正規化"
        self.journal([record])
        self.assertEqual(self.row()["status"], "source_changed")

    def test_only_submission_metadata_is_ignored(self):
        record = self.record()
        record["submission_sha256"] = "a" * 64
        record["uploaded_source_sha256"] = "b" * 64
        record["normalized_source_sha256"] = status.digest(status.normalized_source(self.source()))
        record["source_normalization"] = "submission-metadata-v1"
        self.entry["submission_urls"] = ["https://judge.yosupo.jp/submission/456"]
        self.group([self.entry])
        self.journal([record])
        self.assertEqual(self.row()["status"], "current_ac")
        self.write("DataStructure/example.cpp", "// a real source comment changed\nint library_value = 1;\n")
        self.assertEqual(self.row()["status"], "source_changed")

    def test_semantic_raw_string_or_continuation_is_not_normalized(self):
        for text in ('const char* x=R"(value\n\n)";\n', 'int x=1; /* block */\n',
                     '#define VALUE \\\n+ 1\n'):
            record = dict(submission_sha256="a" * 64, uploaded_source_sha256="b" * 64,
                          normalized_source_sha256=status.digest(status.normalized_source(text.encode())),
                          source_normalization="submission-metadata-v1")
            self.assertIsNone(status.source_match(record, text.encode()))

    def test_local_only_and_orphan_records(self):
        local = {**self.entry, "id": "local", "problem_url": None}
        self.group([self.entry, local])
        ghost = self.record()
        ghost["id"] = "ghost"
        self.journal([ghost])
        data = status.build_status(self.root)
        self.assertEqual(data["counts"]["local_only"], 1)
        self.assertEqual(data["orphan_judgement_ids"], ["ghost"])

    def test_corrupt_records_fail_without_output_writes(self):
        bad_inputs = ['{', '{}', '{"entries":{}}', '{"entries":[null]}']
        for bad in bad_inputs:
            self.write("Verify/current/judgements.json", bad)
            result = self.cli()
            self.assertNotEqual(result.returncode, 0)
            self.assertFalse((self.root / "Verify/current/status.json").exists())
            self.assertIn("old", (self.root / "Readme.md").read_text())

    def test_duplicate_ids_and_invalid_hashes_rejected(self):
        record = self.record()
        for rows in ([record, record], [{**record, "submission_sha256": "not-a-hash"}],
                     [{**record, "time_ms": float("nan")}], [{**record, "time_ms": -1}]):
            self.journal(rows)
            with self.assertRaises(ValueError):
                self.row()

    def test_url_and_id_injection_rejected(self):
        for url in ("javascript:alert(1)", "https://judge.yosupo.jp@evil.test/x", "https://evil.test/x",
                    "https://judge.yosupo.jp/x\ntext", "https://user@judge.yosupo.jp/x"):
            record = self.record()
            record["url"] = url
            self.journal([record])
            with self.assertRaises(ValueError):
                self.row()
        record = self.record()
        record["id"] = "bad|<script>"
        self.journal([record])
        with self.assertRaises(ValueError):
            self.row()

    def test_markdown_text_is_escaped(self):
        record = self.record()
        record["status"] = "WA|[link](bad)<script>"
        self.journal([record])
        text = status.status_markdown(status.build_status(self.root))
        self.assertIn(r"WA\|\[link\]\(bad\)&lt;script&gt;", text)
        self.assertNotIn("<script>", text)
        self.assertEqual(status.safe_url("https://judge.yosupo.jp/submission/(1)"),
                         "https://judge.yosupo.jp/submission/%281%29")

    def test_readme_markers_missing_duplicate_reversed(self):
        for text in ("nothing", status.START + status.START + status.END,
                     status.END + status.START, status.START):
            with self.assertRaises(ValueError):
                status.update_readme(text, "new\n")

    def test_check_never_mutates_and_generation_preserves_surroundings(self):
        self.journal([self.record()])
        before = {p.relative_to(self.root): p.read_bytes() for p in self.root.rglob('*') if p.is_file()}
        check = self.cli("--check")
        self.assertNotEqual(check.returncode, 0)
        after = {p.relative_to(self.root): p.read_bytes() for p in self.root.rglob('*') if p.is_file()}
        self.assertEqual(before, after)
        self.assertEqual(self.cli().returncode, 0)
        readme = (self.root / "Readme.md").read_text()
        self.assertTrue(readme.startswith("before\n" + status.START))
        self.assertTrue(readme.endswith(status.END + "\nafter\n"))
        self.assertEqual(self.cli("--check").returncode, 0)

    def test_status_only_does_not_need_or_modify_readme(self):
        (self.root / "Readme.md").unlink()
        self.assertEqual(self.cli("--status-only").returncode, 0)
        self.assertFalse((self.root / "Readme.md").exists())
        self.assertEqual(self.cli("--status-only", "--check").returncode, 0)

    def test_readme_path_cannot_escape_root(self):
        with self.assertRaises(ValueError):
            status.generate(self.root, readme="../outside.md")


if __name__ == "__main__":
    unittest.main()
